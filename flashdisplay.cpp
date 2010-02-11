/**************************************************************************
    Lightspark, a free flash player implementation

    Copyright (C) 2009  Alessandro Pignotti (a.pignotti@sssup.it)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
**************************************************************************/

#include <list>
#include <algorithm>

#include "abc.h"
#include "flashdisplay.h"
#include "swf.h"
#include "flashgeom.h"
#include "flashnet.h"
#include "flashsystem.h"
#include "streams.h"
#include "compat.h"
#include "class.h"

#include <GL/glew.h>
#include <fstream>
using namespace std;
using namespace lightspark;

extern TLSDATA SystemState* sys;
extern TLSDATA RenderThread* rt;
extern TLSDATA ParseThread* pt;

REGISTER_CLASS_NAME(LoaderInfo);
REGISTER_CLASS_NAME(MovieClip);
REGISTER_CLASS_NAME(DisplayObject);
REGISTER_CLASS_NAME(DisplayObjectContainer);
REGISTER_CLASS_NAME(Sprite);
REGISTER_CLASS_NAME(Loader);
REGISTER_CLASS_NAME(Shape);
REGISTER_CLASS_NAME(Stage);
REGISTER_CLASS_NAME(Graphics);
REGISTER_CLASS_NAME(LineScaleMode);
REGISTER_CLASS_NAME(StageScaleMode);
REGISTER_CLASS_NAME(StageAlign);

void LoaderInfo::sinit(Class_base* c)
{
	assert(c->constructor==NULL);
	c->constructor=new Function(_constructor);
	c->super=Class<EventDispatcher>::getClass();
	c->max_level=c->super->max_level+1;
}

void LoaderInfo::buildTraits(ASObject* o)
{
	o->setGetterByQName("loaderURL","",new Function(_getLoaderUrl));
	o->setGetterByQName("url","",new Function(_getUrl));
	o->setGetterByQName("bytesLoaded","",new Function(_getBytesLoaded));
	o->setGetterByQName("bytesTotal","",new Function(_getBytesTotal));
	o->setGetterByQName("applicationDomain","",new Function(_getApplicationDomain));
	o->setGetterByQName("sharedEvents","",new Function(_getSharedEvents));
}

ASFUNCTIONBODY(LoaderInfo,_constructor)
{
	LoaderInfo* th=static_cast<LoaderInfo*>(obj->implementation);
	EventDispatcher::_constructor(obj,args);
	th->sharedEvents=Class<EventDispatcher>::getInstanceS(true);
	return NULL;
}

ASFUNCTIONBODY(LoaderInfo,_getLoaderUrl)
{
	LoaderInfo* th=static_cast<LoaderInfo*>(obj->implementation);
	return Class<ASString>::getInstanceS(true,th->loaderURL)->obj;
}

ASFUNCTIONBODY(LoaderInfo,_getSharedEvents)
{
	LoaderInfo* th=static_cast<LoaderInfo*>(obj->implementation);
	th->sharedEvents->obj->incRef();
	return th->sharedEvents->obj;
}

ASFUNCTIONBODY(LoaderInfo,_getUrl)
{
	LoaderInfo* th=static_cast<LoaderInfo*>(obj->implementation);
	return Class<ASString>::getInstanceS(true,th->url)->obj;
}

ASFUNCTIONBODY(LoaderInfo,_getBytesLoaded)
{
	LoaderInfo* th=static_cast<LoaderInfo*>(obj->implementation);
	return abstract_i(th->bytesLoaded);
}

ASFUNCTIONBODY(LoaderInfo,_getBytesTotal)
{
	LoaderInfo* th=static_cast<LoaderInfo*>(obj->implementation);
	return abstract_i(th->bytesTotal);
}

ASFUNCTIONBODY(LoaderInfo,_getApplicationDomain)
{
	return Class<ApplicationDomain>::getInstanceS(true)->obj;
}

ASFUNCTIONBODY(Loader,_constructor)
{
	Loader* th=static_cast<Loader*>(obj->implementation);
	th->contentLoaderInfo=Class<LoaderInfo>::getInstanceS(true);
	return NULL;
}

ASFUNCTIONBODY(Loader,_getContentLoaderInfo)
{
	Loader* th=static_cast<Loader*>(obj->implementation);
	th->contentLoaderInfo->obj->incRef();
	return th->contentLoaderInfo->obj;
}

ASFUNCTIONBODY(Loader,load)
{
	Loader* th=static_cast<Loader*>(obj->implementation);
/*	if(th->loading)
		return NULL;
	th->loading=true;*/
	abort();
/*	if(args->at(0)->getClassName()!="URLRequest")
	{
		LOG(ERROR,"ArgumentError");
		abort();
	}*/
	URLRequest* r=static_cast<URLRequest*>(args->at(0)->implementation);
	th->url=r->url;
	th->source=URL;
	sys->cur_thread_pool->addJob(th);
	return NULL;
}

ASFUNCTIONBODY(Loader,loadBytes)
{
	Loader* th=static_cast<Loader*>(obj->implementation);
	if(th->loading)
		return NULL;
	//Find the actual ByteArray object
	assert(args->size()>=1);
	assert(args->at(0)->prototype->isSubClass(Class<ByteArray>::getClass()));
	th->bytes=static_cast<ByteArray*>(args->at(0)->implementation);
	if(th->bytes->bytes)
	{
		th->loading=true;
		th->source=BYTES;
		sys->cur_thread_pool->addJob(th);
	}
}

void Loader::sinit(Class_base* c)
{
	assert(c->constructor==NULL);
	c->constructor=new Function(_constructor);
	c->super=Class<DisplayObjectContainer>::getClass();
	c->max_level=c->super->max_level+1;
}

void Loader::buildTraits(ASObject* o)
{
	o->setGetterByQName("contentLoaderInfo","",new Function(_getContentLoaderInfo));
	o->setVariableByQName("loadBytes","",new Function(loadBytes));
//	obj->setVariableByQName("load","",new Function(load));
}

void Loader::execute()
{
	static char name[]="0dump";
	LOG(LOG_NOT_IMPLEMENTED,"Loader async execution " << url);
	if(source==URL)
	{
		abort();
		/*local_root=new RootMovieClip;
		zlib_file_filter zf;
		zf.open(url.raw_buf(),ios_base::in);
		istream s(&zf);

		ParseThread local_pt(sys,local_root,s);
		local_pt.wait();*/
	}
	else if(source==BYTES)
	{
		//Implement loadBytes, now just dump
		assert(bytes->bytes);

		/*FILE* f=fopen(name,"w");
		fwrite(bytes->bytes,1,bytes->len,f);
		fclose(f);

		name[0]++;*/

		//We only support swf files now
		assert(memcmp(bytes->bytes,"CWS",3)==0);

		//The loaderInfo of the content is out contentLoaderInfo
		contentLoaderInfo->obj->incRef();
		local_root=new RootMovieClip(contentLoaderInfo);
		zlib_bytes_filter zf(bytes->bytes,bytes->len);
		istream s(&zf);

		ParseThread local_pt(sys,local_root,s);
		local_pt.wait();
		//HACK: advance to first frame, so that scripts get executed
		//We shold understand how to deliver frame events to movieclips not in the display list
		local_root->advanceFrame();
		content=local_root;
	}
	loaded=true;
	//Add a complete event for this object
	sys->currentVm->addEvent(contentLoaderInfo,Class<Event>::getInstanceS(true,"complete"));
}

void Loader::Render()
{
	if(!loaded)
		return;

	local_root->Render();
}

Sprite::Sprite():graphics(NULL)
{
}

void Sprite::sinit(Class_base* c)
{
	assert(c->constructor==NULL);
	c->constructor=new Function(_constructor);
	c->super=Class<DisplayObjectContainer>::getClass();
	c->max_level=c->super->max_level+1;
}

void Sprite::buildTraits(ASObject* o)
{
	o->setGetterByQName("graphics","",new Function(_getGraphics));
}

void Sprite::Render()
{
/*	if(obj && obj->prototype && obj->prototype->class_name=="ContainerBorderSkin")
	{
		if(parent && parent->obj->prototype->class_name=="VBox")
			__asm__("int $3");
	}*/

	glDrawBuffer(GL_COLOR_ATTACHMENT1_EXT);
	glDisable(GL_BLEND);
	glClearColor(1,1,1,0);
	glClear(GL_COLOR_BUFFER_BIT);

	float matrix[16];
	Matrix.get4DMatrix(matrix);
	glPushMatrix();
	glMultMatrixf(matrix);

	//Draw the dynamically added graphics, if any
	if(graphics)
		graphics->Render();

	glGetFloatv(GL_MODELVIEW, matrix);


/*		FILLSTYLE::fixedColor(0,0,0);
	glBegin(GL_QUADS);
		glVertex2i(0,0);
		glVertex2i(75,0);
		glVertex2i(75,75);
		glVertex2i(0,75);
	glEnd();*/

	glEnable(GL_BLEND);
	glLoadIdentity();
	GLenum draw_buffers[]={GL_COLOR_ATTACHMENT0_EXT,GL_COLOR_ATTACHMENT2_EXT};
	glDrawBuffers(2,draw_buffers);

	glBindTexture(GL_TEXTURE_2D,rt->spare_tex);
	glColor3f(0,0,1);
	glBegin(GL_QUADS);
		glTexCoord2f(0,1);
		glVertex2i(0,0);
		glTexCoord2f(1,1);
		glVertex2i(rt->width,0);
		glTexCoord2f(1,0);
		glVertex2i(rt->width,rt->height);
		glTexCoord2f(0,0);
		glVertex2i(0,rt->height);
	glEnd();
	glPopMatrix();

	//Now draw also the display list
	list<IDisplayListElem*>::iterator it=dynamicDisplayList.begin();
	for(it;it!=dynamicDisplayList.end();it++)
		(*it)->Render();
}

ASFUNCTIONBODY(Sprite,_constructor)
{
	Sprite* th=static_cast<Sprite*>(obj->implementation);
	DisplayObjectContainer::_constructor(obj,NULL);

	return NULL;
}

ASFUNCTIONBODY(Sprite,_getGraphics)
{
	Sprite* th=static_cast<Sprite*>(obj->implementation);
	//Probably graphics is not used often, so create it here
	if(th->graphics==NULL)
		th->graphics=Class<Graphics>::getInstanceS(true);

	th->graphics->obj->incRef();
	return th->graphics->obj;
}

void MovieClip::sinit(Class_base* c)
{
	assert(c->constructor==NULL);
	c->constructor=new Function(_constructor);
	c->super=Class<Sprite>::getClass();
	c->max_level=c->super->max_level+1;
}

void MovieClip::buildTraits(ASObject* o)
{
	o->setGetterByQName("currentFrame","",new Function(_getCurrentFrame));
	o->setGetterByQName("totalFrames","",new Function(_getTotalFrames));
	o->setGetterByQName("framesLoaded","",new Function(_getFramesLoaded));
	o->setVariableByQName("stop","",new Function(stop));
	o->setVariableByQName("nextFrame","",new Function(nextFrame));
}

MovieClip::MovieClip():framesLoaded(0),totalFrames(1),cur_frame(&dynamicDisplayList)
{
}

void MovieClip::addToFrame(DisplayListTag* t)
{
/*	list<IDisplayListElem*>::iterator it=lower_bound(displayList.begin(),displayList.end(),t->getDepth(),list_orderer);
	displayList.insert(it,t);
	displayListLimit=displayList.size();

	t->root=root;
	ASObject* o=static_cast<ASObject*>(t);
	if(o)
		o->setVariableByName("root",this,true);*/

	cur_frame.blueprint.push_back(t);
}

ASFUNCTIONBODY(MovieClip,addFrameScript)
{
	abort();
/*	MovieClip* th=static_cast<MovieClip*>(obj->implementation);
	if(args->size()%2)
	{
		LOG(LOG_ERROR,"Invalid arguments to addFrameScript");
		abort();
	}
	for(int i=0;i<args->size();i+=2)
	{
		int f=args->at(i+0)->toInt();
		IFunction* g=args->at(i+1)->toFunction();

		//Should wait for frames to be received
		if(f>=th->frames.size())
		{
			LOG(LOG_ERROR,"Invalid frame number passed to addFrameScript");
			abort();
		}

		th->frames[f].setScript(g);
	}*/
	return NULL;
}

ASFUNCTIONBODY(MovieClip,createEmptyMovieClip)
{
	LOG(LOG_NOT_IMPLEMENTED,"createEmptyMovieClip");
	return new Undefined;
/*	MovieClip* th=static_cast<MovieClip*>(obj);
	if(th==NULL)
		LOG(ERROR,"Not a valid MovieClip");

	LOG(CALLS,"Called createEmptyMovieClip: " << args->args[0]->toString() << " " << args->args[1]->toString());
	MovieClip* ret=new MovieClip();

	IDisplayListElem* t=new ASObjectWrapper(ret,args->args[1]->toInt());
	list<IDisplayListElem*>::iterator it=lower_bound(th->dynamicDisplayList.begin(),th->dynamicDisplayList.end(),t->getDepth(),list_orderer);
	th->dynamicDisplayList.insert(it,t);

	th->setVariableByName(args->args[0]->toString(),ret);
	return ret;*/
}

ASFUNCTIONBODY(MovieClip,moveTo)
{
	LOG(LOG_NOT_IMPLEMENTED,"Called moveTo");
	return NULL;
}

ASFUNCTIONBODY(MovieClip,lineTo)
{
	LOG(LOG_NOT_IMPLEMENTED,"Called lineTo");
	return NULL;
}

ASFUNCTIONBODY(MovieClip,lineStyle)
{
	LOG(LOG_NOT_IMPLEMENTED,"Called lineStyle");
	return NULL;
}

ASFUNCTIONBODY(MovieClip,swapDepths)
{
	LOG(LOG_NOT_IMPLEMENTED,"Called swapDepths");
	return NULL;
}

ASFUNCTIONBODY(MovieClip,stop)
{
	MovieClip* th=static_cast<MovieClip*>(obj->implementation);
	th->state.stop_FP=true;
	return NULL;
}

ASFUNCTIONBODY(MovieClip,nextFrame)
{
	MovieClip* th=static_cast<MovieClip*>(obj->implementation);
	assert(th->state.FP<th->state.max_FP);
	sys->currentVm->addEvent(NULL,new FrameChangeEvent(th->state.FP+1,th));
	return NULL;
}

ASFUNCTIONBODY(MovieClip,_getFramesLoaded)
{
	MovieClip* th=static_cast<MovieClip*>(obj->implementation);
	//currentFrame is 1-based
	return new Integer(th->framesLoaded);
}

ASFUNCTIONBODY(MovieClip,_getTotalFrames)
{
	MovieClip* th=static_cast<MovieClip*>(obj->implementation);
	//currentFrame is 1-based
	return new Integer(th->totalFrames);
}

ASFUNCTIONBODY(MovieClip,_getCurrentFrame)
{
	MovieClip* th=static_cast<MovieClip*>(obj->implementation);
	//currentFrame is 1-based
	return new Integer(th->state.FP+1);
}

ASFUNCTIONBODY(MovieClip,_constructor)
{
	MovieClip* th=static_cast<MovieClip*>(obj->implementation);
	Sprite::_constructor(obj,NULL);
/*	th->setVariableByQName("swapDepths","",new Function(swapDepths));
	th->setVariableByQName("lineStyle","",new Function(lineStyle));
	th->setVariableByQName("lineTo","",new Function(lineTo));
	th->setVariableByQName("moveTo","",new Function(moveTo));
	th->setVariableByQName("createEmptyMovieClip","",new Function(createEmptyMovieClip));
	th->setVariableByQName("addFrameScript","",new Function(addFrameScript));*/
	return NULL;
}

void MovieClip::advanceFrame()
{
	if(!state.stop_FP || state.explicit_FP /*&& (class_name=="MovieClip")*/)
	{
		//Before assigning the next_FP we initialize the frame
		frames[state.next_FP].init(this,displayList);
		state.FP=state.next_FP;
		if(!state.stop_FP)
			state.next_FP=min(state.FP+1,frames.size()-1); //TODO: use framecount
		state.explicit_FP=false;
	}

}

void MovieClip::Render()
{
	LOG(LOG_TRACE,"Render MovieClip");
	//MovieClip* clip_bak=rt->currentClip;
	//rt->currentClip=this;

	advanceFrame();

	assert(state.FP<frames.size());

	if(!state.stop_FP)
		frames[state.FP].runScript();

	//Set the id in the secondary color
	glPushAttrib(GL_CURRENT_BIT);
	glSecondaryColor3f(id,0,0);

	//Apply local transformation
	glPushMatrix();
	//glTranslatef(_x,_y,0);
	//glRotatef(rotation,0,0,1);
	frames[state.FP].Render();

	glPopMatrix();
	glPopAttrib();

	LOG(LOG_TRACE,"End Render MovieClip");

	//rt->currentClip=clip_bak;
}

void MovieClip::getBounds(number_t& xmin, number_t& xmax, number_t& ymin, number_t& ymax)
{
	//Iterate over the displaylist of the current frame
	//TODO: add dynamic dysplay list
	std::list<std::pair<PlaceInfo, IDisplayListElem*> >::iterator it=frames[state.FP].displayList.begin();
	assert(frames[state.FP].displayList.size()==1);
	it->second->getBounds(xmin,xmax,ymin,ymax);
	//TODO: take rotation into account
	Matrix.multiply2D(xmin,ymin,xmin,ymin);
	Matrix.multiply2D(xmax,ymax,xmax,ymax);
}

DisplayObject::DisplayObject():height(0),width(0),loaderInfo(NULL),rotation(0.0)
{
}

void DisplayObject::sinit(Class_base* c)
{
	assert(c->constructor==NULL);
	c->constructor=new Function(_constructor);
	c->super=Class<EventDispatcher>::getClass();
	c->max_level=c->super->max_level+1;
}

void DisplayObject::buildTraits(ASObject* o)
{
	o->setGetterByQName("loaderInfo","",new Function(_getLoaderInfo));
	o->setGetterByQName("width","",new Function(_getWidth));
	o->setSetterByQName("width","",new Function(_setWidth));
	o->setGetterByQName("scaleX","",new Function(_getScaleX));
	o->setSetterByQName("scaleX","",new Function(undefinedFunction));
	o->setGetterByQName("scaleY","",new Function(_getScaleY));
	o->setSetterByQName("scaleY","",new Function(undefinedFunction));
	o->setGetterByQName("x","",new Function(_getX));
	o->setSetterByQName("x","",new Function(_setX));
	o->setGetterByQName("y","",new Function(_getY));
	o->setSetterByQName("y","",new Function(_setY));
	o->setGetterByQName("height","",new Function(_getHeight));
	o->setSetterByQName("height","",new Function(undefinedFunction));
	o->setGetterByQName("visible","",new Function(_getVisible));
	o->setSetterByQName("visible","",new Function(undefinedFunction));
	o->setGetterByQName("rotation","",new Function(_getRotation));
	o->setSetterByQName("rotation","",new Function(undefinedFunction));
	o->setGetterByQName("name","",new Function(_getName));
	o->setGetterByQName("parent","",new Function(_getParent));
	o->setGetterByQName("root","",new Function(_getRoot));
	o->setGetterByQName("blendMode","",new Function(_getBlendMode));
	o->setSetterByQName("blendMode","",new Function(undefinedFunction));
	o->setGetterByQName("scale9Grid","",new Function(_getScale9Grid));
	o->setSetterByQName("scale9Grid","",new Function(undefinedFunction));
	o->setGetterByQName("stage","",new Function(_getStage));
	o->setVariableByQName("getBounds","",new Function(_getBounds));
	o->setVariableByQName("localToGlobal","",new Function(localToGlobal));
	o->setSetterByQName("name","",new Function(_setName));
	o->setGetterByQName("mask","",new Function(undefinedFunction));
	o->setSetterByQName("mask","",new Function(undefinedFunction));
	o->setGetterByQName("alpha","",new Function(undefinedFunction));
	o->setSetterByQName("alpha","",new Function(undefinedFunction));
	o->setGetterByQName("cacheAsBitmap","",new Function(undefinedFunction));
	o->setSetterByQName("cacheAsBitmap","",new Function(undefinedFunction));
	o->setGetterByQName("opaqueBackground","",new Function(undefinedFunction));
	o->setSetterByQName("opaqueBackground","",new Function(undefinedFunction));
}

ASFUNCTIONBODY(DisplayObject,_getScaleX)
{
	DisplayObject* th=static_cast<DisplayObject*>(obj->implementation);
	return abstract_d(th->Matrix.ScaleX);
}

ASFUNCTIONBODY(DisplayObject,_getScaleY)
{
	DisplayObject* th=static_cast<DisplayObject*>(obj->implementation);
	return abstract_d(th->Matrix.ScaleY);
}

ASFUNCTIONBODY(DisplayObject,_getX)
{
	DisplayObject* th=static_cast<DisplayObject*>(obj->implementation);
	return abstract_d(th->Matrix.TranslateX);
}

ASFUNCTIONBODY(DisplayObject,_setX)
{
	DisplayObject* th=static_cast<DisplayObject*>(obj->implementation);
	assert(args && args->size()==1);
	th->Matrix.TranslateX=args->at(0)->toInt();
	return NULL;
}

ASFUNCTIONBODY(DisplayObject,_getY)
{
	DisplayObject* th=static_cast<DisplayObject*>(obj->implementation);
	return abstract_d(th->Matrix.TranslateY);
}

ASFUNCTIONBODY(DisplayObject,_setY)
{
	DisplayObject* th=static_cast<DisplayObject*>(obj->implementation);
	assert(args && args->size()==1);
	th->Matrix.TranslateY=args->at(0)->toInt();
	return NULL;
}

ASFUNCTIONBODY(DisplayObject,_getBounds)
{
	DisplayObject* th=static_cast<DisplayObject*>(obj->implementation);
	Rectangle* ret=Class<Rectangle>::getInstanceS(true);
	number_t x1,x2,y1,y2;
	th->getBounds(x1,x2,y1,y2);

	//Bounds are in the form [XY]{min,max}
	//convert it to rect (x,y,width,height) representation
	ret->x=x1;
	ret->width=x2-x1;
	ret->y=y1;
	ret->height=y2-y1;
	return ret->obj;
}

ASFUNCTIONBODY(DisplayObject,_constructor)
{
	DisplayObject* th=static_cast<DisplayObject*>(obj->implementation);
	EventDispatcher::_constructor(obj,NULL);

	return NULL;
}

ASFUNCTIONBODY(DisplayObject,_getLoaderInfo)
{
	DisplayObject* th=static_cast<DisplayObject*>(obj->implementation);
	if(th->loaderInfo)
	{
		th->loaderInfo->obj->incRef();
		return th->loaderInfo->obj;
	}
	else
		return new Undefined;
}

ASFUNCTIONBODY(DisplayObject,_getStage)
{
	assert(sys->stage);
	sys->stage->obj->incRef();
	return sys->stage->obj;
}

ASFUNCTIONBODY(DisplayObject,_getScale9Grid)
{
	DisplayObject* th=static_cast<DisplayObject*>(obj->implementation);
	return new Undefined;
}

ASFUNCTIONBODY(DisplayObject,_getBlendMode)
{
	DisplayObject* th=static_cast<DisplayObject*>(obj->implementation);
	return new Undefined;
}

ASFUNCTIONBODY(DisplayObject,localToGlobal)
{
	DisplayObject* th=static_cast<DisplayObject*>(obj->implementation);
	return new Undefined;
}

ASFUNCTIONBODY(DisplayObject,_setRotation)
{
	DisplayObject* th=static_cast<DisplayObject*>(obj->implementation);
	th->rotation=args->at(0)->toNumber();
	return NULL;
}

ASFUNCTIONBODY(DisplayObject,_setWidth)
{
	DisplayObject* th=static_cast<DisplayObject*>(obj->implementation);
	th->width=args->at(0)->toInt();
	abort();
	return NULL;
}

ASFUNCTIONBODY(DisplayObject,_setName)
{
	DisplayObject* th=static_cast<DisplayObject*>(obj->implementation);
	return NULL;
}

ASFUNCTIONBODY(DisplayObject,_getName)
{
	DisplayObject* th=static_cast<DisplayObject*>(obj->implementation);
	return new Undefined;
}

ASFUNCTIONBODY(DisplayObject,_getParent)
{
	DisplayObject* th=static_cast<DisplayObject*>(obj->implementation);
	if(th->parent==NULL)
		return new Undefined;

	th->parent->obj->incRef();
	return th->parent->obj;
}

ASFUNCTIONBODY(DisplayObject,_getRoot)
{
	DisplayObject* th=static_cast<DisplayObject*>(obj->implementation);
	if(th->root)
	{
		th->root->obj->incRef();
		return th->root->obj;
	}
	else
		return new Undefined;
}

ASFUNCTIONBODY(DisplayObject,_getRotation)
{
	DisplayObject* th=static_cast<DisplayObject*>(obj->implementation);
	return new Number(th->rotation);
}

ASFUNCTIONBODY(DisplayObject,_getVisible)
{
	DisplayObject* th=static_cast<DisplayObject*>(obj->implementation);
	return abstract_b(true);
}

ASFUNCTIONBODY(DisplayObject,_getWidth)
{
	DisplayObject* th=static_cast<DisplayObject*>(obj->implementation);

	//DEBUG
	Sprite* sp=dynamic_cast<Sprite*>(th);
	if(sp)
		assert(sp->graphics==NULL && sp->dynamicDisplayList.empty());
	return abstract_i(th->width);
}

ASFUNCTIONBODY(DisplayObject,_getHeight)
{
	DisplayObject* th=static_cast<DisplayObject*>(obj->implementation);
	return abstract_i(th->height);
}

void DisplayObjectContainer::sinit(Class_base* c)
{
	assert(c->constructor==NULL);
	c->constructor=new Function(_constructor);
	c->super=Class<DisplayObject>::getClass();
	c->max_level=c->super->max_level+1;
}

void DisplayObjectContainer::buildTraits(ASObject* o)
{
	o->setGetterByQName("numChildren","",new Function(_getNumChildren));
	o->setVariableByQName("getChildIndex","",new Function(getChildIndex));
	o->setVariableByQName("getChildAt","",new Function(getChildAt));
	o->setVariableByQName("addChild","",new Function(addChild));
	o->setVariableByQName("addChildAt","",new Function(addChildAt));
}

DisplayObjectContainer::DisplayObjectContainer()
{
}

ASFUNCTIONBODY(DisplayObjectContainer,_constructor)
{
	DisplayObject::_constructor(obj,NULL);
	return NULL;
}

ASFUNCTIONBODY(DisplayObjectContainer,_getNumChildren)
{
	return new Integer(0);;
}

void DisplayObjectContainer::_addChildAt(DisplayObject* child, int index)
{
	//Set the root of the movie to this container
	assert(child->root==NULL);
	child->root=root;

	//The HACK for this supports only Sprites now
	assert(child->obj->prototype->isSubClass(Class<Sprite>::getClass()));

//	if(child->obj->prototype->class_name=="BorderSkin")
//		abort();

	//If the child has no parent, set this container to parent
	//If there is a previous parent, purge the child from his list
	if(child->parent)
	{
		//Child already in this container
		if(child->parent==this)
			return;
		else
			child->parent->_removeChild(child);
	}
	child->parent=this;

	//We insert the object in the back of the list
	//TODO: support the 'at index' version of the call
	dynamicDisplayList.push_back(child);
}

void DisplayObjectContainer::_removeChild(IDisplayListElem* child)
{
	assert(child->parent==this);
	assert(child->root==root);

	list<IDisplayListElem*>::iterator it=find(dynamicDisplayList.begin(),dynamicDisplayList.end(),child);
	assert(it!=dynamicDisplayList.end());
	dynamicDisplayList.erase(it);
}

ASFUNCTIONBODY(DisplayObjectContainer,addChildAt)
{
	DisplayObjectContainer* th=static_cast<DisplayObjectContainer*>(obj->implementation);
	assert(args->size()==2);
	//Validate object type
	assert(args->at(0)->prototype->isSubClass(Class<Sprite>::getClass()));
	args->at(0)->incRef();

	//Cast to object
	DisplayObject* d=static_cast<DisplayObject*>(args->at(0)->implementation);
	th->_addChildAt(d,0);

	//Notify the object
	d->obj->incRef();
	sys->currentVm->addEvent(d,Class<Event>::getInstanceS(true,"added",d->obj));

	return d->obj;
}

ASFUNCTIONBODY(DisplayObjectContainer,addChild)
{
	DisplayObjectContainer* th=static_cast<DisplayObjectContainer*>(obj->implementation);
	assert(args->size()==1);
	//Validate object type
	assert(args->at(0)->prototype->isSubClass(Class<Sprite>::getClass()));
	args->at(0)->incRef();

	//Cast to object
	DisplayObject* d=static_cast<DisplayObject*>(args->at(0)->implementation);
	th->_addChildAt(d,0);

	//Notify the object
	d->obj->incRef();
	sys->currentVm->addEvent(d,Class<Event>::getInstanceS(true,"added",d->obj));

	return d->obj;
}

ASFUNCTIONBODY(DisplayObjectContainer,getChildAt)
{
	DisplayObjectContainer* th=static_cast<DisplayObjectContainer*>(obj->implementation);
	assert(args->size()==1);
	int index=args->at(0)->toInt();
	assert(index<th->dynamicDisplayList.size());
	list<IDisplayListElem*>::iterator it=th->dynamicDisplayList.begin();
	for(int i=0;i<index;i++)
		it++;

	(*it)->obj->incRef();
	return (*it)->obj;
}

ASFUNCTIONBODY(DisplayObjectContainer,getChildIndex)
{
	DisplayObjectContainer* th=static_cast<DisplayObjectContainer*>(obj->implementation);
	assert(args->size()==1);
	//Validate object type
	assert(args->at(0)->prototype->isSubClass(Class<DisplayObject>::getClass()));

	//Cast to object
	DisplayObject* d=static_cast<DisplayObject*>(args->at(0)->implementation);

	list<IDisplayListElem*>::const_iterator it=th->dynamicDisplayList.begin();
	int ret=0;
	do
	{
		if(*it==d)
			break;
		
		ret++;
		it++;
		assert(it!=th->dynamicDisplayList.end());
	}
	while(1);
	return abstract_i(ret);
}

void Shape::sinit(Class_base* c)
{
	assert(c->constructor==NULL);
	c->constructor=new Function(_constructor);
	c->super=Class<DisplayObject>::getClass();
	c->max_level=c->super->max_level+1;
}

void Shape::buildTraits(ASObject* o)
{
}

ASFUNCTIONBODY(Shape,_constructor)
{
	DisplayObject::_constructor(obj,NULL);
	return NULL;
}

void Stage::sinit(Class_base* c)
{
	assert(c->constructor==NULL);
	c->constructor=new Function(_constructor);
	c->super=Class<DisplayObjectContainer>::getClass();
	c->max_level=c->super->max_level+1;
}

void Stage::buildTraits(ASObject* o)
{
	o->setGetterByQName("stageWidth","",new Function(_getStageWidth));
	o->setGetterByQName("stageHeight","",new Function(_getStageHeight));
}

Stage::Stage()
{
}

ASFUNCTIONBODY(Stage,_constructor)
{
	return NULL;
}

ASFUNCTIONBODY(Stage,_getStageWidth)
{
	Stage* th=static_cast<Stage*>(obj->implementation);
	RECT size=sys->getFrameSize();
	int width=size.Xmax/20;
	return abstract_d(width);
}

ASFUNCTIONBODY(Stage,_getStageHeight)
{
	Stage* th=static_cast<Stage*>(obj->implementation);
	RECT size=sys->getFrameSize();
	int height=size.Ymax/20;
	return abstract_d(height);
}

void Graphics::sinit(Class_base* c)
{
	assert(c->constructor==NULL);
	c->constructor=new Function(_constructor);
}

void Graphics::buildTraits(ASObject* o)
{
	o->setVariableByQName("clear","",new Function(clear));
	o->setVariableByQName("drawRect","",new Function(drawRect));
	o->setVariableByQName("beginFill","",new Function(beginFill));
}

ASFUNCTIONBODY(Graphics,_constructor)
{
}

ASFUNCTIONBODY(Graphics,clear)
{
	Graphics* th=static_cast<Graphics*>(obj->implementation);
	sem_wait(&th->geometry_mutex);
	th->geometry.clear();
	sem_post(&th->geometry_mutex);
}

ASFUNCTIONBODY(Graphics,drawRect)
{
	Graphics* th=static_cast<Graphics*>(obj->implementation);
	assert(args->size()==4);

	int x=args->at(0)->toInt();
	int y=args->at(1)->toInt();
	int width=args->at(2)->toInt();
	int height=args->at(3)->toInt();

	//Build a shape and add it to the geometry vector
	GeomShape tmpShape;
	tmpShape.outline.push_back(Vector2(x,y));
	tmpShape.outline.push_back(Vector2(x+width,y));
	tmpShape.outline.push_back(Vector2(x+width,y+height));
	tmpShape.outline.push_back(Vector2(x,y+height));
	tmpShape.outline.push_back(Vector2(x,y));

	sem_wait(&th->geometry_mutex);
	th->geometry.push_back(tmpShape);
	sem_post(&th->geometry_mutex);
	return NULL;
}

ASFUNCTIONBODY(Graphics,beginFill)
{
	Graphics* th=static_cast<Graphics*>(obj->implementation);
	if(args->size()>=1)
		cout << "Color " << hex << args->at(0)->toInt() << dec << endl;
	if(args->size()>=2)
		cout << "Alpha " << args->at(1)->toNumber() << endl;
	return NULL;
}

void Graphics::Render()
{
	sem_wait(&geometry_mutex);

	for(int i=0;i<geometry.size();i++)
		geometry[i].Render();

/*	if(geometry.size()==1)
	{
		FILLSTYLE::fixedColor(0,0,0);
		glBegin(GL_QUADS);
			glVertex2i(0,0);
			glVertex2i(75,0);
			glVertex2i(75,75);
			glVertex2i(0,75);
		glEnd();
	}*/
	sem_post(&geometry_mutex);
}

void LineScaleMode::sinit(Class_base* c)
{
	c->setVariableByQName("HORIZONTAL","",Class<ASString>::getInstanceS(true,"horizontal")->obj);
	c->setVariableByQName("NONE","",Class<ASString>::getInstanceS(true,"none")->obj);
	c->setVariableByQName("NORMAL","",Class<ASString>::getInstanceS(true,"normal")->obj);
	c->setVariableByQName("VERTICAL","",Class<ASString>::getInstanceS(true,"vertical")->obj);
}

void StageScaleMode::sinit(Class_base* c)
{
	c->setVariableByQName("EXACT_FIT","",Class<ASString>::getInstanceS(true,"exactFit")->obj);
	c->setVariableByQName("NO_BORDER","",Class<ASString>::getInstanceS(true,"noBorder")->obj);
	c->setVariableByQName("NO_SCALE","",Class<ASString>::getInstanceS(true,"noScale")->obj);
	c->setVariableByQName("SHOW_ALL","",Class<ASString>::getInstanceS(true,"showAll")->obj);
}

void StageAlign::sinit(Class_base* c)
{
	c->setVariableByQName("TOP_LEFT","",Class<ASString>::getInstanceS(true,"TL")->obj);
}
