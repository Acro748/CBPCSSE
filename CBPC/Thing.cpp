#include "Thing.h"

BSFixedString leftPus("NPC L Pussy02");
BSFixedString rightPus("NPC R Pussy02");
BSFixedString belly("HDT Belly");
BSFixedString pelvis("NPC Pelvis [Pelv]");



Thing::Thing(Actor * actor, NiAVObject *obj, BSFixedString &name)
	: boneName(name)
	, velocity(NiPoint3(0, 0, 0))
{
	float nodescale = 1.0f;
	if (actor)
	{
		if (actor->loadedState && actor->loadedState->node)
		{
			//NiAVObject* obj = actor->loadedState->node->GetObjectByName(&name.data);
			if (obj)
			{
				nodescale = obj->m_worldTransform.scale;

				ownerActor = actor;
				node = obj;
			}
			else
			{
				return;
			}
		}
		else
		{
			return;
		}
	}
	else
	{
		return;
	}

	thingCollisionSpheres = CreateThingCollisionSpheres(actor, name.data, nodescale);

	oldWorldPos = obj->m_worldTransform.pos;
	time = clock();

	IsLeftBreastBone = ContainsNoCase(boneName.data, "L Breast");
	IsRightBreastBone = ContainsNoCase(boneName.data, "R Breast");
	IsBreastBone = ContainsNoCase(boneName.data, "breast");
	IsBellyBone = strcmp(boneName.data, belly.data) == 0;

	if(updateThingFirstRun)
	{
		updateThingFirstRun = false;

		auto mypair = std::make_pair(actor->baseForm->formID, name.data);
		std::map<std::pair<UInt32, const char *>, NiPoint3>::const_iterator posMap = thingDefaultPosList.find(mypair);

		if (posMap == thingDefaultPosList.end())
		{
			if(strcmp(name.data, belly.data) == 0)
			{
				thingDefaultPos = emptyPoint;
			}
			else
			{
				//Add it to the list
				thingDefaultPos = obj->m_localTransform.pos;
			}
			thingDefaultPosList[mypair] = thingDefaultPos;
			LOG("Adding %s to default list for %08x -> %g %g %g", name.data, actor->baseForm->formID, thingDefaultPos.x, thingDefaultPos.y, thingDefaultPos.z);
		}
		else
		{
			if (strcmp(name.data, belly.data) == 0)
			{
				thingDefaultPos = emptyPoint;
			}
			else
			{
				thingDefaultPos = posMap->second;
			}
		}
		LOG_INFO("%s default pos -> %g %g %g", boneName.data, thingDefaultPos.x, thingDefaultPos.y, thingDefaultPos.z);

		std::map<std::pair<UInt32, const char*>, NiMatrix33>::const_iterator rotMap = thingDefaultRotList.find(mypair);

		if (rotMap == thingDefaultRotList.end())
		{
			//Add it to the list
			thingDefaultRot = obj->m_localTransform.rot;

			thingDefaultRotList[mypair] = thingDefaultRot;
		}
		else
		{
			thingDefaultRot = rotMap->second;
		}
	}

	skipFramesCount = collisionSkipFrames;
	skipFramesPelvisCount = collisionSkipFramesPelvis;
}

Thing::~Thing() {
}

void RefreshNode(NiAVObject* node)
{
	if (node == nullptr || node->m_name == nullptr)
		return;

	if (std::find(noJitterFixNodesList.begin(), noJitterFixNodesList.end(), node->m_name) != noJitterFixNodesList.end())
		return;

	NiAVObject::ControllerUpdateContext ctx;
	ctx.flags = 0;
	ctx.delta = 0;
	node->UpdateWorldData(&ctx);
}

std::vector<Sphere> Thing::CreateThingCollisionSpheres(Actor * actor, std::string nodeName, float nodescale)
{
	auto actorRef = DYNAMIC_CAST(actor, Actor, TESObjectREFR);
	
	actorWeight = CALL_MEMBER_FN(actorRef, GetWeight)();

	std::vector<ConfigLine>* AffectedNodesListPtr;

	const char * actorrefname = "";
	std::string actorRace = "";

	SpecificNPCConfig snc;

	if (actor->formID == 0x14) //If Player
	{
		actorrefname = "Player";
	}
	else
	{
		actorrefname = CALL_MEMBER_FN(actorRef, GetReferenceName)();
	}

	if (actor->race)
	{
		actorRace = actor->race->fullName.GetName();
	}

	bool success = GetSpecificNPCConfigForActor(actor, snc);

	if (success)
	{
		AffectedNodesListPtr = &(snc.AffectedNodesList);
		thing_bellybulgemultiplier = snc.cbellybulge;
		thing_bellybulgemax = snc.cbellybulgemax;
		thing_bellybulgeposlowest = snc.cbellybulgeposlowest;
		thing_bellybulgelist = snc.bellybulgenodesList;
		thing_bellyBulgeReturnTime = snc.bellyBulgeReturnTime;
		thing_vaginaOpeningLimit = snc.vaginaOpeningLimit;
		thing_vaginaOpeningMultiplier = snc.vaginaOpeningMultiplier;
	}
	else
	{
		AffectedNodesListPtr = &AffectedNodesList;
		thing_bellybulgemultiplier = cbellybulge;
		thing_bellybulgemax = cbellybulgemax;
		thing_bellybulgeposlowest = cbellybulgeposlowest;
		thing_bellybulgelist = bellybulgenodesList;
		thing_bellyBulgeReturnTime = bellyBulgeReturnTime;
		thing_vaginaOpeningLimit = vaginaOpeningLimit;
		thing_vaginaOpeningMultiplier = vaginaOpeningMultiplier;
	}

	std::vector<Sphere> spheres;

	for (int i = 0; i < AffectedNodesListPtr->size(); i++)
	{
		if (AffectedNodesListPtr->at(i).NodeName == nodeName)
		{
			spheres = AffectedNodesListPtr->at(i).CollisionSpheres;
			IgnoredCollidersList = AffectedNodesListPtr->at(i).IgnoredColliders;
			IgnoredSelfCollidersList = AffectedNodesListPtr->at(i).IgnoredSelfColliders;
			IgnoreAllSelfColliders = AffectedNodesListPtr->at(i).IgnoreAllSelfColliders;
			for(int j=0; j<spheres.size(); j++)
			{
				spheres[j].offset100 = GetPointFromPercentage(spheres[j].offset0, spheres[j].offset100, actorWeight);

				spheres[j].radius100 = GetPercentageValue(spheres[j].radius0, spheres[j].radius100, actorWeight)*nodescale;

				spheres[j].radius100pwr2 = spheres[j].radius100*spheres[j].radius100;
			}
			break;
		}
	}
	return spheres;
}

void showPos(NiPoint3 &p) {
	LOG_INFO("%8.2f %8.2f %8.2f\n", p.x, p.y, p.z);
}

void showRot(NiMatrix33 &r) {
	LOG_INFO("%8.2f %8.2f %8.2f\n", r.data[0][0], r.data[0][1], r.data[0][2]);
	LOG_INFO("%8.2f %8.2f %8.2f\n", r.data[1][0], r.data[1][1], r.data[1][2]);
	LOG_INFO("%8.2f %8.2f %8.2f\n", r.data[2][0], r.data[2][1], r.data[2][2]);
}


float solveQuad(float a, float b, float c) {
	float k1 = (-b + sqrtf(b*b - 4*a*c)) / (2 * a);
	//float k2 = (-b - sqrtf(b*b - 4*a*c)) / (2 * a);
	//logger.error("k2 = %f\n", k2);
	return k1;
}

void Thing::updateConfigValues(Actor* actor)
{
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

	stiffness = GetPercentageValue(stiffness_0, stiffness_100, actorWeight);
	stiffness2 = GetPercentageValue(stiffness2_0, stiffness2_100, actorWeight);
	damping = GetPercentageValue(damping_0, damping_100, actorWeight);

	maxOffset = GetPercentageValue(maxOffset_0, maxOffset_100, actorWeight);
	cogOffset = GetPercentageValue(cogOffset_0, cogOffset_100, actorWeight);
	gravityBias = GetPercentageValue(gravityBias_0, gravityBias_100, actorWeight);
	gravityCorrection = GetPercentageValue(gravityCorrection_0, gravityCorrection_100, actorWeight);
	varGravityCorrection = -1 * gravityCorrection;

	timeTick = GetPercentageValue(timeTick_0, timeTick_100, actorWeight);
	linearX = GetPercentageValue(linearX_0, linearX_100, actorWeight);
	linearY = GetPercentageValue(linearY_0, linearY_100, actorWeight);
	linearZ = GetPercentageValue(linearZ_0, linearZ_100, actorWeight);
	rotationalXnew = GetPercentageValue(rotationalXnew_0, rotationalXnew_100, actorWeight);
	rotationalYnew = GetPercentageValue(rotationalYnew_0, rotationalYnew_100, actorWeight);
	rotationalZnew = GetPercentageValue(rotationalZnew_0, rotationalZnew_100, actorWeight);
	timeStep = GetPercentageValue(timeStep_0, timeStep_100, actorWeight);

	gravityInvertedCorrection = GetPercentageValue(gravityInvertedCorrection_0, gravityInvertedCorrection_100, actorWeight);
	gravityInvertedCorrectionStart = GetPercentageValue(gravityInvertedCorrectionStart_0, gravityInvertedCorrectionStart_100, actorWeight);
	gravityInvertedCorrectionEnd = GetPercentageValue(gravityInvertedCorrectionEnd_0, gravityInvertedCorrectionEnd_100, actorWeight);

	breastClothedPushup = GetPercentageValue(breastClothedPushup_0, breastClothedPushup_100, actorWeight);
	breastLightArmoredPushup = GetPercentageValue(breastLightArmoredPushup_0, breastLightArmoredPushup_100, actorWeight);
	breastHeavyArmoredPushup = GetPercentageValue(breastHeavyArmoredPushup_0, breastHeavyArmoredPushup_100, actorWeight);

	breastClothedAmplitude = GetPercentageValue(breastClothedAmplitude_0, breastClothedAmplitude_100, actorWeight);
	breastLightArmoredAmplitude = GetPercentageValue(breastLightArmoredAmplitude_0, breastLightArmoredAmplitude_100, actorWeight);
	breastHeavyArmoredAmplitude = GetPercentageValue(breastHeavyArmoredAmplitude_0, breastHeavyArmoredAmplitude_100, actorWeight);

}

void Thing::updateConfig(Actor* actor, configEntry_t & centry, configEntry_t& centry0weight) {
	//100 weight
	stiffness_100 = centry["stiffness"];
	stiffness2_100 = centry["stiffness2"];
	damping_100 = centry["damping"];
	maxOffset_100 = centry["maxoffset"];
	timeTick_100 = centry["timetick"];
	linearX_100 = centry["linearX"];
	linearY_100 = centry["linearY"];
	linearZ_100 = centry["linearZ"];
	rotationalXnew_100 = centry["rotational"];
	rotationalXnew_100 = centry["rotationalX"];
	rotationalYnew_100 = centry["rotationalY"];
	rotationalZnew_100 = centry["rotationalZ"];

	if (centry.find("timeStep") != centry.end())
		timeStep_100 = centry["timeStep"];
	else 
		timeStep_100 = 1.0f;

	gravityBias_100 = centry["gravityBias"];
	gravityCorrection_100 = centry["gravityCorrection"];
	cogOffset_100 = centry["cogOffset"];
	if (timeTick_100 <= 1)
		timeTick_100 = 1;

	gravityInvertedCorrection_100 = centry["gravityInvertedCorrection"];
	gravityInvertedCorrectionStart_100 = centry["gravityInvertedCorrectionStart"];
	gravityInvertedCorrectionEnd_100 = centry["gravityInvertedCorrectionEnd"];

	breastClothedPushup_100 = centry["breastClothedPushup"];
	breastLightArmoredPushup_100 = centry["breastLightArmoredPushup"];
	breastHeavyArmoredPushup_100 = centry["breastHeavyArmoredPushup"];

	breastClothedAmplitude_100 = centry["breastClothedAmplitude"];
	breastLightArmoredAmplitude_100 = centry["breastLightArmoredAmplitude"];
	breastHeavyArmoredAmplitude_100 = centry["breastHeavyArmoredAmplitude"];

	//0 weight
	stiffness_0 = centry0weight["stiffness"];
	stiffness2_0 = centry0weight["stiffness2"];
	damping_0 = centry0weight["damping"];
	maxOffset_0 = centry0weight["maxoffset"];
	timeTick_0 = centry0weight["timetick"];
	linearX_0 = centry0weight["linearX"];
	linearY_0 = centry0weight["linearY"];
	linearZ_0 = centry0weight["linearZ"];
	rotationalXnew_0 = centry0weight["rotational"];
	rotationalXnew_0 = centry0weight["rotationalX"];
	rotationalYnew_0 = centry0weight["rotationalY"];
	rotationalZnew_0 = centry0weight["rotationalZ"];

	if (centry0weight.find("timeStep") != centry0weight.end())
		timeStep_0 = centry0weight["timeStep"];
	else
		timeStep_0 = 1.0f;

	gravityBias_0 = centry0weight["gravityBias"];
	gravityCorrection_0 = centry0weight["gravityCorrection"];
	cogOffset_0 = centry0weight["cogOffset"];
	if (timeTick_0 <= 1)
		timeTick_0 = 1;

	gravityInvertedCorrection_0 = centry0weight["gravityInvertedCorrection"];
	gravityInvertedCorrectionStart_0 = centry0weight["gravityInvertedCorrectionStart"];
	gravityInvertedCorrectionEnd_0 = centry0weight["gravityInvertedCorrectionEnd"];

	breastClothedPushup_0 = centry0weight["breastClothedPushup"];
	breastLightArmoredPushup_0 = centry0weight["breastLightArmoredPushup"];
	breastHeavyArmoredPushup_0 = centry0weight["breastHeavyArmoredPushup"];

	breastClothedAmplitude_0 = centry0weight["breastClothedAmplitude"];
	breastLightArmoredAmplitude_0 = centry0weight["breastLightArmoredAmplitude"];
	breastHeavyArmoredAmplitude_0 = centry0weight["breastHeavyArmoredAmplitude"];

	updateConfigValues(actor);

	//zOffset = solveQuad(stiffness2, stiffness, -gravityBias);

	//logger.error("z offset = %f\n", solveQuad(stiffness2, stiffness, -gravityBias));
}

void Thing::dump() {
	//showPos(obj->m_worldTransform.pos);
	//showPos(obj->m_localTransform.pos);
}

void Thing::reset() {
	// TODO
}

template <typename T> int sgn(T val) {
	return (T(0) < val) - (val < T(0));
}

void Thing::updatePelvis(Actor *actor)
{
	if (skipFramesPelvisCount > 0)
	{
		skipFramesPelvisCount--;
		return;
	}
	else
	{
		skipFramesPelvisCount = collisionSkipFramesPelvis;
	}

	/*if (CheckPelvisArmor(actor))
	{
		return;
	}*/

	/*LARGE_INTEGER startingTime, endingTime, elapsedMicroseconds;
	LARGE_INTEGER frequency;

	QueryPerformanceFrequency(&frequency);
	QueryPerformanceCounter(&startingTime);*/

	auto loadedState = actor->loadedState;

	if (!loadedState || !loadedState->node) 
	{
		return;
	}
	if (!(*g_thePlayer) || !(*g_thePlayer)->loadedState || !(*g_thePlayer)->loadedState->node)
	{
		return;
	}

	NiAVObject* leftPusObj = loadedState->node->GetObjectByName(&leftPus.data);
	NiAVObject* rightPusObj = loadedState->node->GetObjectByName(&rightPus.data);
		
	if (!leftPusObj || !rightPusObj)
	{
		return;
	}
	else
	{
		if(updatePussyFirstRun)
		{
			updatePussyFirstRun = false;
			
			auto leftpair = std::make_pair(actor->baseForm->formID, leftPus.data);
			std::map<std::pair<UInt32, const char *>, NiPoint3>::const_iterator posMap = thingDefaultPosList.find(leftpair);

			if (posMap == thingDefaultPosList.end())
			{
				//Add it to the list
				leftPussyDefaultPos = leftPusObj->m_localTransform.pos;

				thingDefaultPosList[leftpair] = leftPussyDefaultPos;
				LOG("Adding %s to default list for %08x -> %g %g %g", leftPus.data, actor->baseForm->formID, leftPussyDefaultPos.x, leftPussyDefaultPos.y, leftPussyDefaultPos.z);

			}
			else
			{
				leftPussyDefaultPos = posMap->second;
			}

			auto rightpair = std::make_pair(actor->baseForm->formID, rightPus.data);
			posMap = thingDefaultPosList.find(rightpair);

			if (posMap == thingDefaultPosList.end())
			{
				//Add it to the list
				rightPussyDefaultPos = rightPusObj->m_localTransform.pos;

				thingDefaultPosList[rightpair] = rightPussyDefaultPos;
				LOG("Adding %s to default list for %08x -> %g %g %g", rightPus.data, actor->baseForm->formID, rightPussyDefaultPos.x, rightPussyDefaultPos.y, rightPussyDefaultPos.z);

			}
			else
			{
				rightPussyDefaultPos = posMap->second;
			}
			
			LOG_INFO("Left pussy default pos -> %g %g %g , Right pussy default pos ->  %g %g %g", leftPussyDefaultPos.x, leftPussyDefaultPos.y, leftPussyDefaultPos.z, rightPussyDefaultPos.x, rightPussyDefaultPos.y, rightPussyDefaultPos.z);
		}
		
		leftPusObj->m_localTransform.pos = leftPussyDefaultPos;

		rightPusObj->m_localTransform.pos = rightPussyDefaultPos;
	}

	if (!ActorCollisionsEnabled)
	{
		return;
	}

	// Collision Stuff Start
	NiPoint3 collisionVector = emptyPoint;

	NiMatrix33 pelvisRotation;
	NiPoint3 pelvisPosition;
	
	NiAVObject* pelvisObj = loadedState->node->GetObjectByName(&pelvis.data);
	if (!pelvisObj)
		return;
	
	pelvisRotation = pelvisObj->m_worldTransform.rot;
	pelvisPosition = pelvisObj->m_worldTransform.pos;

	std::vector<int> thingIdList;
	std::vector<int> hashIdList;
	NiPoint3 playerPos = (*g_thePlayer)->loadedState->node->m_worldTransform.pos;
	for (int i = 0; i < thingCollisionSpheres.size(); i++)
	{
		thingCollisionSpheres[i].worldPos = pelvisPosition + (pelvisRotation*thingCollisionSpheres[i].offset100);
		hashIdList = GetHashIdsFromPos(thingCollisionSpheres[i].worldPos - playerPos, thingCollisionSpheres[i].radius100);
		for (int m = 0; m<hashIdList.size(); m++)
		{
			if (!(std::find(thingIdList.begin(), thingIdList.end(), hashIdList[m]) != thingIdList.end()))
			{
				thingIdList.emplace_back(hashIdList[m]);
			}
		}
	}	

	NiPoint3 collisionDiff = emptyPoint;
		
	for (int j = 0; j < thingIdList.size(); j++)
	{
		int id = thingIdList[j];
		if(partitions.find(id) != partitions.end())
		{
			//LOG_INFO("Pelvis hashId=%d", id);
			for (int i = 0; i < partitions[id].partitionCollisions.size(); i++)
			{
				if (partitions[id].partitionCollisions[i].colliderActor == actor && partitions[id].partitionCollisions[i].colliderNodeName.find("Genital") != std::string::npos)
					continue;

				if (IgnoreAllSelfColliders && partitions[id].partitionCollisions[i].colliderActor == actor)
					continue;

				if (partitions[id].partitionCollisions[i].colliderActor == actor && std::find(IgnoredSelfCollidersList.begin(), IgnoredSelfCollidersList.end(), partitions[id].partitionCollisions[i].colliderNodeName) != IgnoredSelfCollidersList.end())
					continue;

				if (std::find(IgnoredCollidersList.begin(), IgnoredCollidersList.end(), partitions[id].partitionCollisions[i].colliderNodeName) != IgnoredCollidersList.end())
					continue;

				InterlockedIncrement(&callCount);
				partitions[id].partitionCollisions[i].CollidedWeight = actorWeight;

				collisionDiff = partitions[id].partitionCollisions[i].CheckPelvisCollision(thingCollisionSpheres, thingCollisionCapsules);
				collisionVector = collisionVector + collisionDiff;
			}
		}
	}

	// Collision Stuff End
	
	NiPoint3 leftVector = collisionVector;
	NiPoint3 rightVector = collisionVector;

	float opening = distance(collisionVector, emptyPoint);

	CalculateDiffVagina(leftVector, opening, true);
	CalculateDiffVagina(rightVector, opening, false);

	NormalizeNiPoint(leftVector, thing_vaginaOpeningLimit*-1.0f, thing_vaginaOpeningLimit);
	leftPusObj->m_localTransform.pos = leftPussyDefaultPos + leftVector;

	NormalizeNiPoint(rightVector, thing_vaginaOpeningLimit*-1.0f, thing_vaginaOpeningLimit);
	rightPusObj->m_localTransform.pos = rightPussyDefaultPos + rightVector;

	RefreshNode(leftPusObj);
	RefreshNode(rightPusObj);
	/*QueryPerformanceCounter(&endingTime);
	elapsedMicroseconds.QuadPart = endingTime.QuadPart - startingTime.QuadPart;
	elapsedMicroseconds.QuadPart *= 1000000000LL;
	elapsedMicroseconds.QuadPart /= frequency.QuadPart;
	LOG("Thing.updatePelvis() Update Time = %lld ns\n", elapsedMicroseconds.QuadPart);*/
}

bool Thing::ApplyBellyBulge(Actor * actor)
{
	if (!(*g_thePlayer) || !(*g_thePlayer)->loadedState || !(*g_thePlayer)->loadedState->node)
	{
		return false;
	}

	NiPoint3 collisionVector = emptyPoint;
	
	NiMatrix33 pelvisRotation;
	NiPoint3 pelvisPosition;

	NiAVObject* bellyObj = actor->loadedState->node->GetObjectByName(&belly.data);
	if (!bellyObj)
		return false;

	if(updateBellyFirstRun)
	{
		updateBellyFirstRun = false;

		auto mypair = std::make_pair(actor->baseForm->formID, belly.data);
		std::map<std::pair<UInt32, const char *>, NiPoint3>::const_iterator posMap = thingDefaultPosList.find(mypair);

		//if (posMap == thingDefaultPosList.end())
		//{
		//	//Add it to the list
		//	bellyDefaultPos = bellyObj->m_localTransform.pos;

		//	thingDefaultPosList[mypair] = bellyDefaultPos;
		//	LOG("Adding %s to default list for %08x -> %g %g %g", belly.data, actor->baseForm->formID, bellyDefaultPos.x, bellyDefaultPos.y, bellyDefaultPos.z);
		//}
		//else
		//{
		//	bellyDefaultPos = posMap->second;
		//}
		//
		if (posMap == thingDefaultPosList.end())
		{
			//Add it to the list
			bellyDefaultPos = emptyPoint;

			thingDefaultPosList[mypair] = bellyDefaultPos;
			LOG("Adding %s to default list for %08x -> %g %g %g", belly.data, actor->baseForm->formID, bellyDefaultPos.x, bellyDefaultPos.y, bellyDefaultPos.z);
		}
		else
		{
			bellyDefaultPos = emptyPoint;
		}
		
		LOG_INFO("Belly default pos -> %g %g %g", bellyDefaultPos.x, bellyDefaultPos.y, bellyDefaultPos.z);
	}

	NiAVObject* pelvisObj = actor->loadedState->node->GetObjectByName(&pelvis.data);
	if (!pelvisObj)
		return false;


	pelvisRotation = pelvisObj->m_worldTransform.rot;
	pelvisPosition = pelvisObj->m_worldTransform.pos;

	std::vector<int> thingIdList;
	std::vector<int> hashIdList;
	
	std::vector<Sphere> pelvisCollisionSpheres;
	std::vector<Capsule> pelvisCollisionCapsules;

	Sphere pelvisSphere;
	pelvisSphere.offset100 = NiPoint3(0, 0, -2);
	pelvisSphere.radius100 = 3.5f;
	pelvisSphere.radius100pwr2 = 12.25f;
	pelvisCollisionSpheres.emplace_back(pelvisSphere);

	NiPoint3 playerPos = (*g_thePlayer)->loadedState->node->m_worldTransform.pos;

	for (int i = 0; i < pelvisCollisionSpheres.size(); i++)
	{
		pelvisCollisionSpheres[i].worldPos = pelvisPosition + (pelvisRotation*pelvisCollisionSpheres[i].offset100);
		hashIdList = GetHashIdsFromPos(pelvisCollisionSpheres[i].worldPos - playerPos, pelvisCollisionSpheres[i].radius100);
		for (int m = 0; m<hashIdList.size(); m++)
		{
			if (!(std::find(thingIdList.begin(), thingIdList.end(), hashIdList[m]) != thingIdList.end()))
			{
				thingIdList.emplace_back(hashIdList[m]);
			}
		}
	}

	NiPoint3 collisionDiff = emptyPoint;

	bool genitalPenetration = false;

	for (int j = 0; j < thingIdList.size(); j++)
	{
		int id = thingIdList[j];
		if (partitions.find(id) != partitions.end())
		{
			for (int i = 0; i < partitions[id].partitionCollisions.size(); i++)
			{
				if (partitions[id].partitionCollisions[i].colliderActor == actor)
					continue;
				
				for (int m = 0; m < thing_bellybulgelist.size(); m++)
				{
					collisionDiff = emptyPoint;

					if (partitions[id].partitionCollisions[i].colliderNodeName.find(thing_bellybulgelist[m]) != std::string::npos)
					{
						InterlockedIncrement(&callCount);

						partitions[id].partitionCollisions[i].CollidedWeight = actorWeight;

						collisionDiff = partitions[id].partitionCollisions[i].CheckPelvisCollision(pelvisCollisionSpheres, pelvisCollisionCapsules);

						if (!CompareNiPoints(collisionDiff, emptyPoint))
						{
							genitalPenetration = true;
						}
					}

					collisionVector = collisionVector + collisionDiff;
				}
			}
		}
	}

	const float opening = distance(collisionVector, emptyPoint);

	if (opening > 0)
	{
		if (thing_bellybulgemultiplier > 0 && genitalPenetration)
		{
			//LOG("opening:%g", opening);
			bellyBulgeCountDown = 1000;
			
			float horPos = opening * thing_bellybulgemultiplier;
			horPos = clamp(horPos, 0.0f, thing_bellybulgemax);
			bellyObj->m_localTransform.pos.y = bellyDefaultPos.y + horPos;

			//float vertPos = opening * bellybulgeposmultiplier;
			//vertPos = clamp(vertPos, bellybulgeposlowest, 0.0f);
			LOG("belly bulge vert:%g horiz:%g", thing_bellybulgeposlowest, horPos);

			if (lastMaxOffsetY < horPos)
			{
				lastMaxOffsetY = abs(horPos);
			}
			if (lastMaxOffsetZ < thing_bellybulgeposlowest)
			{
				lastMaxOffsetZ = abs(thing_bellybulgeposlowest);
			}
			return true;
		}
	}
	if(bellyBulgeCountDown > 0)
	{
		bellyBulgeCountDown--;
		bellyObj->m_localTransform.pos.z = bellyDefaultPos.z + thing_bellybulgeposlowest;
	}
	return false;
}

void Thing::update(Actor *actor) {

	bool collisionsOn = true;
	if (skipFramesCount > 0)
	{
		skipFramesCount--;
		collisionsOn = false;
		if (collisionOnLastFrame)
		{
			return;
		}
	}
	else
	{
		skipFramesCount = collisionSkipFrames;	
		collisionOnLastFrame = false;
	}
		
	/*LARGE_INTEGER startingTime, endingTime, elapsedMicroseconds;
	LARGE_INTEGER frequency;

	QueryPerformanceFrequency(&frequency);
	QueryPerformanceCounter(&startingTime);
	printMessageStr("Thing.update() start", boneName.data);*/
	
	auto newTime = clock();
	auto deltaT = newTime - time;

	time = newTime;

	if (fpsCorrectionEnabled)
	{
		if (deltaT > fps60Tick * 4) deltaT = fps60Tick * 4; //edited
		if (deltaT < fps60Tick / 4) deltaT = fps60Tick / 4; //edited

		fpsCorrection = ((float)deltaT / fps60Tick); //added
	}
	else
	{
		if (deltaT > 64) deltaT = 64;
		if (deltaT < 8) deltaT = 8;
	}

	if (!(*g_thePlayer) || !(*g_thePlayer)->loadedState || !(*g_thePlayer)->loadedState->node)
	{
		return;
	}

	NiMatrix33 objRotation;

	NiAVObject* obj;		
	auto loadedState = actor->loadedState;
	
#ifdef RUNTIME_VR_VERSION_1_4_15
	if ((*g_thePlayer) && actor == (*g_thePlayer)) //To check if we can support VR Body
	{
		NiNode* rootNodeTP = (*g_thePlayer)->GetNiRootNode(0);
		
		NiNode* rootNodeFP = (*g_thePlayer)->GetNiRootNode(2);

		NiNode* mostInterestingRoot = (rootNodeFP != nullptr) ? rootNodeFP : rootNodeTP;		

		obj = ni_cast(mostInterestingRoot->GetObjectByName(&boneName.data), NiNode);
		objRotation = mostInterestingRoot->GetAsNiNode()->m_worldTransform.rot;
	}
	else
	{
	#endif
		if (!loadedState || !loadedState->node) {
			LOG("No loaded state for actor %08x\n", actor->formID);
			return;
		}

		obj = loadedState->node->GetObjectByName(&boneName.data);
		objRotation = loadedState->node->m_worldTransform.rot;
	#ifdef RUNTIME_VR_VERSION_1_4_15	
	}
	#endif
	if (!obj)
	{
		return;
	}


	if (ActorCollisionsEnabled && thing_bellybulgemultiplier > 0 && strcmp(boneName.data, belly.data) == 0)
	{
		if (ApplyBellyBulge(actor))
		{
			RefreshNode(obj);
			return;
		}
	}
	bool IsThereCollision = false;
	bool maybeNot = false;
	NiPoint3 collisionDiff = emptyPoint;
	long originalDeltaT = deltaT;
	NiPoint3 collisionVector = emptyPoint;

	float varCogOffset = cogOffset;
	
	float varGravityBias = gravityBias;

	float varLinearX = linearX;
	float varLinearY = linearY;
	float varLinearZ = linearZ;
	float varRotationalXnew = rotationalXnew;
	float varRotationalYnew = rotationalYnew;
	float varRotationalZnew = rotationalZnew;

	NiPoint3 playerPos = (*g_thePlayer)->loadedState->node->m_worldTransform.pos;

	int collisionCheckCount = 0;

	std::vector<int> thingIdList;
	std::vector<int> hashIdList;
	if (collisionsOn && ActorCollisionsEnabled)
	{
		//LOG("Before Collision Stuff Start");
		// Collision Stuff Start
		for (int i = 0; i < thingCollisionSpheres.size(); i++)
		{
			thingCollisionSpheres[i].worldPos = oldWorldPos + (objRotation*thingCollisionSpheres[i].offset100);
			//printNiPointMessage("thingCollisionSpheres[i].worldPos", thingCollisionSpheres[i].worldPos);
			hashIdList = GetHashIdsFromPos(thingCollisionSpheres[i].worldPos - playerPos, thingCollisionSpheres[i].radius100);
			for(int m=0; m<hashIdList.size(); m++)
			{
				if (std::find(thingIdList.begin(), thingIdList.end(), hashIdList[m]) == thingIdList.end())
				{
					thingIdList.emplace_back(hashIdList[m]);
				}
			}
		}

		NiPoint3 lastcollisionVector = emptyPoint;

		for (int j = 0; j < thingIdList.size(); j++)
		{
			int id = thingIdList[j];
			if (partitions.find(id) != partitions.end())
			{
				for (int i = 0; i < partitions[id].partitionCollisions.size(); i++)
				{
					if (IgnoreAllSelfColliders && partitions[id].partitionCollisions[i].colliderActor == actor)
					{
						//LOG("Ignoring collision between %s and %s because IgnoreAllSelfColliders", partitions[id].partitionCollisions[i].colliderNodeName.c_str(), boneName.data);
						continue;
					}
					if (partitions[id].partitionCollisions[i].colliderActor == actor && std::find(IgnoredSelfCollidersList.begin(), IgnoredSelfCollidersList.end(), partitions[id].partitionCollisions[i].colliderNodeName) != IgnoredSelfCollidersList.end())
					{
						//LOG("Ignoring collision between %s and %s because IgnoredSelfCollidersList", partitions[id].partitionCollisions[i].colliderNodeName.c_str(), boneName.data);
						continue;
					}

					if (std::find(IgnoredCollidersList.begin(), IgnoredCollidersList.end(), partitions[id].partitionCollisions[i].colliderNodeName) != IgnoredCollidersList.end())
					{
						//LOG("Ignoring collision between %s and %s because IgnoredCollidersList", partitions[id].partitionCollisions[i].colliderNodeName.c_str(), boneName.data);
						continue;
					}

					//Actor's own genitals are ignored
					if (partitions[id].partitionCollisions[i].colliderActor == actor && partitions[id].partitionCollisions[i].colliderNodeName.find("Genital") != std::string::npos)
						continue;

					//Actor's own same node is ignored, of course
					if (partitions[id].partitionCollisions[i].colliderActor == actor && std::strcmp(partitions[id].partitionCollisions[i].colliderNodeName.c_str(), boneName.data)==0 )
						continue;

					InterlockedIncrement(&callCount);

					partitions[id].partitionCollisions[i].CollidedWeight = actorWeight;

					if (!CompareNiPoints(lastcollisionVector, collisionVector))
					{
						for (int l = 0; l < thingCollisionSpheres.size(); l++)
						{
							thingCollisionSpheres[l].worldPos = oldWorldPos + (objRotation*thingCollisionSpheres[l].offset100) + collisionVector;
						}
					}
					lastcollisionVector = collisionVector;

					bool colliding = false;
					collisionDiff = partitions[id].partitionCollisions[i].CheckCollision(colliding, thingCollisionSpheres, thingCollisionCapsules, timeTick, originalDeltaT, maxOffset, false);
					if (colliding)
						IsThereCollision = true;

					collisionVector = collisionVector + collisionDiff*movementMultiplier;
					collisionVector.x = clamp(collisionVector.x, -maxOffset, maxOffset);
					collisionVector.y = clamp(collisionVector.y, -maxOffset, maxOffset);
					collisionVector.z = clamp(collisionVector.z, -maxOffset, maxOffset);

					collisionCheckCount++;
				}
			}
		}
		if (IsThereCollision)
		{
			float timeMultiplier = timeTick / (float)deltaT;

			collisionVector *= timeMultiplier;

			varCogOffset = 0;
			varGravityCorrection = 0;
			varGravityBias = 0;
		}
		//LOG("After Collision Stuff");
	}
		
	//3 Breast bone shaking prevention - Not working
	/*bool breastBoneCollision = false;
	if (IsBreastBone)
	{
		if (IsLeftBreastBone)
		{
			LeftBreastCollisionMap[actor][boneName.data] = IsThereCollision;
			BreastCollisionAmountMap[actor][boneName.data] = collisionVector;

			NiPoint3 totalColVector;
			int cnt = 0;
			for (auto& it : LeftBreastCollisionMap[actor])
			{
				if (it.second)
				{
					breastBoneCollision = true;
					totalColVector += BreastCollisionAmountMap[actor][boneName.data];
					cnt++;
				}
			}
			if (breastBoneCollision && cnt > 0)
			{
				collisionVector = totalColVector / cnt;
				varCogOffset = 0;
				varGravityCorrection = 0;
				varGravityBias = 0;
			}
		}
		else if (IsRightBreastBone)
		{
			RightBreastCollisionMap[actor][boneName.data] = IsThereCollision;
			BreastCollisionAmountMap[actor][boneName.data] = collisionVector;

			NiPoint3 totalColVector;
			int cnt = 0;
			for (auto& it : RightBreastCollisionMap[actor])
			{
				if (it.second)
				{
					breastBoneCollision = true;
					totalColVector += BreastCollisionAmountMap[actor][boneName.data];
					cnt++;
				}
			}
			if (breastBoneCollision && cnt > 0)
			{
				collisionVector = totalColVector / cnt;
				varCogOffset = 0;
				varGravityCorrection = 0;
				varGravityBias = 0;
			}
		}
	}*/
	//

	NiPoint3 newPos = oldWorldPos;

	NiPoint3 posDelta = emptyPoint;

	NiPoint3 target = obj->m_parent->m_worldTransform * NiPoint3(0, varCogOffset, 0);

	if (/*IsBreastBone ? !breastBoneCollision : */!IsThereCollision)
	{		
		//Get the reference bone to know which way the breasts are orientated
		NiAVObject* breastGravityReferenceBone = loadedState->node->GetObjectByName(&breastGravityReferenceBoneString.data);

		if (breastGravityReferenceBone != nullptr)
		{
			//Get the orientation (here the Z element of the rotation matrix (1.0 when standing up, -1.0 when upside down))			
			float gravityRatio = (breastGravityReferenceBone->m_worldTransform.rot.data[2][2] + 1.0f) * 0.5f;

			//Remap the value from 0.0 => 1.0 to user defined values and clamps it
			gravityRatio = remap(gravityRatio, gravityInvertedCorrectionStart, gravityInvertedCorrectionEnd, 0.0, 1.0);
			gravityRatio = clamp(gravityRatio, 0.0f, 1.0f);

			//Calculate the resulting gravity
			varGravityCorrection = (gravityRatio * gravityCorrection) + ((1.0 - gravityRatio) * gravityInvertedCorrection);

			//Code sent by KheiraDjet(modified)
			if (IsBreastBone)
			{
				//Determine which armor the actor is wearing
				if (skipArmorCheck <= 0) //This is a little heavy, check only on equip/unequip events
				{
					forceAmplitude = 1.0f;

					TESForm* wornForm = papyrusActor::GetWornForm(actor, 0x00000004);

					if (wornForm != nullptr)
					{
						TESObjectARMO* armor = DYNAMIC_CAST(wornForm, TESForm, TESObjectARMO);
						if (armor != nullptr)
						{
							isHeavyArmor = (armor->keyword.HasKeyword(KeywordArmorHeavy));
							isLightArmor = (armor->keyword.HasKeyword(KeywordArmorLight));
							isClothed = (armor->keyword.HasKeyword(KeywordArmorClothing));
						}
						else
						{
							isClothed = false;
							isLightArmor = false;
							isHeavyArmor = false;
						}
					}
					else
					{
						isClothed = false;
						isLightArmor = false;
						isHeavyArmor = false;
					}
					skipArmorCheck = 1;
				}

				if (isHeavyArmor)
				{
					varGravityCorrection = varGravityCorrection + breastHeavyArmoredPushup;
					forceAmplitude = breastHeavyArmoredAmplitude;
				}
				else if (isLightArmor)
				{
					varGravityCorrection = varGravityCorrection + breastLightArmoredPushup;
					forceAmplitude = breastLightArmoredAmplitude;
				}
				else if (isClothed)
				{
					varGravityCorrection = varGravityCorrection + breastClothedPushup;
					forceAmplitude = breastClothedAmplitude;
				}
			}
		}
		//
		
		//Offset to move Center of Mass make rotaional motion more significant  
		NiPoint3 diff = target - oldWorldPos;

		diff += NiPoint3(0, 0, varGravityCorrection);

		if (fabs(diff.x) > 100 || fabs(diff.y) > 100 || fabs(diff.z) > 100) //prevent shakes
		{
			//logger.error("transform reset\n");
			obj->m_localTransform.pos = thingDefaultPos;
			obj->m_localTransform.rot = thingDefaultRot;
			oldWorldPos = target;
			velocity = emptyPoint;
			time = clock();
			return;
		}

		float timeMultiplier = timeTick / (float)deltaT;
		diff *= timeMultiplier;

		// Compute the "Spring" Force
		NiPoint3 diff2(diff.x * diff.x * sgn(diff.x), diff.y * diff.y * sgn(diff.y), diff.z * diff.z * sgn(diff.z));
		NiPoint3 force = (diff * stiffness) + (diff2 * stiffness2) - (NiPoint3(0, 0, varGravityBias) / (fpsCorrectionEnabled ? fpsCorrection : 1.0f));
		//showPos(diff);
		//showPos(force);

		do {
			// Assume mass is 1, so Accelleration is Force, can vary mass by changinf force
			//velocity = (velocity + (force * timeStep)) * (1 - (damping * timeStep));
			velocity = (velocity + (force * timeStep)) - (velocity * (damping * timeStep)); //edited

			if (fpsCorrectionEnabled)
			{
				posDelta += (velocity * timeStep * fpsCorrection);
			}
			else
			{
				posDelta += (velocity * timeStep);
			}

			deltaT -= timeTick;
		} while (deltaT >= timeTick);

		if (collisionsOn && ActorCollisionsEnabled)
		{
			//LOG("Before Maybe Collision Stuff Start");
			NiPoint3 maybePos = newPos + posDelta;


			//After cbp movement collision detection
			thingIdList.clear();
			for (int i = 0; i < thingCollisionSpheres.size(); i++)
			{
				thingCollisionSpheres[i].worldPos = (objRotation*thingCollisionSpheres[i].offset100) + maybePos;
				hashIdList = GetHashIdsFromPos(thingCollisionSpheres[i].worldPos - playerPos, thingCollisionSpheres[i].radius100);
				for (int m = 0; m<hashIdList.size(); m++)
				{
					if (!(std::find(thingIdList.begin(), thingIdList.end(), hashIdList[m]) != thingIdList.end()))
					{
						thingIdList.emplace_back(hashIdList[m]);
					}
				}
			}
			//Prevent normal movement to cause collision (This prevents shakes)			
			collisionVector = emptyPoint;
			NiPoint3 lastcollisionVector = emptyPoint;
			for (int j = 0; j < thingIdList.size(); j++)
			{
				int id = thingIdList[j];
				//LOG_INFO("Thing hashId=%d", id);
				if (partitions.find(id) != partitions.end())
				{
					for (int i = 0; i < partitions[id].partitionCollisions.size(); i++)
					{
						if (IgnoreAllSelfColliders && partitions[id].partitionCollisions[i].colliderActor == actor)
						{
							//LOG("Ignoring collision between %s and %s because IgnoreAllSelfColliders", partitions[id].partitionCollisions[i].colliderNodeName.c_str(), boneName.data);
							continue;
						}
						if (partitions[id].partitionCollisions[i].colliderActor == actor && std::find(IgnoredSelfCollidersList.begin(), IgnoredSelfCollidersList.end(), partitions[id].partitionCollisions[i].colliderNodeName) != IgnoredSelfCollidersList.end())
						{
							//LOG("Ignoring collision between %s and %s because IgnoredSelfCollidersList", partitions[id].partitionCollisions[i].colliderNodeName.c_str(), boneName.data);
							continue;
						}

						if (std::find(IgnoredCollidersList.begin(), IgnoredCollidersList.end(), partitions[id].partitionCollisions[i].colliderNodeName) != IgnoredCollidersList.end())
						{
							//LOG("Ignoring collision between %s and %s because IgnoredCollidersList", partitions[id].partitionCollisions[i].colliderNodeName.c_str(), boneName.data);
							continue;
						}

						//Actor's own genitals are ignored
						if (partitions[id].partitionCollisions[i].colliderActor == actor && partitions[id].partitionCollisions[i].colliderNodeName.find("Genital") != std::string::npos)
							continue;

						//Actor's own same node is ignored, of course
						if (partitions[id].partitionCollisions[i].colliderActor == actor && std::strcmp(partitions[id].partitionCollisions[i].colliderNodeName.c_str(), boneName.data) == 0)
							continue;

						InterlockedIncrement(&callCount);
						
						partitions[id].partitionCollisions[i].CollidedWeight = actorWeight;

						if (!CompareNiPoints(lastcollisionVector, collisionVector))
						{
							for (int l = 0; l < thingCollisionSpheres.size(); l++)
							{
								thingCollisionSpheres[l].worldPos = (objRotation*thingCollisionSpheres[l].offset100) + maybePos + collisionVector;
							}
						}
						lastcollisionVector = collisionVector;

						bool colliding = false;
						collisionDiff = partitions[id].partitionCollisions[i].CheckCollision(colliding, thingCollisionSpheres, thingCollisionCapsules, timeTick, originalDeltaT, maxOffset, false);
						if (colliding)
						{
							velocity = emptyPoint;
							maybeNot = true;
							collisionVector = collisionVector + collisionDiff;
							collisionVector.x = clamp(collisionVector.x, -maxOffset, maxOffset);
							collisionVector.y = clamp(collisionVector.y, -maxOffset, maxOffset);
							collisionVector.z = clamp(collisionVector.z, -maxOffset, maxOffset);
						}

						collisionCheckCount++;
					}
				}
			}
			
			if (!maybeNot)
				newPos = newPos + posDelta;
			else
			{
				//Low pass filter to prevent shakes
				float absValg = abs(0.0f - varGravityCorrection);
				if (absValg < 0.1f)
				{
					absValg = 0.1f;
				}
				varGravityCorrection = varGravityCorrection + deltaT * ((0.0f - varGravityCorrection) / (30.0f * (1.0f / absValg)));

				if (IsBreastBone && forceAmplitude < 0.99999f)
				{
					float absVal = abs(1.0f - forceAmplitude);
					if (absVal < 0.1f)
					{
						absVal = 0.1f;
					}
					forceAmplitude = forceAmplitude + deltaT * ((1.0f - forceAmplitude) / (30.0f * (1.0f / absVal)));
				}
				else
				{
					forceAmplitude = 1.0f;
				}
				collisionVector *= timeMultiplier;
				newPos = maybePos + collisionVector;
				IsThereCollision = true;
			}

			//LOG("After Maybe Collision Stuff End");
		}
		else
			newPos = newPos + posDelta;
	}
	else
	{
		newPos = newPos + collisionVector;
		collisionOnLastFrame = true;
	}	
	
	// clamp the difference to stop the breast severely lagging at low framerates
	NiPoint3 diff = newPos - target;

	//Logging
	if (logging != 0)
	{
		TESObjectREFR* actorRef = DYNAMIC_CAST(actor, Actor, TESObjectREFR);
		if (actorRef)
		{
			LOG("%s - %s - Thing.update() %s - Diff: %g %g %g - Collision: %s - CheckCount: %d", CALL_MEMBER_FN(actorRef, GetReferenceName)(), IsActorMale(actor) ? "Male" : "Female", boneName.data, diff.x, diff.y, diff.z, IsThereCollision ? (maybeNot ? "mYES" : "YES") : "no", collisionCheckCount);
		}
	}
	
	oldWorldPos = diff + target;

	//logger.error("set positions\n");
	// move the bones based on the supplied weightings
	// Convert the world translations into local coordinates
	auto invRot = obj->m_parent->m_worldTransform.rot.Transpose();

	varLinearX = varLinearX * forceAmplitude;
	varLinearY = varLinearY * forceAmplitude;
	varLinearZ = varLinearZ * forceAmplitude;
	varRotationalXnew = varRotationalXnew * forceAmplitude;
	varRotationalYnew = varRotationalYnew * forceAmplitude;
	varRotationalZnew = varRotationalZnew * forceAmplitude;
			
	if (thing_bellybulgemultiplier > 0 && IsBellyBone)
	{
		auto localGravity = emptyPoint;
		if (varGravityCorrection < 0)
		{
			localGravity = invRot * NiPoint3(0, 0, varGravityCorrection);
		}
		
		float maxOffsetY = maxOffset;
		float maxOffsetZ = maxOffset;
		
		if (lastMaxOffsetY > maxOffset)
		{
			maxOffsetY = lastMaxOffsetY;
		}
		if (lastMaxOffsetZ > maxOffset)
		{
			maxOffsetZ = lastMaxOffsetZ;
		}
		
		auto ldiff = invRot * diff;
		
		oldWorldPos = (obj->m_parent->m_worldTransform.rot * ldiff) + target;
				
		float maxAllowedInOneFrame = thing_bellybulgemax / (thing_bellyBulgeReturnTime / ((float)originalDeltaT / (float)CLOCKS_PER_SEC));

		obj->m_localTransform.pos.x = thingDefaultPos.x + clamp(ldiff.x * varLinearX - localGravity.x, -maxOffset, maxOffset) + localGravity.x;
		obj->m_localTransform.pos.y = thingDefaultPos.y + clamp(ldiff.y * varLinearY - localGravity.y, 0.0f, maxOffsetY) + localGravity.y;

		if(oldBulgeY > 0 && oldBulgeY > obj->m_localTransform.pos.y)
		{
			float posDiff = clamp(oldBulgeY - obj->m_localTransform.pos.y, 0.0f, maxAllowedInOneFrame);

			obj->m_localTransform.pos.y = thingDefaultPos.y + oldBulgeY - posDiff;
		}

		if (bellyBulgeCountDown > 0)
		{
			obj->m_localTransform.pos.z = thingDefaultPos.z + thing_bellybulgeposlowest;
		}
		else
		{
			obj->m_localTransform.pos.z = thingDefaultPos.z + clamp(ldiff.z * varLinearZ - localGravity.z, -maxOffsetZ, 0.0f) + localGravity.z;
		}
		
		lastMaxOffsetY = abs(obj->m_localTransform.pos.y);
		lastMaxOffsetZ = abs(obj->m_localTransform.pos.z);

		oldBulgeY = obj->m_localTransform.pos.y;

		auto rdiffXnew = ldiff * (varRotationalXnew);
		auto rdiffYnew = ldiff * (varRotationalYnew);
		auto rdiffZnew = ldiff * (varRotationalZnew);

		NiMatrix33 newRot;
		newRot.SetEulerAngles(rdiffYnew.x, rdiffZnew.y, rdiffXnew.z);

		obj->m_localTransform.rot = thingDefaultRot * newRot;
	}
	else
	{
		diff.x = clamp(diff.x, -maxOffset, maxOffset);
		diff.y = clamp(diff.y, -maxOffset, maxOffset);
		diff.z = clamp(diff.z - varGravityCorrection, -maxOffset, maxOffset) + varGravityCorrection;
		
		auto ldiff = invRot * diff;
		
		oldWorldPos = (obj->m_parent->m_worldTransform.rot * ldiff) + target;		
		
		obj->m_localTransform.pos.x = thingDefaultPos.x + ldiff.x * varLinearX;
		obj->m_localTransform.pos.y = thingDefaultPos.y + ldiff.y * varLinearY;
		obj->m_localTransform.pos.z = thingDefaultPos.z + ldiff.z * varLinearZ;

		auto rdiffXnew = ldiff * (varRotationalXnew);
		auto rdiffYnew = ldiff * (varRotationalYnew);
		auto rdiffZnew = ldiff * (varRotationalZnew);
		NiMatrix33 newRot;
		newRot.SetEulerAngles(rdiffYnew.x, rdiffZnew.y, rdiffXnew.z);
		
		obj->m_localTransform.rot = thingDefaultRot * newRot;
	}

	RefreshNode(obj);

	//logger.error("end update()\n");
	/*QueryPerformanceCounter(&endingTime);
	elapsedMicroseconds.QuadPart = endingTime.QuadPart - startingTime.QuadPart;
	elapsedMicroseconds.QuadPart *= 1000000000LL;
	elapsedMicroseconds.QuadPart /= frequency.QuadPart;
	LOG("Thing.update() Update Time = %lld ns\n", elapsedMicroseconds.QuadPart);*/
}

void Thing::CalculateDiffVagina(NiPoint3 &collisionDiff, float opening, bool left)
{
	if (opening > 0)
	{
		if (left)
		{
			collisionDiff = NiPoint3(thing_vaginaOpeningMultiplier*-1, 0, 0)*(opening / 3);
		}
		else
		{
			collisionDiff = NiPoint3(thing_vaginaOpeningMultiplier, 0, 0)*(opening / 3);
		}
	}
	else
	{
		collisionDiff = emptyPoint;
	}
}

