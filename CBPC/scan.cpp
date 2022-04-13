#pragma once

#include "SimObj.h"


#pragma warning(disable : 4996)


extern SKSETaskInterface *g_task;

std::unordered_map<UInt32, SimObj> actors;

std::vector<UInt32> notExteriorWorlds = { 0x69857, 0x1EE62, 0x20DCB, 0x1FAE2, 0x34240, 0x50015, 0x2C965, 0x29AB7, 0x4F838, 0x3A9D6, 0x243DE, 0xC97EB, 0xC350D, 0x1CDD3, 0x1CDD9, 0x21EDB, 0x1E49D, 0x2B101, 0x2A9D8, 0x20BFE };


//void UpdateWorldDataToChild(NiAVObject)
void dumpTransform(NiTransform t) {
	Console_Print("%8.2f %8.2f %8.2f", t.rot.data[0][0], t.rot.data[0][1], t.rot.data[0][2]);
	Console_Print("%8.2f %8.2f %8.2f", t.rot.data[1][0], t.rot.data[1][1], t.rot.data[1][2]);
	Console_Print("%8.2f %8.2f %8.2f", t.rot.data[2][0], t.rot.data[2][1], t.rot.data[2][2]);

	Console_Print("%8.2f %8.2f %8.2f", t.pos.x, t.pos.y, t.pos.z);
	Console_Print("%8.2f", t.scale);
}


bool visitObjects(NiAVObject  *parent, std::function<bool(NiAVObject*, int)> functor, int depth = 0) {
	if (!parent) return false;
	NiNode * node = parent->GetAsNiNode();
	if (node) {
		if (functor(parent, depth))
			return true;

		for (UInt32 i = 0; i < node->m_children.m_emptyRunStart; i++) {
			NiAVObject * object = node->m_children.m_data[i];
			if (object) {
				if (visitObjects(object, functor, depth + 1))
					return true;
			}
		}
	}
	else if (functor(parent, depth))
		return true;

	return false;
}

std::string spaces(int n) {
	auto s = std::string(n, ' ');
	return s;
}

bool printStuff(NiAVObject *avObj, int depth) {
	std::string sss = spaces(depth);
	const char *ss = sss.c_str();
	LOG_INFO("%savObj Name = %s, RTTI = %s\n", ss, avObj->m_name, avObj->GetRTTI()->name);

	NiNode *node = avObj->GetAsNiNode();
	if (node) {
		LOG_INFO("%snode %s, RTTI %s\n", ss, node->m_name, node->GetRTTI()->name);
	}
	return false;
}


void dumpVec(NiPoint3 p) {
	LOG_INFO("%8.2f %8.2f %8.2f\n", p.x, p.y, p.z);
}




template<class T>
inline void safe_delete(T*& in) {
	if (in) {
		delete in;
		in = NULL;
	}
}

#ifdef RUNTIME_VR_VERSION_1_4_15


#elif RUNTIME_VERSION_1_5_97 || RUNTIME_VERSION_1_5_80 || RUNTIME_VERSION_1_5_73

#elif RUNTIME_VERSION_1_5_62 || RUNTIME_VERSION_1_5_53 || RUNTIME_VERSION_1_5_50

void TESObjectREFR::IncRef()
{
	handleRefObject.IncRef();
}

void TESObjectREFR::DecRef()
{
	handleRefObject.DecRef();
}

typedef bool(*_LookupREFRByHandle2)(UInt32 & refHandle, NiPointer<TESObjectREFR> & refrOut);
extern RelocAddr<_LookupREFRByHandle2> LookupREFRByHandle2;

RelocAddr<_LookupREFRByHandle2> LookupREFRByHandle2(0x00132A90);

#elif RUNTIME_VERSION_1_5_39 || RUNTIME_VERSION_1_5_23

void TESObjectREFR::IncRef()
{
	handleRefObject.IncRef();
}

void TESObjectREFR::DecRef()
{
	handleRefObject.DecRef();
}

typedef bool(*_LookupREFRByHandle2)(UInt32 & refHandle, NiPointer<TESObjectREFR> & refrOut);
extern RelocAddr<_LookupREFRByHandle2> LookupREFRByHandle2;

RelocAddr<_LookupREFRByHandle2> LookupREFRByHandle2(0x00132A20);

#endif


AIProcessManager* AIProcessManager::GetSingleton()
{
#ifdef RUNTIME_VR_VERSION_1_4_15

	static RelocPtr<AIProcessManager*> singleton(0x01F831B0); //For VR 1.4.15

#elif RUNTIME_VERSION_1_6_353

	static RelocPtr<AIProcessManager*> singleton(0x01F5A450); //For SSE 1.6.353 and up

#elif RUNTIME_VERSION_1_6_342

	static RelocPtr<AIProcessManager*> singleton(0x01F5A450); //For SSE 1.6.342 and up

#elif RUNTIME_VERSION_1_6_323

	static RelocPtr<AIProcessManager*> singleton(0x01F592D0); //For SSE 1.6.323 and up

#elif RUNTIME_VERSION_1_6_318

	static RelocPtr<AIProcessManager*> singleton(0x01F592D0); //For SSE 1.6.318 and up

#elif RUNTIME_VERSION_1_5_97 || RUNTIME_VERSION_1_5_80 || RUNTIME_VERSION_1_5_73

	static RelocPtr<AIProcessManager*> singleton(0x01EBEAD0); //For SSE 1.5.73 and up

#elif RUNTIME_VERSION_1_5_62 || RUNTIME_VERSION_1_5_53 || RUNTIME_VERSION_1_5_50 || RUNTIME_VERSION_1_5_39

	static RelocPtr<AIProcessManager*> singleton(0x01EE5AD0); //For SSE 1.5.39 - 1.5.62

#elif RUNTIME_VERSION_1_5_23

	static RelocPtr<AIProcessManager*> singleton(0x01EE4A50); //For SSE 1.5.23

#endif

	return *singleton;
}

bool compareActorEntries(const ActorEntry& entry1, const ActorEntry& entry2)
{
	return entry1.actorDistSqr < entry2.actorDistSqr;
}

bool ActorIsInAngle(Actor* actor, float originalHeading, NiPoint3 cameraPosition)
{
	if (actor == (*g_thePlayer))
		return true;

	if (actorAngle >= 360)
		return true;

	if (actorAngle <= 0 && actor != (*g_thePlayer))
	{
		return false;
	}

	NiPoint3 position = actor->loadedState->node->m_worldTransform.pos;

	float heading = 0;
	float attitude = 0;
	GetAttitudeAndHeadingFromTwoPoints(cameraPosition, NiPoint3(position.x, position.y, cameraPosition.z), attitude, heading);
	heading = heading * 57.295776f;

	return AngleDifference(originalHeading, heading) <= (actorAngle * 0.5f);
}

std::atomic<TESObjectCELL *> curCell = nullptr;
std::atomic<bool> curCellSpaceExterior = false;
std::atomic<UInt32> curCellWorldspaceFormId = 0;

int frameCount = 0;

bool inCreatureForm = false;

float lastPlayerWeight = 50.0f;

int skipFramesScanCount = 0;

LARGE_INTEGER startingTime, endingTime, elapsedMicroseconds;
LARGE_INTEGER frequency;


bool debugtimelog = false;
bool firsttimeloginit = true;
LARGE_INTEGER totaltime;
int debugtimelog_framecount = 1;
int totalcallcount = 0;

///Average Update Time per 1000 frames
///
///#UseParallelProcessingOLD = 3
///Average Update Time = 702435 ns
///Average Update Time = 626482 ns
///Average Update Time = 626149 ns
///Average Update Time = 682146 ns
///Average Update Time = 674943 ns
///Average Update Time = 738473 ns
///Average Update Time = 715636 ns
///Average Update Time = 773313 ns
///
///#UseParallelProcessingOLD = 0
///Average Update Time = 1247070 ns
///Average Update Time = 1088073 ns
///Average Update Time = 1171247 ns
///Average Update Time = 1254823 ns
///Average Update Time = 1611882 ns
///Average Update Time = 1671157 ns
///Average Update Time = 1661606 ns
///Average Update Time = 1504056 ns

void updateActors() 
{	
	if (debugtimelog || logging)
	{
		if (firsttimeloginit)
		{
			firsttimeloginit = false;
			totaltime.QuadPart = 0;

		}

		QueryPerformanceFrequency(&frequency);
		QueryPerformanceCounter(&startingTime);
	}
	// We scan the cell and build the list every time - only look up things by ID once
	// we retain all state by actor ID, in a map - it's cleared on cell change

	actorEntries.clear();

	auto mm = MenuManager::GetSingleton();

	if ((mm->IsGamePaused()) && !raceSexMenuOpen.load())
		return;

	if (tuningModeCollision != 0 || consoleCollisionReload.load())
	{
		if (consoleCollisionReload.load())
		{
			consoleCollisionReload.store(false);

			loadMasterConfig();
			loadCollisionConfig();
			loadExtraCollisionConfig();

#ifdef RUNTIME_VR_VERSION_1_4_15
			LoadWeaponCollisionConfig();
#endif
			actors.clear();
		}
		else
		{
			frameCount++;
			if (frameCount % (120 * tuningModeCollision) == 0)
			{
				loadMasterConfig();
				loadCollisionConfig();
				loadExtraCollisionConfig();

#ifdef RUNTIME_VR_VERSION_1_4_15
				LoadWeaponCollisionConfig();
#endif
				actors.clear();
			}
			if (frameCount >= 1000000)
				frameCount = 0;
		}
	}


	//logger.error("scan Cell\n");
	if (!(*g_thePlayer) || !(*g_thePlayer)->loadedState)
		return;

	AIProcessManager* processMan = AIProcessManager::GetSingleton();
	TESObjectCELL* cell = (*g_thePlayer)->parentCell;
	if (!cell)
		return;

	bool cellChanged = false;

	if (cell != curCell.load())
	{		
		cellChanged = (cell->worldSpace != nullptr && cell->worldSpace->unk158 == nullptr && std::find(notExteriorWorlds.begin(), notExteriorWorlds.end(), cell->worldSpace->formID) == notExteriorWorlds.end()) != (curCellSpaceExterior.load() && std::find(notExteriorWorlds.begin(), notExteriorWorlds.end(), curCellWorldspaceFormId.load()) == notExteriorWorlds.end());
		curCell.store(cell);
		curCellSpaceExterior.store((cell->worldSpace != nullptr && cell->worldSpace->unk158 == nullptr));
		curCellWorldspaceFormId.store(cell->worldSpace != nullptr ? cell->worldSpace->formID : 0);
		//LOG("Cell changed. %s",(cell->worldSpace != nullptr && cell->worldSpace->unk158 == nullptr) ? "Exterior" : "Interior");
	}

	callCount = 0;

	//TESNPC * actorNPC;
	Actor* actor;
		
	NiPointer<TESObjectREFR> ToRefr = NULL;

	bool creatureFormChange = false;
	bool playerWeightChange = false;
	if (skipFramesScanCount > 0) //We don't wanna do this every frame;
	{
		skipFramesScanCount--;		
	}
	else
	{
		skipFramesScanCount = 180;

		if ((*g_thePlayer) && ((Actor*)(*g_thePlayer))->race != nullptr)
		{
			auto actorRef = DYNAMIC_CAST(((Actor*)(*g_thePlayer)), Actor, TESObjectREFR);
			if (actorRef)
			{
				float playerWeight = CALL_MEMBER_FN(actorRef, GetWeight)();

				if (fabsf(playerWeight - lastPlayerWeight) > 0.001f)
				{
					playerWeightChange = true;
				}
				lastPlayerWeight = playerWeight;
			}

			if (((Actor*)(*g_thePlayer))->race->formID == 0xCDD84 || ((Actor*)(*g_thePlayer))->race->formID == VampireLordBeastRaceFormId) //In beast form
			{
				if (inCreatureForm == false)
				{
					creatureFormChange = true;
					inCreatureForm = true;
				}
			}
			else
			{
				if (inCreatureForm == true)
				{
					creatureFormChange = true;
					inCreatureForm = false;
				}
			}
		}
	}

	if (cellChanged || raceSexMenuClosed.load() || creatureFormChange || playerWeightChange || MainMenuOpen.load())
	{
		raceSexMenuClosed.store(false);
		MainMenuOpen.store(false);
		actors.clear();
	}
	else
	{
		if (processMan)
		{
			float originalHeading = 0;
			NiPoint3 cameraPosition;
			NiPoint3 cameraHeadingPosition;
#ifndef RUNTIME_VR_VERSION_1_4_15
			if (useCamera)
			{
				PlayerCamera* camera = PlayerCamera::GetSingleton();

				if (camera != nullptr && camera->cameraNode != nullptr)
				{
					cameraPosition = camera->cameraNode->m_worldTransform.pos + (camera->cameraNode->m_worldTransform.rot * NiPoint3(0, -200.0f, 0));
					cameraHeadingPosition = cameraPosition + (camera->cameraNode->m_worldTransform.rot * NiPoint3(0, 500.0f, 0));
				}
				else
				{
					cameraPosition = (*g_thePlayer)->loadedState->node->m_worldTransform.pos + ((*g_thePlayer)->loadedState->node->m_worldTransform.rot * NiPoint3(0, -200.0f, 0));
					cameraHeadingPosition = cameraPosition + ((*g_thePlayer)->loadedState->node->m_worldTransform.rot * NiPoint3(0, 500.0f, 0));
				}
			}
			else
			{
#endif
				cameraPosition = (*g_thePlayer)->loadedState->node->m_worldTransform.pos + ((*g_thePlayer)->loadedState->node->m_worldTransform.rot * NiPoint3(0, -200.0f, 0));
				cameraHeadingPosition = cameraPosition + ((*g_thePlayer)->loadedState->node->m_worldTransform.rot * NiPoint3(0, 500.0f, 0));
#ifndef RUNTIME_VR_VERSION_1_4_15
			}
#endif 
			float heading = 0;
			float attitude = 0;
			GetAttitudeAndHeadingFromTwoPoints(cameraPosition, cameraHeadingPosition, attitude, heading);
			originalHeading = heading * 57.295776f;

			NiPoint3 relativeActorPos;

			for (UInt32 i = 0; i < processMan->actorsHigh.count+1; i++)
			{
				if (i < processMan->actorsHigh.count)
				{
#ifdef RUNTIME_VR_VERSION_1_4_15
					LookupREFRByHandle(processMan->actorsHigh[i], ToRefr);
#elif RUNTIME_VERSION_1_6_323 ||RUNTIME_VERSION_1_6_318 || RUNTIME_VERSION_1_5_97 || RUNTIME_VERSION_1_5_80 || RUNTIME_VERSION_1_5_73
					LookupREFRByHandle(processMan->actorsHigh[i], ToRefr);
#else
					LookupREFRByHandle2(processMan->actorsHigh[i], ToRefr);
#endif
				}
				if (ToRefr != nullptr || i == processMan->actorsHigh.count)
				{
					if (i == processMan->actorsHigh.count)
					{
						actor = (*g_thePlayer);
					}
					else
					{
						actor = DYNAMIC_CAST(ToRefr, TESObjectREFR, Actor);
					}

					if (actor && actor->loadedState && actor->loadedState->node)
					{
						if (actor->race && actor != (*g_thePlayer))
						{
							std::string actorRace = actor->race->fullName.GetName();

							/*if (!(actor->race->data.raceFlags & TESRace::kRace_AllowPCDialogue) && !(actor->race->data.raceFlags & TESRace::kRace_Playable))
							{
								if (extraRacesList.empty())
								{
									continue;
								}
								else if (std::find(extraRacesList.begin(), extraRacesList.end(), actorRace) == extraRacesList.end())
								{
									continue;
								}
							}
							else */if (actor->race->data.raceFlags & TESRace::kRace_Child)
								continue;

							LOG("actorRace: %s", actorRace.c_str());
						}

						if (actor->loadedState->node)
						{
							relativeActorPos = actor->loadedState->node->m_worldTransform.pos - (*g_thePlayer)->loadedState->node->m_worldTransform.pos;

							float actorDistSqr = magnitudePwr2(relativeActorPos);

							if ((actor->flags1 & Actor::kFlags_IsPlayerTeammate) != Actor::kFlags_IsPlayerTeammate)
							{
								if (actorDistSqr > actorBounceDistance)
									continue;

								if (!ActorIsInAngle(actor, originalHeading, cameraPosition))
								{
									continue;
								}
							}

							if (actors.find(actor->formID) == actors.end()) //If actor isn't in the actors list
							{
								//logger.info("Tracking Actor with form ID %08x in cell %ld\n", actor->formID, actor->parentCell);
								if (IsActorValid(actor))
								{
									auto obj = SimObj(actor);
									obj.actorDistSqr = actorDistSqr;
									actors.emplace(actor->formID, obj);
									actorEntries.emplace_back(ActorEntry{ actor->formID, actor, IsActorMale(actor) ? 1 : 0, actorDistSqr, actorDistSqr <= actorDistance });
								}
							}
							else if (IsActorValid(actor))
							{
								actors[actor->formID].actorDistSqr = actorDistSqr;
								actorEntries.emplace_back(ActorEntry{ actor->formID, actor, IsActorMale(actor) ? 1 : 0, actorDistSqr, actorDistSqr <= actorDistance });
							}
						}
					}
				}
			}
		}
	}

	if (ActorInCombat(*g_thePlayer))
	{
		if (actorEntries.size() > inCombatActorCount)
		{
			std::sort(actorEntries.begin(), actorEntries.end(), compareActorEntries);

			actorEntries.resize(inCombatActorCount);
		}
	}
	else
	{
		if (actorEntries.size() > outOfCombatActorCount)
		{
			std::sort(actorEntries.begin(), actorEntries.end(), compareActorEntries);

			actorEntries.resize(outOfCombatActorCount);
		}
	}

	#ifdef RUNTIME_VR_VERSION_1_4_15
	auto playerIt = actors.find(0x14);
	if (playerIt != actors.end())
	{
		UpdatePlayerColliders(playerIt->second.actorColliders);
	}
	#endif

	LOG("ActorCount: %d", actorEntries.size());
				
	partitions.clear();

	LOG("Starting collider hashing");
	std::vector<int> ids;

	std::vector<int> hashIdList;

	NiPoint3 playerPos = (*g_thePlayer)->loadedState->node->m_worldTransform.pos;
	int colliderSphereCount = 0;
	int colliderCapsuleCount = 0;

	for (int u = 0; u < actorEntries.size(); u++)
	{
		if (actorEntries[u].collisionsEnabled == false)
			continue;

		auto objIt = actors.find(actorEntries[u].id);
		if (objIt != actors.end())
		{				
			UpdateColliderPositions(objIt->second.actorColliders, objIt->second.NodeCollisionSync);

			for (auto &collider : objIt->second.actorColliders)
			{
				ids.clear();
				for (int j = 0; j < collider.second.collisionSpheres.size(); j++)
				{
					hashIdList = GetHashIdsFromPos(collider.second.collisionSpheres[j].worldPos - playerPos, collider.second.collisionSpheres[j].radius100);

					for (int m = 0; m < hashIdList.size(); m++)
					{
						if (std::find(ids.begin(), ids.end(), hashIdList[m]) == ids.end())
						{
							//LOG_INFO("ids.emplace_back(%d)", hashIdList[m]);
							ids.emplace_back(hashIdList[m]);
							partitions[hashIdList[m]].partitionCollisions.emplace_back(collider.second);
						}
					}
					colliderSphereCount++;
				}
				for (int j = 0; j < collider.second.collisionCapsules.size(); j++)
				{
					hashIdList = GetHashIdsFromPos((collider.second.collisionCapsules[j].End1_worldPos + collider.second.collisionCapsules[j].End2_worldPos) * 0.5f - playerPos
						, (collider.second.collisionCapsules[j].End1_radius100 + collider.second.collisionCapsules[j].End2_radius100) * 0.5f);
					for (int m = 0; m < hashIdList.size(); m++)
					{
						if (std::find(ids.begin(), ids.end(), hashIdList[m]) == ids.end())
						{
							//LOG_INFO("ids.emplace_back(%d)", hashIdList[m]);
							ids.emplace_back(hashIdList[m]);
							partitions[hashIdList[m]].partitionCollisions.emplace_back(collider.second);
						}
					}
					colliderCapsuleCount++;
				}
				#ifdef RUNTIME_VR_VERSION_1_4_15
				for (int j = 0; j < collider.second.collisionTriangles.size(); j++)
				{
					for (int k = 0; k < 101; k = k + 20)
					{
						NiPoint3 pos = GetPointFromPercentage(collider.second.collisionTriangles[j].a, collider.second.collisionTriangles[j].b, k);

						int id = GetHashIdFromPos(pos - playerPos);
						if (id != -1)
						{
							if (std::find(ids.begin(), ids.end(), id) == ids.end())
							{
								ids.emplace_back(id);
								partitions[id].partitionCollisions.emplace_back(collider.second);
							}
						}
					}
					for (int k = 0; k < 101; k = k + 20)
					{
						NiPoint3 pos = GetPointFromPercentage(collider.second.collisionTriangles[j].a, collider.second.collisionTriangles[j].c, k);

						int id = GetHashIdFromPos(pos - playerPos);
						if (id != -1)
						{
							if (std::find(ids.begin(), ids.end(), id) == ids.end())
							{
								ids.emplace_back(id);
								partitions[id].partitionCollisions.emplace_back(collider.second);
							}
						}
					}
					for (int k = 0; k < 101; k = k + 20)
					{
						NiPoint3 pos = GetPointFromPercentage(collider.second.collisionTriangles[j].b, collider.second.collisionTriangles[j].c, k);

						int id = GetHashIdFromPos(pos - playerPos);
						if (id != -1)
						{
							if (std::find(ids.begin(), ids.end(), id) == ids.end())
							{
								ids.emplace_back(id);
								partitions[id].partitionCollisions.emplace_back(collider.second);
							}
						}
					}
				}
				#endif
			}
		}
	}
	LOG("Collider sphere count = %d", colliderSphereCount);
	LOG("Collider capsule count = %d", colliderCapsuleCount);

	//Print partitions
	/*LOG("============================");
	LOG("PARTITIONS size=%d", partitions.size());
	int p = 0;
	for (auto kv : partitions)
	{
		if (kv.second.partitionCollisions.size() > 0)
		{
			LOG("Partition %d - HashId: %ld -----------------------", p, kv.first);
			for (int c = 0; c < kv.second.partitionCollisions.size(); c++)
			{
				std::string actorName = "";
				std::string actorSex = "";
				if (kv.second.partitionCollisions.at(c).colliderActor != nullptr)
				{
					TESObjectREFR* actorRef = DYNAMIC_CAST(kv.second.partitionCollisions.at(c).colliderActor, Actor, TESObjectREFR);
					if (actorRef != nullptr)
					{
						actorName = CALL_MEMBER_FN(actorRef, GetReferenceName)();
						actorSex = IsActorMale(kv.second.partitionCollisions.at(c).colliderActor) ? "Male" : "Female";
					}
				}

				LOG("%s - %s - Node:%s - Spheres:%d", actorName.c_str(), actorSex.c_str(), kv.second.partitionCollisions.at(c).colliderNodeName.c_str(), kv.second.partitionCollisions.at(c).collisionSpheres.size());
			}
		}
		p++;
	}
	LOG("============================");*/


	//static bool done = false;
	//if (!done && player->loadedState->node) {
	//	visitObjects(player->loadedState->node, printStuff);
	//	BSFixedString cs("UUNP");
	//	auto bodyAV = player->loadedState->node->GetObjectByName(&cs.data);
	//	BSTriShape *body = bodyAV->GetAsBSTriShape();
	//	logger.info("GetAsBSTriShape returned  %lld\n", body);
	//	auto geometryData = body->geometryData;
	//	//logger.info("Num verts = %d\n", geometryData->m_usVertices);


	//	done = true;
	//}

	static int count = 0;
	if ((configReloadCount && count++ > configReloadCount) || consoleConfigReload.load())
	{
		consoleConfigReload.store(false);
		count = 0;
		loadConfig();
		for each (auto & a in actorEntries)
		{
			auto objIt = actors.find(a.id);
			if (objIt == actors.end())
			{
				//logger.error("missing Sim Object\n");
			}
			else
			{
				if (a.actor != nullptr && a.actor->loadedState != nullptr)
				{
					SimObj& obj = objIt->second;
					obj.updateConfig(a.actor);
				}
			}
		}

	}
	//logger.error("Updating %d entites\n", actorEntries.size());

	concurrency::parallel_for_each(actorEntries.begin(), actorEntries.end(), [&](const auto& a)
	{
		auto objIt = actors.find(a.id);
		if (objIt == actors.end())
		{
			//logger.error("missing Sim Object\n");
		}
		else
		{
			if (a.actor != nullptr && a.actor->loadedState != nullptr)
			{
				SimObj& obj = objIt->second;
				if (obj.isBound())
				{
					obj.update(a.actor, a.collisionsEnabled);
				}
				else
				{
					obj.bind(a.actor, a.sex == 1);
				}
			}
		}
	});

	//LOG("Collider Check Call Count: %d", callCount);

	if (debugtimelog || logging)
	{
		QueryPerformanceCounter(&endingTime);
		elapsedMicroseconds.QuadPart = endingTime.QuadPart - startingTime.QuadPart;
		elapsedMicroseconds.QuadPart *= 1000000000LL;
		elapsedMicroseconds.QuadPart /= frequency.QuadPart;
		//long long avg = elapsedMicroseconds.QuadPart / callCount;
		totaltime.QuadPart += elapsedMicroseconds.QuadPart;
		totalcallcount += callCount;
		//LOG_ERR("Collider Check Call Count: %d - Update Time = %lld ns", callCount, elapsedMicroseconds.QuadPart);
		if (debugtimelog_framecount % 1000 == 0)
		{
			LOG_ERR("Collider Check Call Count: %.2f - Average Update Time in 1000 frame = %lld ns\n", (float)totalcallcount / (float)debugtimelog_framecount, totaltime.QuadPart / debugtimelog_framecount);
			totaltime.QuadPart = 0;
			debugtimelog_framecount = 0;
			totalcallcount = 0;
		}
		debugtimelog_framecount++;
	}
	//logger.info("Update Time = %lld ns\n", elapsedMicroseconds.QuadPart);

	return;

}

EventDispatcher<TESEquipEvent>* g_TESEquipEventDispatcher;
TESEquipEventHandler g_TESEquipEventHandler;

/*
UInt32 kSlotMask30 = 0x00000001;
UInt32 kSlotMask31 = 0x00000002;
*/
UInt32 kSlotMask32 = 0x00000004;
/*
UInt32 kSlotMask33 = 0x00000008;
UInt32 kSlotMask34 = 0x00000010;
UInt32 kSlotMask35 = 0x00000020;
UInt32 kSlotMask36 = 0x00000040;
UInt32 kSlotMask37 = 0x00000080;
UInt32 kSlotMask38 = 0x00000100;
UInt32 kSlotMask39 = 0x00000200;
UInt32 kSlotMask40 = 0x00000400;
UInt32 kSlotMask41 = 0x00000800;
UInt32 kSlotMask42 = 0x00001000;
UInt32 kSlotMask43 = 0x00002000;
UInt32 kSlotMask44 = 0x00004000;
UInt32 kSlotMask45 = 0x00008000;
UInt32 kSlotMask46 = 0x00010000;
UInt32 kSlotMask47 = 0x00020000;
UInt32 kSlotMask48 = 0x00040000;
UInt32 kSlotMask49 = 0x00080000;
UInt32 kSlotMask50 = 0x00100000;
UInt32 kSlotMask51 = 0x00200000;
UInt32 kSlotMask52 = 0x00400000;
UInt32 kSlotMask53 = 0x00800000;
UInt32 kSlotMask54 = 0x01000000;
UInt32 kSlotMask55 = 0x02000000;
UInt32 kSlotMask56 = 0x04000000;
UInt32 kSlotMask57 = 0x08000000;
UInt32 kSlotMask58 = 0x10000000;
UInt32 kSlotMask59 = 0x20000000;
UInt32 kSlotMask60 = 0x40000000;
UInt32 kSlotMask61 = 0x80000000;
*/

EventResult TESEquipEventHandler::ReceiveEvent(TESEquipEvent* evn, EventDispatcher<TESEquipEvent>* dispatcher)
{
	if (!evn)
		return EventResult::kEvent_Continue;
		
	if (!(*g_thePlayer) || !(*g_thePlayer)->loadedState)
		return EventResult::kEvent_Continue;

	if (evn->actor == nullptr)
		return EventResult::kEvent_Continue;
		
	if (!(evn->baseObject > 0))
		return EventResult::kEvent_Continue;
				
	TESForm* form = LookupFormByID(evn->baseObject);
	if (form == nullptr)
		return EventResult::kEvent_Continue;

	TESObjectARMO* armor = DYNAMIC_CAST(form, TESForm, TESObjectARMO);		
	if (armor == nullptr)
		return EventResult::kEvent_Continue;
						
	if (isWantSlot(armor, kSlotMask32)) // body
	{
		Actor* actor = DYNAMIC_CAST(evn->actor, TESObjectREFR, Actor);

		if (actor == nullptr && actor->loadedState == nullptr)
			return EventResult::kEvent_Continue;

		auto objIt = actors.find(actor->formID);

		if (objIt == actors.end())
			return EventResult::kEvent_Continue;

		SimObj& obj = objIt->second;

		if (!obj.isBound())
			return EventResult::kEvent_Continue;

		for (auto& t : obj.things)
		{
			for (auto& tt : t.second)
			{
				if (tt.second.IsBreastBone)
				{
					tt.second.skipArmorCheck = 0;
				}
			}
		}
	}		

	return EventResult::kEvent_Continue;
}

/*
class ScanDelegate : public TaskDelegate {
public:
virtual void Run() {
updateActors();
}
virtual void Dispose() {
delete this;
}

};


void scaleTest() {
g_task->AddTask(new ScanDelegate());
return;
}
*/