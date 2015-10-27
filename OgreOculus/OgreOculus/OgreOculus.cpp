#include "OgreOculus.h"

enum eyes{left, right, nbEyes};
int max(int a, int b)
{
	if (a > b) return a; return b;
}

OgreOculus::OgreOculus(void)
	: root(0),
	smgr(0),
	window(0),
	mPlayerNode(0),
	mCamera(0),
	mDirection(Ogre::Vector3::ZERO),
	mMove(1),
	mRotate(0.13),

	hmd(0),

	mInputManager(0),
	mMouse(0),
	mKeyboard(0)//,
	//mCameraMan(0)
{

}

OgreOculus::~OgreOculus(void)
{
	//if (mCameraMan) delete mCameraMan;

	windowClosed(window);
	delete root;
}

int OgreOculus::go(void)
{
	//Create Root object
	//Ogre::Root* root = new Ogre::Root("plugin.cfg", "ogre.cfg");
	root = new Ogre::Root("plugin.cfg", "ogre.cfg");
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
	/*Ogre::RenderWindow* */window = root->createRenderWindow("Ogre + Oculus = <3", hmdDesc.Resolution.w/2, hmdDesc.Resolution.h/2, false);

	//Create scene manager and cameras
	//Ogre::SceneManager* smgr = root->createSceneManager(Ogre::ST_GENERIC);
	smgr = root->createSceneManager(Ogre::ST_GENERIC);
	Ogre::Camera* cams[nbEyes];

	//OCR cameras
	cams[left] = smgr->createCamera("leftcam");
	cams[right] = smgr->createCamera("rightcam");

	//setup body and head cam
	Ogre::SceneNode* mBodyNode = smgr->getRootSceneNode()->createChildSceneNode("BodyNode");
	//Ogre::SceneNode*  mBodyTiltNode = mBodyNode->createChildSceneNode();
	//Ogre::SceneNode*  mHeadNode = mBodyTiltNode->createChildSceneNode("HeadNode");
	Ogre::SceneNode* mHeadNode = mBodyNode->createChildSceneNode("HeadNode");
	mBodyNode->setFixedYawAxis( true );	// don't roll!

	mHeadNode->attachObject(cams[left]);
	mHeadNode->attachObject(cams[right]);

	//set cam position
	//float dist = 0.05;
	Ogre::Real dist = Ogre::Real(0.05);
	cams[right]->setPosition(Ogre::Vector3(dist/2.0f, 0.0f, 0.0f));

	cams[right]->setNearClipDistance(Ogre::Real(0.001));

	cams[left]->setPosition(Ogre::Vector3(-dist/2.0f, 0.0f,0.0f));

	cams[left]->setNearClipDistance(Ogre::Real(0.001));

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

	Ogre::Plane plane(Ogre::Vector3::UNIT_Y, 0);
	Ogre::MeshManager::getSingleton().createPlane(
		"ground",
		Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
		plane, 
		1500, 1500, 20, 20, 
		true, 
		1, 5, 5, 
		Ogre::Vector3::UNIT_Z);
	Ogre::Entity* groundEntity = smgr->createEntity("ground");
	smgr->getRootSceneNode()->createChildSceneNode()->attachObject(groundEntity);
	groundEntity->setMaterialName("Examples/Rockwall");
	/*---------------------------------------------*/
	/* OGRE MODEL CREATION ENDS HERE                */
	/*---------------------------------------------*/

	// Camera part here
	createCamera();
	
	/*
	Ogre::Camera* mCamera = smgr->createCamera("PlayerCam");
	//mCamera->setPosition(Ogre::Vector3(0, 50, 100));
	//mCamera->lookAt(Ogre::Vector3(0, 0, 0));
	mCamera->setNearClipDistance(Ogre::Real(5));

	Ogre::Vector3 initialCameraPosition = Ogre::Vector3(0.0, 50.0, 100.0);
	Ogre::Vector3 initialCameraLookAt = Ogre::Vector3::ZERO;
	Ogre::Real movementSpeed = Ogre::Real(1);
	Ogre::Real mRotate = Ogre::Real(0.13);
	//Ogre::SceneNode* mPlayerNode = smgr->getRootSceneNode()->createChildSceneNode("PlayerNode");
	mPlayerNode = smgr->getRootSceneNode()->createChildSceneNode("PlayerNode");
	mPlayerNode->attachObject(mCamera);
	//mPlayerNode->setPosition(0, 50.0, 100.0); //view is different from window camera
	mPlayerNode->setPosition(initialCameraPosition);
	//mPlayerNode->lookAt(Ogre::Vector3::ZERO, Ogre::SceneNode::TS_WORLD);
	mPlayerNode->lookAt(initialCameraLookAt, Ogre::SceneNode::TS_WORLD);
	//Ogre::Vector3 playerDirection = Ogre::Vector3(0,0,-1);
	Ogre::Vector3 playerDirection = initialCameraLookAt - initialCameraPosition;
	playerDirection.normalise();
	//Ogre::LogManager::getSingleton().logMessage(Ogre::StringConverter::toString(playerDirection));
	*/

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
	/******************************************************/
	/*
	size_t hWnd = 0;

	window->getCustomAttribute("WINDOW", &hWnd);
	OIS::InputManager* mInputManager = OIS::InputManager::createInputSystem(hWnd);
	//InputManager* inputManager = InputManager::initialise(window); //(Ogre::RenderWindow *renderWindow
	//InputManager* mInputMgr = InputManager::getSingletonPtr();
	//mInputMgr->initialise(window);
	//OIS::Keyboard* mKeyboard = static_cast<OIS::Keyboard*>(oisInputManager->createInputObject(OIS::OISKeyboard, true));
	OIS::Keyboard* mKeyboard = static_cast<OIS::Keyboard*>(mInputManager->createInputObject(OIS::OISKeyboard, false));
	OIS::Mouse* mMouse = static_cast<OIS::Mouse*>(mInputManager->createInputObject(OIS::OISMouse, false));

	//set window extent (should change when window resizes)
	const OIS::MouseState &mouseState = mMouse->getMouseState();
	//OIS::MouseState &mouseState = const_cast<OIS::MouseState &>(mMouse->getMouseState());
	//OIS::MouseState &mutableMouseState = const_cast<OIS::MouseState &>(mMouse->getMouseState());
	mouseState.width = window->getWidth(); //??
	mouseState.height = window->getHeight();
	//mouseState.width = Ogre::Root::getSingleton().getAutoCreatedWindow()->getWidth(); //??
	//mouseState.height =  Ogre::Root::getSingleton().getAutoCreatedWindow()->getHeight();
	//mutableMouseState.width = window->getWidth();
	//mutableMouseState.height = window->getHeight();
	//mouseState.X.abs = (int)(window->getWidth()/2.0);
	//mouseState.Y.abs = (int)(window->getHeight()/2.0);
	//mutableMouseState.X.abs = window->getWidth()/2.0;
	//mutableMouseState.Y.abs = window->getHeight()/2.0;
	//mouseState.X.rel = (int)(window->getWidth()/2.0);
	//mouseState.Y.rel = (int)(window->getHeight()/2.0);

    //Ogre::LogManager::getSingleton().logMessage();
	*/
	createFrameListener();
	/******************************************************************/

	ovrFrameTiming hmdFrameTiming;
	Ogre::Vector3 cameraPosition;
	Ogre::Quaternion cameraOrientation;
	ovrTrackingState ts;
	OVR::Posef pose;
	OVR::Quatf oculusOrient;
	OVR::Vector3f oculusPos;
	ovrLayerHeader* layers;

	/* MOUSE MOVEMENT (OLD)
	int mouseXstate = mouseState.X.rel;
	int mouseYstate = mouseState.Y.rel;
	//Ogre::Vector3 internalXAxis = Ogre::Vector3(1,0,0);
	Ogre::Vector3 internalXAxis = Ogre::Quaternion(Ogre::Degree(-90), Ogre::Vector3::UNIT_Y) * playerDirection;
	internalXAxis.y = 0;
	internalXAxis.normalise();
	//int mouseXstate = mMouse->getMouseState().X.rel;
	//int mouseYstate = mMouse->getMouseState().Y.rel;
	*/
	//static Ogre::Real move = 1;
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
		mMouse->capture();
		/*
		Ogre::Vector3 dirVec = Ogre::Vector3::ZERO;
		
		//mMouse->getMouseState();

		if (mKeyboard->isKeyDown(OIS::KC_W) || mKeyboard->isKeyDown(OIS::KC_UP))
		{
			dirVec.z -= move;
			//Ogre::LogManager::getSingleton().logMessage("w pressed");
			//mPlayerNode->translate(playerDirection*movementSpeed);
			//Ogre::LogManager::getSingleton().logMessage(Ogre::StringConverter::toString(mouseState.X.rel));

			Ogre::LogManager::getSingleton().logMessage(Ogre::StringConverter::toString(mMouse->getMouseState().X.rel));
			//Ogre::LogManager::getSingleton().logMessage(Ogre::StringConverter::toString(mouseState.X.abs));
			Ogre::LogManager::getSingleton().logMessage(Ogre::StringConverter::toString(mMouse->getMouseState().X.abs));
			//Ogre::LogManager::getSingleton().logMessage(Ogre::StringConverter::toString(mouseState.X.rel - mouseXstate));
			Ogre::LogManager::getSingleton().logMessage(Ogre::StringConverter::toString(mMouse->getMouseState().X.rel - mouseXstate));

			//Ogre::LogManager::getSingleton().logMessage(Ogre::StringConverter::toString(mHeadNode->getOrientation()));
			//mBodyNode->setOrientation(mHeadNode->getOrientation());
			//Ogre::LogManager::getSingleton().logMessage(Ogre::StringConverter::toString(mHeadNode->getOrientation() * Ogre::Vector3(0, 0, -1)));
			//Ogre::Vector3 direction = mHeadNode->getOrientation() * Ogre::Vector3(0, 0, -1);
			//Ogre::LogManager::getSingleton().logMessage(Ogre::StringConverter::toString(mBodyNode->getPosition()));
			//mBodyNode->setPosition( mBodyNode->getPosition() + direction );
			//Ogre::LogManager::getSingleton().logMessage(Ogre::StringConverter::toString(mBodyNode->getPosition()));
			//Ogre::LogManager::getSingleton().logMessage("error?");
		}
		if (mKeyboard->isKeyDown(OIS::KC_S) || mKeyboard->isKeyDown(OIS::KC_DOWN))
		{
			dirVec.z += move;
			//mPlayerNode->translate(-playerDirection*movementSpeed);
		}
		if (mKeyboard->isKeyDown(OIS::KC_A) || mKeyboard->isKeyDown(OIS::KC_LEFT))
		{
			dirVec.x -= move;

			Ogre::Vector3 temp = Ogre::Quaternion(Ogre::Degree(90), Ogre::Vector3::UNIT_Y) * playerDirection;// v2 = Quaternion(Degree(-90), Vector3::UNIT_Y) * v1
			// set y = 0 and normalize?
			temp.y = 0;
			temp.normalise();
			mPlayerNode->translate(temp*movementSpeed);

		}
		if (mKeyboard->isKeyDown(OIS::KC_D) || mKeyboard->isKeyDown(OIS::KC_RIGHT))
		{
			dirVec.x += move;

			Ogre::Vector3 temp = Ogre::Quaternion(Ogre::Degree(-90), Ogre::Vector3::UNIT_Y) * playerDirection;// v2 = Quaternion(Degree(-90), Vector3::UNIT_Y) * v1
			// set y = 0 and normalize?
			temp.y = 0;
			temp.normalise();
			mPlayerNode->translate(temp*movementSpeed);

		}
		if (mKeyboard->isKeyDown(OIS::KC_ESCAPE))
		{
			render = false;
			//break;
		}
		//mPlayerNode->translate(dirVec, Ogre::Node::TS_LOCAL);
		smgr->getSceneNode("PlayerNode")->translate(dirVec, Ogre::Node::TS_LOCAL);
        //float forward = (mKeyboard->isKeyDown( OIS::KC_W ) ? 0 : 1) + (mKeyboard->isKeyDown( OIS::KC_S ) ? 0 : -1);
        //float leftRight = (mKeyboard->isKeyDown( OIS::KC_A ) ? 0 : 1) + (mKeyboard->isKeyDown( OIS::KC_D ) ? 0 : -1);
		//float rotation = (mKeyboard->isKeyDown( OIS::KC_E ) ? 0 : 1) + (mKeyboard->isKeyDown( OIS::KC_Q ) ? 0 : -1);
        //Ogre::Vector3 dirX = mBodyTiltNode->_getDerivedOrientation()*Ogre::Vector3::UNIT_X;
        //Ogre::Vector3 dirZ = mBodyTiltNode->_getDerivedOrientation()*Ogre::Vector3::UNIT_Z;
		*/

        //mBodyNode->setPosition( mBodyNode->getPosition() + dirZ*forward +dirX*leftRight );
		//	mBodyNode->yaw(Ogre::Degree(0.8f)*rotation);

		//Mouse movement
		//yaw
		/*
		int mouseXChange = mouseState.X.rel - mouseXstate;
		//int mouseXChange = mMouse->getMouseState().X.rel - mouseXstate;
		mPlayerNode->yaw(Ogre::Degree(-mRotate * mouseXChange), Ogre::Node::TS_WORLD);
		mouseXstate = mouseState.X.rel;
		//mouseXstate = mMouse->getMouseState().X.rel;
		playerDirection = Ogre::Quaternion(Ogre::Degree(-mRotate * mouseXChange), Ogre::Vector3::UNIT_Y) * playerDirection;
		internalXAxis = Ogre::Quaternion(Ogre::Degree(-mRotate * mouseXChange), Ogre::Vector3::UNIT_Y) * internalXAxis;
		// modify currentDirectionc vector !!

		//pitch
		int mouseYChange = mouseState.Y.rel - mouseYstate;
		//int mouseYChange = mMouse->getMouseState().Y.rel - mouseYstate;
		mPlayerNode->pitch(Ogre::Degree(-mRotate * mouseYChange), Ogre::Node::TS_WORLD);
		mouseYstate = mouseState.Y.rel;
		//mouseYstate = mMouse->getMouseState().Y.rel;
		playerDirection = Ogre::Quaternion(Ogre::Degree(-mRotate * mouseXChange), internalXAxis) * playerDirection;
		// modify currentDirectionc vector !!

		//mouseState.X.rel = (int)(window->getWidth()/2.0);
		//mouseState.Y.rel = (int)(window->getHeight()/2.0);
		*/
		mPlayerNode->translate(mDirection, Ogre::Node::TS_LOCAL);

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

void OgreOculus::createCamera(void)
{
	mCamera = smgr->createCamera("PlayerCam");
	//mCamera->setPosition(Ogre::Vector3(0, 50, 100));
	//mCamera->lookAt(Ogre::Vector3(0, 0, 0));
	mCamera->setNearClipDistance(5);

	mPlayerNode = smgr->getRootSceneNode()->createChildSceneNode("PlayerNode", Ogre::Vector3(0,50,100));
	mPlayerNode->attachObject(mCamera);
	mPlayerNode->lookAt(Ogre::Vector3(0, 0, 0), Ogre::SceneNode::TS_WORLD);

	//mCameraMan = new OgreBites::SdkCameraMan(mCamera);
}

void OgreOculus::createFrameListener(void)
{
	Ogre::LogManager::getSingletonPtr()->logMessage("*** Initializing OIS ***");

	size_t hWnd = 0;
	window->getCustomAttribute("WINDOW", &hWnd);
	mInputManager = OIS::InputManager::createInputSystem(hWnd);

	mKeyboard = static_cast<OIS::Keyboard*>(mInputManager->createInputObject(OIS::OISKeyboard, true));
	mMouse = static_cast<OIS::Mouse*>(mInputManager->createInputObject(OIS::OISMouse, true));

	mMouse->setEventCallback(this);
	mKeyboard->setEventCallback(this);

	//
	//root->addFrameListener(this);
}

bool OgreOculus::keyPressed(const OIS::KeyEvent &ke)
{
	switch (ke.key)
	{
	case OIS::KC_W:
	case OIS::KC_UP:
		mDirection.z = -mMove;
		break;

	case OIS::KC_S:
	case OIS::KC_DOWN:
		mDirection.z = mMove;
		break;

	case OIS::KC_A:
	case OIS::KC_LEFT:
		mDirection.x = -mMove;
		break;

	case OIS::KC_D:
	case OIS::KC_RIGHT:
		mDirection.x = mMove;
		break;
	}
	/*
	if (ke.key == OIS::KC_W || ke.key == OIS::KC_UP)
	{
		Ogre::LogManager::getSingletonPtr()->logMessage("w pressed");
		//mPlayerNode->translate(Ogre::Vector3(0,0,-1));
	}
	*/
	return true;
}

bool OgreOculus::keyReleased(const OIS::KeyEvent &ke)
{
	switch (ke.key)
	{
	case OIS::KC_W:
	case OIS::KC_UP:
		mDirection.z = 0;
		break;

	case OIS::KC_S:
	case OIS::KC_DOWN:
		mDirection.z = 0;
		break;

	case OIS::KC_A:
	case OIS::KC_LEFT:
		mDirection.x = 0;
		break;

	case OIS::KC_D:
	case OIS::KC_RIGHT:
		mDirection.x = 0;
		break;
	}
	return true;
}

bool OgreOculus::mouseMoved(const OIS::MouseEvent &me)
{
	mPlayerNode->yaw(Ogre::Degree(-mRotate * me.state.X.rel), Ogre::Node::TS_WORLD);
	mPlayerNode->pitch(Ogre::Degree(-mRotate * me.state.Y.rel), Ogre::Node::TS_LOCAL);
	return true;
}

bool OgreOculus::mousePressed(const OIS::MouseEvent &me, OIS::MouseButtonID id)
{
	return true;
}

bool OgreOculus::mouseReleased(const OIS::MouseEvent &me, OIS::MouseButtonID id)
{
	return true;
}

void OgreOculus::windowClosed(Ogre::RenderWindow* rw)
{
	if (rw == window)
	{
		if (mInputManager)
		{
			mInputManager->destroyInputObject(mMouse);
			mInputManager->destroyInputObject(mKeyboard);

			OIS::InputManager::destroyInputSystem(mInputManager);
			mInputManager = 0;
		}
	}
}

mainFunc()
{
	OgreOculus oo;

	try {
		oo.go();
	} catch(Ogre::Exception& e) {
		std::cerr << "An exception has occurred: " <<
                e.getFullDescription().c_str() << std::endl;
	}

	return 0;

}
