#include "ActorEntry.h"


concurrency::concurrent_vector<ActorEntry> actorEntries;

std::map<std::pair<UInt32, const char*>, NiPoint3> thingDefaultPosList;
std::map<std::pair<UInt32, const char *>, NiMatrix33> thingDefaultRotList;
