#pragma once
#include <vector>

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
extern std::vector<ActorEntry> actorEntries;

extern std::map<std::pair<UInt32, const char *>, NiPoint3> thingDefaultPosList;
extern std::map<std::pair<UInt32, const char*>, NiMatrix33> thingDefaultRotList;