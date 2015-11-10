#include <windows.h>
#include <Ogre.h>

std::map<Ogre::Entity*, Ogre::Vector3>* positionRequests(NULL);
std::map<Ogre::Entity*, std::string>* animationRequests(NULL);
std::map<Ogre::Entity*, std::vector<int>>* rotationRequests(NULL);
std::map<std::string, std::string>* message(NULL);

void loop(Ogre::SceneManager* smgr);
void setPositionX(Ogre::Entity* entity, int x);
void setPositionY(Ogre::Entity* entity, int y);
void setPositionZ(Ogre::Entity* entity, int z);
void animate(Ogre::Entity* entity, std::string animation);
void setRoll(Ogre::Entity* entity, int roll);
void setPitch(Ogre::Entity* entity, int pitch);
void setYaw(Ogre::Entity* entity, int yaw);

// Manage physics from this loop
// Please don't alter the Ogre scene manager directly. Instead use the designated methods (setPositionX, setPositionY, setPositionZ, setRoll, setPitch, setYaw, animate)
void loop(Ogre::SceneManager* smgr){
	int dir = 1;
	int timeStep = 10;
	while(true){
		if((*::message).size() == 1){
			continue;
		}
		Sleep(timeStep);
		Ogre::SceneManager::MovableObjectIterator iterator = smgr->getMovableObjectIterator("Entity");
		while(iterator.hasMoreElements()){
			if((*::message).size() == 1){
				continue;
			}
			Ogre::Entity* entity = static_cast<Ogre::Entity*>(iterator.getNext());

			// <>< <>< Make the cute fishy swim <>< <><
			//This only moves fish
			if(entity->hasAnimationState("swim")){
			
				Ogre::SceneNode* sceneNode = entity->getParentSceneNode();

				// Update position
				Ogre::Vector3 pos = sceneNode->getPosition();
				if(pos.x > 120){
					dir = -1;
					setYaw(entity, 180);
				}
				if(pos.x < -120){
					dir = 1;
					setYaw(entity, 180);
				}
				setPositionX(entity, sceneNode->getPosition().x + dir);
				animate(entity, "swim");
			}

		}
		//ExitThread(0);
	}
}

void physicsLoop(Ogre::SceneManager* smgr, std::map<std::string, std::string>* message, std::map<Ogre::Entity*, Ogre::Vector3>* positionRequests, std::map<Ogre::Entity*, std::string>* animationRequests, std::map<Ogre::Entity*, std::vector<int>>* rotationRequests){
	::positionRequests = positionRequests;
	::animationRequests = animationRequests;
	::rotationRequests = rotationRequests;
	::message = message;
	loop(smgr);
}

void setPositionX(Ogre::Entity* entity, int x){
	Ogre::Vector3 pos = Ogre::Vector3(x, entity->getParentSceneNode()->getPosition().y, entity->getParentSceneNode()->getPosition().z);
	(*::positionRequests).insert(std::pair<Ogre::Entity*, Ogre::Vector3>(entity, pos));
}

void setPositionY(Ogre::Entity* entity, int y){
	Ogre::Vector3 pos = Ogre::Vector3(entity->getParentSceneNode()->getPosition().x, y, entity->getParentSceneNode()->getPosition().z);
	(*::positionRequests).insert(std::pair<Ogre::Entity*, Ogre::Vector3>(entity, pos));
}


void setPositionZ(Ogre::Entity* entity, int z){
	Ogre::Vector3 pos = Ogre::Vector3(entity->getParentSceneNode()->getPosition().x, entity->getParentSceneNode()->getPosition().y, z);
	(*::positionRequests).insert(std::pair<Ogre::Entity*, Ogre::Vector3>(entity, pos));
}


void animate(Ogre::Entity* entity, std::string animation){
	(*::animationRequests).insert(std::pair<Ogre::Entity*, std::string>(entity, animation));
}

void setRoll(Ogre::Entity* entity, int roll){ 	
	std::vector<int> rollPitchYaw(3);
	rollPitchYaw[0] = roll;
	rollPitchYaw[1] = 0;
	rollPitchYaw[2] = 0;
	(*::rotationRequests).insert(std::pair<Ogre::Entity*, std::vector<int>>(entity, rollPitchYaw));
}

void setPitch(Ogre::Entity* entity, int pitch){ 	
	std::vector<int> rollPitchYaw(3);
	rollPitchYaw[0] = 0;
	rollPitchYaw[1] = pitch;
	rollPitchYaw[2] = 0;
	(*::rotationRequests).insert(std::pair<Ogre::Entity*, std::vector<int>>(entity, rollPitchYaw));
}

void setYaw(Ogre::Entity* entity, int yaw){ 	
	std::vector<int> rollPitchYaw(3);
	rollPitchYaw[0] = 0;
	rollPitchYaw[1] = 0;
	rollPitchYaw[2] = yaw;
	(*::rotationRequests).insert(std::pair<Ogre::Entity*, std::vector<int>>(entity, rollPitchYaw));
}