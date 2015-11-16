#include <Ogre.h>

void createOgreModel(Ogre::SceneManager* smgr){
	smgr->setAmbientLight(Ogre::ColourValue(0.5, 0.5, 0.5));
	Ogre::Entity* ogreEntity = smgr->createEntity("tudorhouse.mesh");
	Ogre::SceneNode* ogreNode = smgr->getRootSceneNode()->createChildSceneNode();
	ogreNode->attachObject(ogreEntity);
	ogreNode->setPosition(Ogre::Vector3(-300, 550, 200));

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

	//Add some fish
	for(int i = 0; i < 0; i++){
		Ogre::Entity* ogreEntity2 = smgr->createEntity("fish.mesh");
		Ogre::SceneNode* ogreNode2 = smgr->getRootSceneNode()->createChildSceneNode();
		ogreNode2->setPosition(Ogre::Vector3(0, 50, i*10));
		ogreNode2->attachObject(ogreEntity2);

		Ogre::AnimationState* mAnimationState = ogreEntity2->getAnimationState("swim");
		mAnimationState->setLoop(true);
		mAnimationState->setEnabled(true);
		Ogre::Degree angle = Ogre::Degree(180); 
		ogreNode2->roll(angle);
		ogreNode2->pitch(angle);
	}

}