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
#include <OVR_CAPI_0_7_0.h>rr

enum eyes{left, right, nbEyes};

int max(int a, int b)
{
	if (a > b) return a; return b;
}

mainFunc()
{
	//Create Root object 
	Ogre::Root* root = new Ogre::Root("plugin.cfg", "ogre.cfg");
	//opengl
    root->loadPlugin("RenderSystem_GL_d");
    root->setRenderSystem(root->getRenderSystemByName("OpenGL Rendering Subsystem"));

	//Initialize Root
	root->initialise(false);

	//initialize oculus
	ovrHmd hmd;
	ovrHmdDesc hmdDesc;
	ovrGraphicsLuid luid;
	ovr_Initialize(nullptr);
	if(ovr_Create(&hmd, &luid) != ovrSuccess)
		exit(-1);
	hmdDesc = ovr_GetHmdDesc(hmd);
	if(ovr_ConfigureTracking(hmd,
		ovrTrackingCap_Orientation |ovrTrackingCap_MagYawCorrection |ovrTrackingCap_Position,
		0) != ovrSuccess)
		exit(-2);

	// Turn off HUD
	ovr_SetInt(hmd, "PerfHudMode", ovrPerfHud_Off);

	//create a window
	Ogre::RenderWindow* window = root->createRenderWindow("Ogre + Oculus = <3", hmdDesc.Resolution.w/2, hmdDesc.Resolution.h/2, false);

	//Create scene manager and cameras
	Ogre::SceneManager* smgr = root->createSceneManager(Ogre::ST_GENERIC);
	Ogre::Camera* cams[nbEyes];
	
	//OCR cameras
	cams[left] = smgr->createCamera("leftcam");
	cams[right] = smgr->createCamera("rightcam");

	//setup body and head cam
	Ogre::SceneNode* mBodyNode = smgr->getRootSceneNode()->createChildSceneNode("BodyNode");
	Ogre::SceneNode*  mBodyTiltNode = mBodyNode->createChildSceneNode();
	Ogre::SceneNode*  mHeadNode = mBodyTiltNode->createChildSceneNode("HeadNode"); 
	mBodyNode->setFixedYawAxis( true );	// don't roll! 

	mHeadNode->attachObject(cams[left]);
	mHeadNode->attachObject(cams[right]);
	
	//set cam position
	float dist = 0.05;
	cams[right]->setPosition(Ogre::Vector3(dist/2.0f, 0.0f, 0.0f));
	
	cams[right]->setNearClipDistance(0.001);

	cams[left]->setPosition(Ogre::Vector3(-dist/2.0f, 0.0f,0.0f));
	
	cams[left]->setNearClipDistance(0.001);

	mBodyNode->setPosition( 30.0,50.0, 100.0 ); //view is different from window camera
	mBodyNode->lookAt(Ogre::Vector3::ZERO, Ogre::SceneNode::TS_WORLD);
	

	// Load Ogre resource paths from config file
    Ogre::ConfigFile cf;
    cf.load("resources_d.cfg");

    // Go through all sections & settings in the file
    Ogre::ConfigFile::SectionIterator seci = cf.getSectionIterator();

    Ogre::String secName, typeName, archName;
    while (seci.hasMoreElements())
    {
        secName = seci.peekNextKey();
        Ogre::ConfigFile::SettingsMultiMap *settings = seci.getNext();
        Ogre::ConfigFile::SettingsMultiMap::iterator i;
        for (i = settings->begin(); i != settings->end(); ++i)
        {
            typeName = i->first;
            archName = i->second;

            Ogre::ResourceGroupManager::getSingleton().addResourceLocation(
                archName, typeName, secName);
        }
    }

	// Set resources
	Ogre::TextureManager::getSingleton().setDefaultNumMipmaps(5);
	Ogre::ResourceGroupManager::getSingleton().initialiseAllResourceGroups();
	 
	/*---------------------------------------------*/
	/* OGRE MODEL CREATION BEGINS HERE              */
	/*---------------------------------------------*/

	smgr->setAmbientLight(Ogre::ColourValue(0.5, 0.5, 0.5));
	Ogre::Entity* ogreEntity = smgr->createEntity("ogrehead.mesh");
	Ogre::SceneNode* ogreNode = smgr->getRootSceneNode()->createChildSceneNode();
	ogreNode->attachObject(ogreEntity);

	/*---------------------------------------------*/
	/* OGRE MODEL CREATION ENDS HERE                */
	/*---------------------------------------------*/

	// Camera part here
	Ogre::Camera* mCamera = smgr->createCamera("PlayerCam");
	mCamera->setPosition(Ogre::Vector3(0, 50, 100));
	mCamera->lookAt(Ogre::Vector3(0, 0, 0));
	mCamera->setNearClipDistance(5);

	// Viewport and scene (is this even used?)
	Ogre::Viewport* vp = window->addViewport(mCamera);
	vp->setBackgroundColour(Ogre::ColourValue(34, 89, 0)); //yellow

	// Some other camera stuff. Not sure if needed
	mCamera->setAspectRatio(
    Ogre::Real(vp->getActualWidth()) /
    Ogre::Real(vp->getActualHeight()));

	//init glew
	if(glewInit() != GLEW_OK)
		exit(-3);
	
	//get texture sizes
	ovrSizei texSizeL, texSizeR;
	texSizeL = ovr_GetFovTextureSize(hmd, ovrEye_Left, hmdDesc.DefaultEyeFov[left], 1);
	texSizeR = ovr_GetFovTextureSize(hmd, ovrEye_Right, hmdDesc.DefaultEyeFov[right], 1);

	//calculate render buffer size
	ovrSizei bufferSize;
	bufferSize.w = texSizeL.w + texSizeR.w;
	bufferSize.h = max(texSizeL.h, texSizeR.h);

	//create render texture set
	ovrSwapTextureSet* textureSet;
	if(ovr_CreateSwapTextureSetGL(hmd, GL_RGB, bufferSize.w, bufferSize.h, &textureSet) != ovrSuccess)
		exit(-4);

	//create ogre render texture
	Ogre::GLTextureManager* textureManager = static_cast<Ogre::GLTextureManager*>(Ogre::GLTextureManager::getSingletonPtr());
	Ogre::TexturePtr rtt_texture(textureManager->createManual("RttTex", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, 
		Ogre::TEX_TYPE_2D, bufferSize.w, bufferSize.h, 0, Ogre::PF_R8G8B8, Ogre::TU_RENDERTARGET));
	Ogre::RenderTexture* rttEyes = rtt_texture->getBuffer(0, 0)->getRenderTarget();
	Ogre::GLTexture* gltex = static_cast<Ogre::GLTexture*>(Ogre::GLTextureManager::getSingleton().getByName("RttTex").getPointer());
	GLuint renderTextureID = gltex->getGLID();

	//put camera viewport on the ogre render texture
	Ogre::Viewport* vpts[nbEyes];
	vpts[left]=rttEyes->addViewport(cams[left], 0, 0, 0, 0.5f);
	vpts[right]=rttEyes->addViewport(cams[right], 1, 0.5f, 0, 0.5f);
	vpts[left]->setBackgroundColour(Ogre::ColourValue(34, 89, 0)); // Black background 
	vpts[right]->setBackgroundColour(Ogre::ColourValue(34, 89, 0));
	
	ovrTexture* mirrorTexture;
	if(ovr_CreateMirrorTextureGL(hmd, GL_RGB, hmdDesc.Resolution.w, hmdDesc.Resolution.h, &mirrorTexture) != ovrSuccess)
		exit(-5);
	Ogre::TexturePtr mirror_texture(textureManager->createManual("MirrorTex", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, 
		Ogre::TEX_TYPE_2D, hmdDesc.Resolution.w, hmdDesc.Resolution.h, 0, Ogre::PF_R8G8B8, Ogre::TU_RENDERTARGET));

	//get GLIDs
	GLuint ogreMirrorTextureID = static_cast<Ogre::GLTexture*>(Ogre::GLTextureManager::getSingleton().getByName("MirrorTex").getPointer())->getGLID();
	GLuint oculusMirrorTextureID = ((ovrGLTexture*)mirrorTexture)->OGL.TexId;

	//Create EyeRenderDesc
	ovrEyeRenderDesc EyeRenderDesc[nbEyes];
	EyeRenderDesc[left] = ovr_GetRenderDesc(hmd, ovrEye_Left, hmdDesc.DefaultEyeFov[left]);
	EyeRenderDesc[right] = ovr_GetRenderDesc(hmd, ovrEye_Right, hmdDesc.DefaultEyeFov[right]);

	//Get offsets
	ovrVector3f offset[nbEyes];
	offset[left]=EyeRenderDesc[left].HmdToEyeViewOffset;
	offset[right]=EyeRenderDesc[right].HmdToEyeViewOffset;

	//Compositor layer
	ovrLayerEyeFov layer;
	layer.Header.Type = ovrLayerType_EyeFov;
	layer.Header.Flags = 0;
	layer.ColorTexture[left] = textureSet;
	layer.ColorTexture[right] = textureSet;
	layer.Fov[left] = EyeRenderDesc[left].Fov;
	layer.Fov[right] = EyeRenderDesc[right].Fov;
	layer.Viewport[left] = OVR::Recti(0, 0, bufferSize.w/2, bufferSize.h);
	layer.Viewport[right] = OVR::Recti(bufferSize.w/2, 0, bufferSize.w/2, bufferSize.h);

	//Get projection matrices
	for(size_t eyeIndex(0); eyeIndex < ovrEye_Count; eyeIndex++)
	{
		//Get the projection matrix
		OVR::Matrix4f proj = ovrMatrix4f_Projection(EyeRenderDesc[eyeIndex].Fov, 
			static_cast<float>(0.01f), 
			4000, 
			true);

		//Convert it to Ogre matrix
		Ogre::Matrix4 OgreProj;
		for(size_t x(0); x < 4; x++)
			for(size_t y(0); y < 4; y++)
				OgreProj[x][y] = proj.M[x][y];

		//Set the matrix
		cams[eyeIndex]->setCustomProjectionMatrix(true, OgreProj);
	}

		// Variables for render loop
	bool render(true);
	/*
	Ogre::LogManager::getSingleton().logMessage("foer");
	OIS::ParamList pl;

	//OIS::InputManager* inputManager;
    size_t windowHnd = 0;
    std::ostringstream windowHndStr;

	//tell OIS about the Ogre window
	try
	{
    window->getCustomAttribute("WINDOW", &windowHnd);
    windowHndStr << windowHnd;
    pl.insert(std::make_pair(std::string("WINDOW"), windowHndStr.str()));
	Ogre::LogManager::getSingleton().logMessage("mellom");


    OIS::InputManager* inputManager = OIS::InputManager::createInputSystem(pl);
	}
	catch (...)
	{
		Ogre::LogManager::getSingleton().logMessage("e.eText");
		//std::cout << e.eText << std::endl;
		//Ogre::LogManager::getSingleton().logMessage(e.eText);
	}
	Ogre::LogManager::getSingleton().logMessage("etter");
	*/

	size_t hWnd = 0;

	window->getCustomAttribute("WINDOW", &hWnd);
	OIS::InputManager* inputManager = OIS::InputManager::createInputSystem(hWnd);

	OIS::Keyboard* mKeyboard = static_cast<OIS::Keyboard*>(inputManager->createInputObject(OIS::OISKeyboard, true));

 
    //Ogre::LogManager::getSingleton().logMessage();


	ovrFrameTiming hmdFrameTiming;
	Ogre::Vector3 cameraPosition;
	Ogre::Quaternion cameraOrientation;
	ovrTrackingState ts;
	OVR::Posef pose;
	OVR::Quatf oculusOrient;
	OVR::Vector3f oculusPos;
	ovrLayerHeader* layers;
	

	// Render loop
	while(render)
	{
		Ogre::WindowEventUtilities::messagePump();
		//advance textureset index
		textureSet->CurrentIndex = (textureSet->CurrentIndex + 1) % textureSet->TextureCount;
		//Ogre::LogManager::getSingleton().logMessage(Ogre::StringConverter::toString(textureSet->CurrentIndex));

		hmdFrameTiming = ovr_GetFrameTiming(hmd, 0);
		ts = ovr_GetTrackingState(hmd, hmdFrameTiming.DisplayMidpointSeconds);
		pose = ts.HeadPose.ThePose;
		ovr_CalcEyePoses(pose, offset, layer.RenderPose);
		oculusOrient = pose.Rotation;
		oculusPos = pose.Translation;
		//Ogre::LogManager::getSingleton().logMessage(Ogre::StringConverter::toString(Ogre::Quaternion(oculusOrient.w, oculusOrient.x, oculusOrient.y, oculusOrient.z)));
		mHeadNode->setOrientation(cameraOrientation * Ogre::Quaternion(oculusOrient.w, oculusOrient.x, oculusOrient.y, oculusOrient.z));
		
		mKeyboard->capture();
		if (mKeyboard->isKeyDown(OIS::KC_W))
		{
			Ogre::LogManager::getSingleton().logMessage("w pressed");
			Ogre::LogManager::getSingleton().logMessage(Ogre::StringConverter::toString(mHeadNode->getOrientation() * Ogre::Vector3(0, 0, -1)));
			mBodyNode->setOrientation(mHeadNode->getOrientation());
			Ogre::LogManager::getSingleton().logMessage(Ogre::StringConverter::toString(mHeadNode->getOrientation() * Ogre::Vector3(0, 0, -1)));
			Ogre::Vector3 direction = mHeadNode->getOrientation() * Ogre::Vector3(0, 0, -1);
			Ogre::LogManager::getSingleton().logMessage(Ogre::StringConverter::toString(mBodyNode->getPosition()));
			mBodyNode->setPosition( mBodyNode->getPosition() + direction );
			Ogre::LogManager::getSingleton().logMessage(Ogre::StringConverter::toString(mBodyNode->getPosition()));
		}
        //float forward = (mKeyboard->isKeyDown( OIS::KC_W ) ? 0 : 1) + (mKeyboard->isKeyDown( OIS::KC_S ) ? 0 : -1);
        //float leftRight = (mKeyboard->isKeyDown( OIS::KC_A ) ? 0 : 1) + (mKeyboard->isKeyDown( OIS::KC_D ) ? 0 : -1);
		//float rotation = (mKeyboard->isKeyDown( OIS::KC_E ) ? 0 : 1) + (mKeyboard->isKeyDown( OIS::KC_Q ) ? 0 : -1);
        //Ogre::Vector3 dirX = mBodyTiltNode->_getDerivedOrientation()*Ogre::Vector3::UNIT_X;
        //Ogre::Vector3 dirZ = mBodyTiltNode->_getDerivedOrientation()*Ogre::Vector3::UNIT_Z;
		

        //mBodyNode->setPosition( mBodyNode->getPosition() + dirZ*forward +dirX*leftRight );		
		//	mBodyNode->yaw(Ogre::Degree(0.8f)*rotation);

		root->_fireFrameRenderingQueued();
		vpts[left]->update();
		vpts[right]->update();

		//Copy the rendered image to the Oculus Swap Texture
		glCopyImageSubData(renderTextureID, GL_TEXTURE_2D, 0, 0, 0, 0, 
		((ovrGLTexture*)(&textureSet->Textures[textureSet->CurrentIndex]))->OGL.TexId, GL_TEXTURE_2D, 0, 0, 0, 0, 
		bufferSize.w,bufferSize.h, 1);
		layers = &layer.Header;
		

		ovr_SubmitFrame(hmd, 0, nullptr, &layers, 1);

		window->update();
		

		if(window->isClosed()) render = false;
	}

	ovr_Destroy(hmd);
	ovr_Shutdown();

	delete root;
	return EXIT_SUCCESS;
}