#include "SimObj.h"



// Note we don't ref count the nodes becasue it's ignored when the Actor is deleted, and calling Release after that can corrupt memory
const char *leftBreastName = "NPC L Breast";
const char *rightBreastName = "NPC R Breast";
const char *LBreast01Name = "L Breast01";
const char* LBreast02Name = "L Breast02";
const char* LBreast03Name = "L Breast03";
const char* RBreast01Name = "R Breast01";
const char* RBreast02Name = "R Breast02";
const char* RBreast03Name = "R Breast03";
const char *leftButtName = "NPC L Butt";
const char *rightButtName = "NPC R Butt";
const char *bellyName = "HDT Belly";
const char *bellyName2 = "NPC Belly";
const char *leftPussy = "NPC L Pussy02";
const char *rightPussy = "NPC R Pussy02";
const char *pelvis = "NPC Pelvis [Pelv]";

//const char *scrotumName = "NPC GenitalsScrotum [GenScrot]";
//const char *leftScrotumName = "NPC L GenitalsScrotum [LGenScrot]";
//const char *rightScrotumName = "NPC R GenitalsScrotum [RGenScrot]";

std::vector<std::string> femaleSpecificBones = { leftBreastName, rightBreastName, LBreast01Name, LBreast02Name, LBreast03Name, RBreast01Name, RBreast02Name, RBreast03Name, leftButtName, rightButtName, bellyName, bellyName2, pelvis};

//std::unordered_map<const char *, std::string> configMap = {
//	{ leftBreastName, "Breast" },{ rightBreastName, "Breast" },
//	{ leftButtName, "Butt" },{ rightButtName, "Butt" },
//	{ bellyName, "Belly" },{ scrotumName, "Scrotum" } };


SimObj::SimObj(Actor *actor)
	: things(){
	id = actor->formID;
	ownerActor = actor;
}

SimObj::~SimObj() {
}


bool SimObj::bind(Actor *actor, bool isMale)
{
	//logger.error("bind\n");

	auto loadedState = actor->loadedState;
	if (loadedState && loadedState->node) 
	{
		bound = true;

		things.clear();
		actorColliders.clear();
		const std::string actorRace = actor->race->fullName.GetName();

		std::shared_mutex obj_read_lock;

		concurrency::parallel_for(size_t(0), affectedBones.size(), [&](size_t i)
		{
			BSFixedString firstcs;
			bool isfirstbone = true;
			std::unordered_map<const char*, Thing> thingmsg;
			for (int j = 0; j < affectedBones.at(i).size(); j++)
			{
				if (isMale && malePhysics == 0 /*&& !(malePhysicsOnlyForExtraRaces != 0 && std::find(extraRacesList.begin(), extraRacesList.end(), actorRace) != extraRacesList.end())*/)
				{
					if (std::find(femaleSpecificBones.begin(), femaleSpecificBones.end(), affectedBones.at(i).at(j)) != femaleSpecificBones.end() || ContainsNoCase(affectedBones.at(i).at(j), "breast") || ContainsNoCase(affectedBones.at(i).at(j), "thigh") || ContainsNoCase(affectedBones.at(i).at(j), "calf"))
					{
						continue;
					}
				}
				if ((nodeConditionsMap.find(affectedBones.at(i).at(j)) != nodeConditionsMap.end()) && !CheckActorForConditions(actor, nodeConditionsMap[affectedBones.at(i).at(j)]))
				{
					continue;
				}
				BSFixedString cs = ReturnUsableString(affectedBones.at(i).at(j));
				obj_read_lock.lock();
				auto bone = loadedState->node->GetObjectByName(&cs.data);
				obj_read_lock.unlock();
				if (bone)
				{
					thingmsg.insert(std::make_pair(cs.data, Thing(actor, bone, cs)));
					if (isfirstbone)
					{
						firstcs = cs;
						isfirstbone = false;
					}
				}
			}
			if (!isfirstbone)
				things.insert(std::make_pair(firstcs.data, thingmsg));
		});
		GroundCollisionEnabled = CreateActorColliders(actor, actorColliders);

		updateConfig(actor);

		#ifdef RUNTIME_VR_VERSION_1_4_15
		if (actor->formID == 0x14)
		{
			CreatePlayerColliders(actorColliders);
		}
		#endif

		return true;
	}
	return false;
}

bool IsActorValid(Actor *actor) {
	if (actor->flags & TESForm::kFlagIsDeleted)
		return false;
	if (actor && actor->loadedState && actor->loadedState->node)
		return true;
	return false;
}


void SimObj::update(Actor *actor, bool CollisionsEnabled) {
	if (!bound)
		return;
	//logger.error("update\n");

	if (!(*g_thePlayer) || !(*g_thePlayer)->loadedState || !(*g_thePlayer)->loadedState->node)
	{
		return;
	}

	float groundPos = -10000.0f;
	float gravityRatio = 1.0f;
	if (actor->loadedState && actor->loadedState->node)
	{
		if (GroundCollisionEnabled)
		{
			NiAVObject* groundobj = actor->loadedState->node->GetObjectByName(&GroundReferenceBone.data);
			if (groundobj)
			{
				groundPos = groundobj->m_worldTransform.pos.z; //Get ground by NPC Root [Root] node

				NiAVObject* highheelobj = actor->loadedState->node->GetObjectByName(&HighheelReferenceBone.data);
				if (highheelobj)
				{
					groundPos -= highheelobj->m_localTransform.pos.z; //Get highheel offset by NPC node
				}
			}
		}
	}
	else
		return;

	//## thing_Refresh_node_lock
	// editing the node update time seems to affect the entire node tree even if without editing entire node tree

	//## thing_ReadNode_lock
	// It seems that a read error occurs when the GetObjectByName() function is called simultaneously

	std::shared_mutex thing_ReadNode_lock, thing_Refresh_node_lock;

	concurrency::parallel_for_each(things.begin(), things.end(), [&](auto& t)
	{
		for (auto& tt : t.second) //The basic unit is parallel processing, but some physics chain nodes need sequential loading
		{
			bool isStopPhysics = false;
			if (ActorNodeStoppedPhysicsMap.find(GetActorNodeString(actor, tt.first)) != ActorNodeStoppedPhysicsMap.end())
				isStopPhysics = ActorNodeStoppedPhysicsMap[GetActorNodeString(actor, tt.first)];

			if (!isStopPhysics)
			{
				tt.second.ActorCollisionsEnabled = CollisionsEnabled;
				if (strcmp(tt.first, pelvis) == 0)
				{
					tt.second.updatePelvis(actor, thing_ReadNode_lock, thing_Refresh_node_lock);
				}
				else
				{
					tt.second.groundPos = groundPos;
					tt.second.update(actor, thing_ReadNode_lock, thing_Refresh_node_lock);
					if (tt.second.VirtualCollisionEnabled)
					{
						NodeCollisionSync[tt.first] = tt.second.collisionSync;
					}
				}
			}
		}
	});
	//logger.error("end SimObj update\n");
}

bool SimObj::updateConfig(Actor* actor) {

	float actorWeight = 50;
	try
	{
		if (actor != nullptr)
		{
			auto actorRef = DYNAMIC_CAST(actor, Actor, TESObjectREFR);

			if (actorRef != nullptr)
			{
				actorWeight = CALL_MEMBER_FN(actorRef, GetWeight)();
			}
		}
	}
	catch (...)
	{

	}

	concurrency::parallel_for_each(things.begin(), things.end(), [&](auto& t) {
		//LOG("t.first:[%s]", t.first);

		for (auto& tt : t.second) {
			tt.second.actorWeight = actorWeight;

			std::string& section = configMap[tt.first];
			//LOG("config section:[%s]", section.c_str());

			SpecificNPCBounceConfig snbc;
			//If the config of that part is not set and just set to default, run to no condition
			if (GetSpecificNPCBounceConfigForActor(actor, snbc) && IsConfigActuallyAllocated(snbc, section))
			{
				tt.second.updateConfig(actor, snbc.config[section], snbc.config0weight[section]);
			}
			else
			{
				tt.second.updateConfig(actor, config[section], config0weight[section]);
			}

			tt.second.GroundCollisionEnabled = GroundCollisionEnabled;
		}
	});
	return true;
}

bool ActorInCombat(Actor* actor)
{
#ifdef RUNTIME_VR_VERSION_1_4_15
	UInt64* vtbl = *((UInt64**)actor);
	return ((_IsInCombatNative)(vtbl[0xE5]))(actor);
#else
	return actor->IsInCombat();
#endif
}