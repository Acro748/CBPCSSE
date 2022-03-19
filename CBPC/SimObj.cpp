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

//## obj_bind_lock
// Although I didn't look closely
// it's used unordered_map [] so that's not thread safety
// and I felt like needed to lock a lot of things on the parts where the collider attach part
// but it work once per actor, so I just locked entirety
// If necessary, we can make more optimizations later

//## obj_sync_lock
// unordered_map [] and new write value is not thread safety, so locked it

std::shared_mutex obj_bind_lock, obj_sync_lock;

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
	std::lock_guard<std::shared_mutex> obj_bind_guard(obj_bind_lock);

	auto loadedState = actor->loadedState;
	if (loadedState && loadedState->node) 
	{
		bound = true;

		things.clear();
		actorColliders.clear();
		const std::string actorRace = actor->race->fullName.GetName();

		for (int i = 0; i<affectedBones.size(); i++)
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
				if (!CheckActorForConditions(actor, nodeConditionsMap[affectedBones.at(i).at(j)]))
				{
					continue;
				}
				BSFixedString cs = ReturnUsableString(affectedBones.at(i).at(j));
				auto bone = loadedState->node->GetObjectByName(&cs.data);
				if (bone)
				{
					thingmsg.emplace(cs.data, Thing(actor, bone, cs));
					if (isfirstbone)
					{
						firstcs = cs;
						isfirstbone = false;
					}
				}
			}
			if (!isfirstbone)
				things.emplace(firstcs.data, thingmsg);
		}
		updateConfig(actor);
			
		GroundCollisionEnabled = CreateActorColliders(actor, actorColliders);

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
	concurrency::parallel_for_each(things.begin(), things.end(), [&](auto& t)
	{
		bool isStopPhysics = false;
		if (ActorNodeStoppedPhysicsMap.find(GetActorNodeString(actor, t.first)) != ActorNodeStoppedPhysicsMap.end())
			isStopPhysics = ActorNodeStoppedPhysicsMap[GetActorNodeString(actor, t.first)];

		if (!isStopPhysics)
		{
			for (auto& tt : t.second) //The basic unit is parallel processing, but some physics chain nodes need sequential loading
			{
				tt.second.ActorCollisionsEnabled = CollisionsEnabled;
				tt.second.GroundCollisionEnabled = GroundCollisionEnabled;
				if (strcmp(t.first, pelvis) == 0)
				{
					tt.second.updatePelvis(actor);
				}
				else
				{
					tt.second.update(actor);
					if (tt.second.VirtualCollisionEnabled)
					{
						obj_sync_lock.lock();
						NodeCollisionSync[tt.first] = tt.second.collisionSync;
						obj_sync_lock.unlock();
					}
				}
			}
		}
	});
	//logger.error("end SimObj update\n");
}

bool SimObj::updateConfig(Actor* actor) {
	for (auto &t : things) {
		//LOG("t.first:[%s]", t.first);

		for (auto& tt : t.second) {
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
		}
	}
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