#pragma once
#include <vector>
#include <concurrent_unordered_map.h>
#include <concurrent_vector.h>

#include "skse64/GameReferences.h"

//Actor Entries for access:
struct ActorEntry
{
	UInt32 id;
	Actor *actor;
	int sex; //1 for male, 0 for female
	float actorDistSqr;
	bool collisionsEnabled = true;
};
extern concurrency::concurrent_vector<ActorEntry> actorEntries;

extern std::map<std::pair<UInt32, const char *>, NiPoint3> thingDefaultPosList;
extern std::map<std::pair<UInt32, const char*>, NiMatrix33> thingDefaultRotList;