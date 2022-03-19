#pragma once

#include <cstdlib>
#include <cstdio>

#include <typeinfo>

#include <memory>
#include <vector>
#include <chrono>
#include <algorithm>
#include <cassert>
#include <atomic>
#include <string>
#include <sstream>
#include <iterator>
#include <algorithm>
#include <atomic>
#include <functional>

#include <unordered_set>
#include <unordered_map>
#include <vector>
#include "amp.h"
#include <ppl.h>

#include "skse64/GameReferences.h"
#include "skse64/NiNodes.h"
#include "skse64/NiTypes.h"
#include "skse64/NiObjects.h"
#include "skse64/GameForms.h"
#include "skse64/GameRTTI.h"


#include "skse64/NiGeometry.h"
#include "skse64/PluginAPI.h"
#include "skse64/GameStreams.h"
#include "skse64/NiExtraData.h"

#include "Thing.h"

#define NINODE_CHILDREN(ninode) ((NiTArray <NiAVObject *> *) ((char*)(&(ninode->m_children))))

extern std::shared_mutex obj_bind_lock, obj_sync_lock;

class SimObj {
	UInt32 id = 0;
	bool bound = false;
public:
	//The basic unit is parallel processing, but some physics chain nodes need sequential loading
	std::unordered_map<const char *, std::unordered_map<const char*, Thing>> things;
	std::unordered_map<std::string, Collision> actorColliders;
	std::unordered_map<std::string, NiPoint3> NodeCollisionSync;

	Actor* ownerActor;
	float actorDistSqr;

	SimObj(Actor *actor);
	SimObj() {}
	~SimObj();
	bool bind(Actor *actor, bool isMale);
	void update(Actor *actor, bool CollisionsEnabled);
	bool updateConfig(Actor* actor);
	bool isBound() const { return bound; }

	bool GroundCollisionEnabled = false;
};

bool IsActorValid(Actor *actor);


extern const char *leftBreastName;
extern const char *rightBreastName;
extern const char *leftButtName;
extern const char *rightButtName;
extern const char *bellyName;




class AIProcessManager
{
public:
	static AIProcessManager* GetSingleton();

	void StopArtObject(TESObjectREFR* ref, BGSArtObject* art);

	UInt8  unk000;                   // 008
	bool   enableDetection;          // 001 
	bool   unk002;                   // 002 
	UInt8  unk003;                   // 003
	UInt32 unk004;                   // 004
	bool   enableHighProcess;        // 008 
	bool   enableLowProcess;         // 009 
	bool   enableMiddleHighProcess;  // 00A 
	bool   enableMiddleLowProcess;   // 00B 
	bool   enableAISchedules;        // 00C 
	UInt8  unk00D;                   // 00D
	UInt8  unk00E;                   // 00E
	UInt8  unk00F;                   // 00F
	SInt32 numActorsInHighProcess;   // 010
	UInt32 unk014[(0x30 - 0x014) / sizeof(UInt32)];
	tArray<UInt32>  actorsHigh; // 030 
	tArray<UInt32>  actorsLow;  // 048 
	tArray<UInt32>  actorsMiddleLow; // 060
	tArray<UInt32>  actorsMiddleHigh; // 078
	UInt32  unk90[(0xF0 - 0x7C) / sizeof(UInt32)];
	tArray<void*> activeEffectShaders; // 108
									   //mutable BSUniqueLock			 activeEffectShadersLock; // 120
};
static_assert(offsetof(AIProcessManager, numActorsInHighProcess) >= 0x10, "Unk141F831B0::actorsHigh is too early!");
static_assert(offsetof(AIProcessManager, numActorsInHighProcess) <= 0x10, "Unk141F831B0::actorsHigh is too late!");

static_assert(offsetof(AIProcessManager, actorsHigh) >= 0x030, "Unk141F831B0::actorsHigh is too early!");
static_assert(offsetof(AIProcessManager, actorsHigh) <= 0x039, "Unk141F831B0::actorsHigh is too late!");

static_assert(offsetof(AIProcessManager, actorsLow) >= 0x048, "Unk141F831B0::actorsLow is too early!");
static_assert(offsetof(AIProcessManager, actorsLow) <= 0x048, "Unk141F831B0::actorsLow is too late!");

static_assert(offsetof(AIProcessManager, actorsMiddleLow) >= 0x060, "Unk141F831B0::actorsMiddleLow is too early!");
static_assert(offsetof(AIProcessManager, actorsMiddleLow) <= 0x060, "Unk141F831B0::actorsMiddleLow is too late!");

static_assert(offsetof(AIProcessManager, actorsMiddleHigh) >= 0x078, "Unk141F831B0::actorsMiddleHigh is too early!");
static_assert(offsetof(AIProcessManager, actorsMiddleHigh) <= 0x078, "Unk141F831B0::actorsMiddleHigh is too late!");

static_assert(offsetof(AIProcessManager, activeEffectShaders) >= 0x108, "Unk141F831B0::activeEffectShaders is too early!");
static_assert(offsetof(AIProcessManager, activeEffectShaders) <= 0x108, "Unk141F831B0::activeEffectShaders is too late!");



typedef bool(*_IsInCombatNative)(Actor* actor);

bool ActorInCombat(Actor* actor);

