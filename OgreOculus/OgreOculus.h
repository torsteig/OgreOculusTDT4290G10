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

// For physics loop
#include <thread>
#include "PhysicsLoop.h"

// For separate Ogre model creation
#include "OgreModel.h"

enum eyes{left, right, nbEyes};

class OgreOculus : public Ogre::WindowEventListener, public OIS::KeyListener, public OIS::MouseListener
{
public:
	OgreOculus(void);
	virtual ~OgreOculus(void);
	virtual int go(void);

protected:
	virtual void createEventListener(void);
	virtual void createCamera(void);
	virtual void windowClosed(Ogre::RenderWindow* rw);

	virtual bool keyPressed(const OIS::KeyEvent &ke);
	virtual bool keyReleased(const OIS::KeyEvent &ke);
    virtual bool mouseMoved(const OIS::MouseEvent &me);
    virtual bool mousePressed(const OIS::MouseEvent &me, OIS::MouseButtonID id);
    virtual bool mouseReleased(const OIS::MouseEvent &me, OIS::MouseButtonID id);

	// Ogre variables
	Ogre::Root*					root;
	Ogre::SceneManager*			smgr;
	Ogre::RenderWindow*			window;
	Ogre::SceneNode*			mPlayerNode;
	Ogre::SceneNode*			mHeadNode;
	Ogre::Camera*				mCamera;
	Ogre::Vector3				mDirection;
	Ogre::Real					mMove;
	Ogre::Real					mRotate;
	Ogre::Camera*				cams[nbEyes];
	Ogre::Quaternion			initialOculusOrientation;
	Ogre::Real					headPositionTrackingSensitivity;

	// Oculus variables
	ovrHmd						hmd;
	OVR::Quatf					oculusOrient;
	OVR::Vector3f				oculusPos;

	// OIS variables
	OIS::InputManager*			mInputManager;
	OIS::Mouse*					mMouse;
	OIS::Keyboard*				mKeyboard;
};

#endif