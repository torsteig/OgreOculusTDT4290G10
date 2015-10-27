#ifndef __OgreOculus_h_
#define __OgreOculus_h_

#ifdef _WIN32
#define mainFunc() int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,LPSTR lpCmdLine, int nCmdShow)
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <glew.h>
#else
#define mainFunc() int main(void)
#endif
#include <iostream>
#include <Ogre.h>
#include <OIS.h>
#include <OVR.h>
#include <RenderSystems/GL/OgreGLTextureManager.h>
#include <RenderSystems/GL/OgreGLRenderSystem.h>
#include <RenderSystems/GL/OgreGLTexture.h>
#include <OVR_CAPI.h>
#include <OVR_CAPI_GL.h>
#include <OVR_CAPI_0_7_0.h>

class OgreOculus : public Ogre::FrameListener, public Ogre::WindowEventListener, public OIS::KeyListener, public OIS::MouseListener
{
public:
	OgreOculus(void);
	virtual ~OgreOculus(void);

	virtual int go(void);

protected:
	virtual bool keyPressed(const OIS::KeyEvent &arg);
	virtual bool keyReleased(const OIS::KeyEvent &arg);
    virtual bool mouseMoved(const OIS::MouseEvent &arg);
    virtual bool mousePressed(const OIS::MouseEvent &arg, OIS::MouseButtonID id);
    virtual bool mouseReleased(const OIS::MouseEvent &arg, OIS::MouseButtonID id);

	Ogre::Root*			root;
	ovrHmd				hmd;
};

#endif