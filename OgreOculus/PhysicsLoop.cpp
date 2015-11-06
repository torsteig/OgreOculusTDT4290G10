#include <windows.h>
#include <Ogre.h>

void physicsLoop(Ogre::SceneManager* smgr){
	
	int dir = 1;
	int timeStep = 50;
	while(true){
		Sleep(timeStep);
		Ogre::SceneManager::MovableObjectIterator iterator = smgr->getMovableObjectIterator("Entity");
		while(iterator.hasMoreElements()){
			Ogre::Entity* entity = static_cast<Ogre::Entity*>(iterator.getNext());

			// <>< <>< Make the cute fishy swim <>< <><
			//This only moves fish
			if(entity->hasAnimationState("swim")){
				
				Ogre::SceneNode* sceneNode = entity->getParentSceneNode();

				// Update position
				Ogre::Vector3 pos = sceneNode->getPosition();
				if(pos.x > 120){
					dir = -1;
					Ogre::Degree angle = Ogre::Degree(180); 
					sceneNode->yaw(angle);
				}
				if(pos.x < -120){
					dir = 1;
					Ogre::Degree angle = Ogre::Degree(-180); 
					sceneNode->yaw(angle);
				}

				pos += Ogre::Vector3(dir, 0, 0);
				sceneNode->setPosition(pos);

				entity->getAnimationState("swim")->addTime(0.1);
			}

		}

	}

}