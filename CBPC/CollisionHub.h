#pragma once

#include "Collision.h"
#include "ActorEntry.h"



#ifdef RUNTIME_VR_VERSION_1_4_15
void CreatePlayerColliders(concurrency::concurrent_unordered_map<std::string, Collision> &actorCollidersList);
void UpdatePlayerColliders(concurrency::concurrent_unordered_map<std::string, Collision> &actorCollidersList);
#endif

extern long callCount;

bool CreateActorColliders(Actor * actor, concurrency::concurrent_unordered_map<std::string, Collision> &actorCollidersList);

void UpdateColliderPositions(concurrency::concurrent_unordered_map<std::string, Collision> &colliderList, concurrency::concurrent_unordered_map<std::string, NiPoint3> NodeCollisionSyncList);


struct Partition
{
	concurrency::concurrent_vector<Collision> partitionCollisions;
};

typedef concurrency::concurrent_unordered_map<int, Partition> PartitionMap;

extern PartitionMap partitions;



int GetHashIdFromPos(NiPoint3 pos);

std::vector<int> GetHashIdsFromPos(NiPoint3 pos, float radius);

bool CheckPelvisArmor(Actor* actor);

