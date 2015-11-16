#include "OgreOculus.h"

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
	initialOculusOrientation(Ogre::Quaternion(1,0,0,0)),
	headPositionTrackingSensitivity(300),
	hmd(0),
	mInputManager(0),
	mMouse(0),
	mKeyboard(0)
{

}

OgreOculus::~OgreOculus(void)
{
	windowClosed(window);
	delete root;
}

int OgreOculus::go(void)
{
	// Create Root object
	root = new Ogre::Root("plugin.cfg", "ogre.cfg");

	// OpenGL
    root->loadPlugin("RenderSystem_GL_d");
    root->setRenderSystem(root->getRenderSystemByName("OpenGL Rendering Subsystem"));

	// Initialize Root
	root->initialise(false);

	// Initialize Oculus
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

	// Create a window
	window = root->createRenderWindow("Ogre + Oculus = <3", hmdDesc.Resolution.w/2, hmdDesc.Resolution.h/2, false);

	// Create scene manager and cameras
	smgr = root->createSceneManager(Ogre::ST_GENERIC);

	// Load Ogre resource paths from config file
    Ogre::ConfigFile cf;
    cf.load("resources_d.cfg");

    // Go through all sections & settings in the file and add resources
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

	// Create the model itself via OgreModel.cpp
	createOgreModel(smgr);

	// Create camera
	createCamera();

	// Set viewport and background color
	Ogre::Viewport* vp = window->addViewport(mCamera);
	vp->setBackgroundColour(Ogre::ColourValue(0.3, 0.4, 0.7)); // Yellow

	// Set aspect ratio
	mCamera->setAspectRatio(
    Ogre::Real(vp->getActualWidth()) /
    Ogre::Real(vp->getActualHeight()));

	// Initialize glew
	if(glewInit() != GLEW_OK)
		exit(-3);

	// Get texture sizes
	ovrSizei texSizeL, texSizeR;
	texSizeL = ovr_GetFovTextureSize(hmd, ovrEye_Left, hmdDesc.DefaultEyeFov[left], 1);
	texSizeR = ovr_GetFovTextureSize(hmd, ovrEye_Right, hmdDesc.DefaultEyeFov[right], 1);

	// Calculate render buffer size
	ovrSizei bufferSize;
	bufferSize.w = texSizeL.w + texSizeR.w;
	bufferSize.h = max(texSizeL.h, texSizeR.h);

	// Create render texture set
	ovrSwapTextureSet* textureSet;
	if(ovr_CreateSwapTextureSetGL(hmd, GL_RGB, bufferSize.w, bufferSize.h, &textureSet) != ovrSuccess)
		exit(-4);

	// Create Ogre render texture
	Ogre::GLTextureManager* textureManager = static_cast<Ogre::GLTextureManager*>(Ogre::GLTextureManager::getSingletonPtr());
	Ogre::TexturePtr rtt_texture(textureManager->createManual("RttTex", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
		Ogre::TEX_TYPE_2D, bufferSize.w, bufferSize.h, 0, Ogre::PF_R8G8B8, Ogre::TU_RENDERTARGET));
	Ogre::RenderTexture* rttEyes = rtt_texture->getBuffer(0, 0)->getRenderTarget();
	Ogre::GLTexture* gltex = static_cast<Ogre::GLTexture*>(Ogre::GLTextureManager::getSingleton().getByName("RttTex").getPointer());
	GLuint renderTextureID = gltex->getGLID();

	// Put camera viewport on the ogre render texture
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

	// Get GLIDs
	GLuint ogreMirrorTextureID = static_cast<Ogre::GLTexture*>(Ogre::GLTextureManager::getSingleton().getByName("MirrorTex").getPointer())->getGLID();
	GLuint oculusMirrorTextureID = ((ovrGLTexture*)mirrorTexture)->OGL.TexId;

	// Create EyeRenderDesc
	ovrEyeRenderDesc EyeRenderDesc[nbEyes];
	EyeRenderDesc[left] = ovr_GetRenderDesc(hmd, ovrEye_Left, hmdDesc.DefaultEyeFov[left]);
	EyeRenderDesc[right] = ovr_GetRenderDesc(hmd, ovrEye_Right, hmdDesc.DefaultEyeFov[right]);

	// Get offsets
	ovrVector3f offset[nbEyes];
	offset[left]=EyeRenderDesc[left].HmdToEyeViewOffset;
	offset[right]=EyeRenderDesc[right].HmdToEyeViewOffset;

	// Compositor layer
	ovrLayerEyeFov layer;
	layer.Header.Type = ovrLayerType_EyeFov;
	layer.Header.Flags = 0;
	layer.ColorTexture[left] = textureSet;
	layer.ColorTexture[right] = textureSet;
	layer.Fov[left] = EyeRenderDesc[left].Fov;
	layer.Fov[right] = EyeRenderDesc[right].Fov;
	layer.Viewport[left] = OVR::Recti(0, 0, bufferSize.w/2, bufferSize.h);
	layer.Viewport[right] = OVR::Recti(bufferSize.w/2, 0, bufferSize.w/2, bufferSize.h);

	// Get projection matrices
	for(size_t eyeIndex(0); eyeIndex < ovrEye_Count; eyeIndex++)
	{
		// Get the projection matrix
		OVR::Matrix4f proj = ovrMatrix4f_Projection(EyeRenderDesc[eyeIndex].Fov,
			static_cast<float>(0.01f),
			4000,
			true);

		// Convert it to Ogre matrix
		Ogre::Matrix4 OgreProj;
		for(size_t x(0); x < 4; x++)
			for(size_t y(0); y < 4; y++)
				OgreProj[x][y] = proj.M[x][y];

		// Set the matrix
		cams[eyeIndex]->setCustomProjectionMatrix(true, OgreProj);
	}

	// Variables for render loop
	bool render(true);
	ovrFrameTiming hmdFrameTiming;
	ovrTrackingState ts;
	OVR::Posef pose;
	ovrLayerHeader* layers;

	// Create event listener for handling user input
	createEventListener();

	//Run physics loop in a new thread
	std::map<Ogre::Entity*, Ogre::Vector3> positionRequests;
	std::map<Ogre::Entity*, std::string> animationRequests;
	std::map<Ogre::Entity*, std::vector<int>> rotationRequests;
	std::map<std::string, std::string> message;
	std::thread physicsThread(physicsLoop, smgr, &message, &positionRequests, &animationRequests, &rotationRequests);

	// Render loop
	while(render)
	{
		// Suspend physics loop and perform requested movement/rotations/animations
		if(positionRequests.size() > 0 || animationRequests.size() > 0 || rotationRequests.size() > 0){
			message.insert(std::pair<std::string, std::string>("", ""));
		
			for(auto const &request : positionRequests) {
				Ogre::Vector3 pos = request.second;
				Ogre::SceneNode* sceneNode = request.first->getParentSceneNode();
				sceneNode->setPosition(pos);
			}

			for(auto const &request : animationRequests) {
				request.first->getAnimationState(request.second)->addTime(0.1);
			}

			for(auto const &request : rotationRequests) {
				Ogre::SceneNode* sceneNode = request.first->getParentSceneNode();
				sceneNode->roll(Ogre::Degree(request.second[0]));
				sceneNode->pitch(Ogre::Degree(request.second[1]));
				sceneNode->yaw(Ogre::Degree(request.second[2]));
			}

			positionRequests.clear();
			animationRequests.clear();
			rotationRequests.clear();

			// Resume physics loop
			message.clear();
		}

		// Update Ogre window
		Ogre::WindowEventUtilities::messagePump();

		// Advance textureset index
		textureSet->CurrentIndex = (textureSet->CurrentIndex + 1) % textureSet->TextureCount;
		
		// Capture user input
		mKeyboard->capture();
		mMouse->capture();

		// Movement calculations
		mPlayerNode->translate(mDirection, Ogre::Node::TS_LOCAL);
		hmdFrameTiming = ovr_GetFrameTiming(hmd, 0);
		ts = ovr_GetTrackingState(hmd, hmdFrameTiming.DisplayMidpointSeconds);
		pose = ts.HeadPose.ThePose;
		ovr_CalcEyePoses(pose, offset, layer.RenderPose);
		oculusOrient = pose.Rotation;
		oculusPos = pose.Translation;
		mHeadNode->setOrientation(Ogre::Quaternion(oculusOrient.w, oculusOrient.x, oculusOrient.y, oculusOrient.z) * initialOculusOrientation.Inverse());
		
		// Apply head tracking
		mHeadNode->setPosition(headPositionTrackingSensitivity * Ogre::Vector3(oculusPos.x, oculusPos.y,oculusPos.z));
		
		// Update Ogre viewports
		root->_fireFrameRenderingQueued();
		vpts[left]->update();
		vpts[right]->update();

		// Copy the rendered image to the Oculus Swap Texture
		glCopyImageSubData(renderTextureID, GL_TEXTURE_2D, 0, 0, 0, 0,
		((ovrGLTexture*)(&textureSet->Textures[textureSet->CurrentIndex]))->OGL.TexId, GL_TEXTURE_2D, 0, 0, 0, 0,
		bufferSize.w,bufferSize.h, 1);
		layers = &layer.Header;

		// Submit new frame to the Oculus and update window
		ovr_SubmitFrame(hmd, 0, nullptr, &layers, 1);
		window->update();

		// Exit loop when window is closed
		if(window->isClosed()) render = false;
	}

	// Shud down Oculus
	ovr_Destroy(hmd);
	ovr_Shutdown();

	// Delete Ogre root and return
	delete root;
	return EXIT_SUCCESS;
}

// Function for creating Ogre cameras
void OgreOculus::createCamera(void)
{
	// Create camera and head node
	mCamera = smgr->createCamera("PlayerCam");
	mCamera->setNearClipDistance(5);

	mPlayerNode = smgr->getRootSceneNode()->createChildSceneNode("PlayerNode", Ogre::Vector3(0,50,100));
	mHeadNode = mPlayerNode->createChildSceneNode("HeadNode");

	mHeadNode->attachObject(mCamera);
	mPlayerNode->lookAt(Ogre::Vector3(0, 50, 0), Ogre::SceneNode::TS_WORLD);
	mPlayerNode->setFixedYawAxis(true);

	// OCR cameras
	cams[left] = smgr->createCamera("leftcam");
	cams[right] = smgr->createCamera("rightcam");

	// Set cam position
	Ogre::Real dist = Ogre::Real(0.05);
	cams[right]->setNearClipDistance(Ogre::Real(0.001));
	cams[left]->setNearClipDistance(Ogre::Real(0.001));
	Ogre::SceneNode* mRightEyeNode = mHeadNode->createChildSceneNode("RightEyeNode", Ogre::Vector3(dist/2.0f, 0.0f, 0.0f));
	Ogre::SceneNode* mLeftEyeNode = mHeadNode->createChildSceneNode("LeftEyeNode", Ogre::Vector3(-dist/2.0f, 0.0f, 0.0f));
	mRightEyeNode->attachObject(cams[right]);
	mLeftEyeNode->attachObject(cams[left]);
}

// Event listener for user input
void OgreOculus::createEventListener(void)
{
	Ogre::LogManager::getSingletonPtr()->logMessage("*** Initializing OIS ***");

	size_t hWnd = 0;
	window->getCustomAttribute("WINDOW", &hWnd);
	mInputManager = OIS::InputManager::createInputSystem(hWnd);

	mKeyboard = static_cast<OIS::Keyboard*>(mInputManager->createInputObject(OIS::OISKeyboard, true));
	mMouse = static_cast<OIS::Mouse*>(mInputManager->createInputObject(OIS::OISMouse, true));

	mMouse->setEventCallback(this);
	mKeyboard->setEventCallback(this);
}

bool OgreOculus::keyPressed(const OIS::KeyEvent &ke)
{
	switch (ke.key)
	{
	case OIS::KC_ESCAPE:
		 std::exit(0);
		break;
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

	case OIS::KC_L:
		Ogre::LogManager::getSingleton().logMessage(Ogre::StringConverter::toString(Ogre::Vector3(oculusPos.x, oculusPos.y, oculusPos.z)));
		break;

	case OIS::KC_R:
		initialOculusOrientation = Ogre::Quaternion(oculusOrient.w, oculusOrient.x, oculusOrient.y, oculusOrient.z);
		mPlayerNode->setPosition(Ogre::Vector3(0,50,100));
		mPlayerNode->lookAt(Ogre::Vector3(0, 50, 0), Ogre::SceneNode::TS_WORLD);
		break;

	case OIS::KC_LSHIFT:
		mMove = mMove*2;
		mDirection = 2*mDirection;
		break;
	}
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

	case OIS::KC_LSHIFT:
		mMove = mMove/2.0;
		mDirection = mDirection /2.0;
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
