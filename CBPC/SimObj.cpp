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

		for (int i = 0; i<affectedBones.size(); i++)
		{
			if (isMale && malePhysics == 0 /*&& !(malePhysicsOnlyForExtraRaces != 0 && std::find(extraRacesList.begin(), extraRacesList.end(), actorRace) != extraRacesList.end())*/)
			{
				if (std::find(femaleSpecificBones.begin(), femaleSpecificBones.end(), affectedBones.at(i)) != femaleSpecificBones.end() || ContainsNoCase(affectedBones.at(i), "breast") || ContainsNoCase(affectedBones.at(i), "thigh") || ContainsNoCase(affectedBones.at(i), "calf"))
				{
					continue;
				}
			}
			if (!CheckActorForConditions(actor, nodeConditionsMap[affectedBones.at(i)]))
			{
				continue;
			}
			BSFixedString cs = ReturnUsableString(affectedBones.at(i));
			auto bone = loadedState->node->GetObjectByName(&cs.data);
			if (bone) 
			{
				things.emplace(cs.data, Thing(actor, bone, cs));
			}
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
	if (useParallelProcessing == 1 && !raceSexMenuOpen.load())
	{
		concurrency::parallel_for_each(things.begin(), things.end(), [&](auto& t)
		{
			if (ActorNodeStoppedPhysicsMap[GetActorNodeString(actor, t.first)] == false)
			{
				t.second.ActorCollisionsEnabled = CollisionsEnabled;
				t.second.GroundCollisionEnabled = GroundCollisionEnabled;
				if (strcmp(t.first, pelvis) == 0)
				{
					t.second.updatePelvis(actor);
				}
				else
				{
					t.second.update(actor);
				}
			}
		});
	}
	else
	{
		for (auto &t : things) 
		{
			if (ActorNodeStoppedPhysicsMap[GetActorNodeString(actor, t.first)] == false)
			{
				t.second.ActorCollisionsEnabled = CollisionsEnabled;
				t.second.GroundCollisionEnabled = GroundCollisionEnabled;
				if (strcmp(t.first, pelvis) == 0)
				{
					t.second.updatePelvis(actor);
				}
				else
				{
					t.second.update(actor);
				}
			}
		}
	}
	//logger.error("end SimObj update\n");
}

bool SimObj::updateConfig(Actor* actor) {
	for (auto &t : things) {
		//LOG("t.first:[%s]", t.first);

		std::string &section = configMap[t.first];
		//LOG("config section:[%s]", section.c_str());

		SpecificNPCBounceConfig snbc;
		if (GetSpecificNPCBounceConfigForActor(actor, snbc))
		{
			t.second.updateConfig(actor, snbc.config[section], snbc.config0weight[section]);
		}
		else
		{
			t.second.updateConfig(actor, config[section], config0weight[section]);
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