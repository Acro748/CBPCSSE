#pragma once

#include "Collision.h"
#include "ActorEntry.h"



#ifdef RUNTIME_VR_VERSION_1_4_15
void CreatePlayerColliders(std::unordered_map<std::string, Collision> &actorCollidersList);
void UpdatePlayerColliders(std::unordered_map<std::string, Collision> &actorCollidersList);
#endif

extern long callCount;

bool CreateActorColliders(Actor * actor, std::unordered_map<std::string, Collision> &actorCollidersList);

void UpdateColliderPositions(std::unordered_map<std::string, Collision> &colliderList, std::unordered_map<std::string, NiPoint3> NodeCollisionSyncList);


struct Partition
{
	std::vector<Collision> partitionCollisions;
};

typedef std::unordered_map<int, Partition> PartitionMap;

extern PartitionMap partitions;



int GetHashIdFromPos(NiPoint3 pos);

std::vector<int> GetHashIdsFromPos(NiPoint3 pos, float radius);

bool CheckPelvisArmor(Actor* actor);

