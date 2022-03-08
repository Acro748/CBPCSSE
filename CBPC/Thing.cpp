#include "Thing.h"

BSFixedString leftPus("NPC L Pussy02");
BSFixedString rightPus("NPC R Pussy02");
BSFixedString belly("HDT Belly");
BSFixedString pelvis("NPC Pelvis [Pelv]");
BSFixedString highheel("NPC");


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
				spheres[j].offset100 = GetPointFromPercentage(spheres[j].offset0, spheres[j].offset100, actorWeight) * nodescale;

				spheres[j].radius100 = GetPercentageValue(spheres[j].radius0, spheres[j].radius100, actorWeight) * nodescale;

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

	//maxOffset = GetPercentageValue(maxOffset_0, maxOffset_100, actorWeight);
	XmaxOffset = GetPercentageValue(XmaxOffset_0, XmaxOffset_100, actorWeight);
	XminOffset = GetPercentageValue(XminOffset_0, XminOffset_100, actorWeight);
	YmaxOffset = GetPercentageValue(YmaxOffset_0, YmaxOffset_100, actorWeight);
	YminOffset = GetPercentageValue(YminOffset_0, YminOffset_100, actorWeight);
	ZmaxOffset = GetPercentageValue(ZmaxOffset_0, ZmaxOffset_100, actorWeight);
	ZminOffset = GetPercentageValue(ZminOffset_0, ZminOffset_100, actorWeight);
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
	linearXrotationX = GetPercentageValue(linearXrotationX_0, linearXrotationX_100, actorWeight);
	linearXrotationY = GetPercentageValue(linearXrotationY_0, linearXrotationY_100, actorWeight);
	linearXrotationZ = GetPercentageValue(linearXrotationZ_0, linearXrotationZ_100, actorWeight);
	linearYrotationX = GetPercentageValue(linearYrotationX_0, linearYrotationX_100, actorWeight);
	linearYrotationY = GetPercentageValue(linearYrotationY_0, linearYrotationY_100, actorWeight);
	linearYrotationZ = GetPercentageValue(linearYrotationZ_0, linearYrotationZ_100, actorWeight);
	linearZrotationX = GetPercentageValue(linearZrotationX_0, linearZrotationX_100, actorWeight);
	linearZrotationY = GetPercentageValue(linearZrotationY_0, linearZrotationY_100, actorWeight);
	linearZrotationZ = GetPercentageValue(linearZrotationZ_0, linearZrotationZ_100, actorWeight);
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

	collisionFriction = GetPercentageValue(collisionFriction_0, collisionFriction_100, actorWeight);
	collisionPenetration = GetPercentageValue(collisionPenetration_0, collisionPenetration_100, actorWeight);

	collisionXmaxOffset = GetPercentageValue(collisionXmaxOffset_0, collisionXmaxOffset_100, actorWeight);
	collisionXminOffset = GetPercentageValue(collisionXminOffset_0, collisionXminOffset_100, actorWeight);
	collisionYmaxOffset = GetPercentageValue(collisionYmaxOffset_0, collisionYmaxOffset_100, actorWeight);
	collisionYminOffset = GetPercentageValue(collisionYminOffset_0, collisionYminOffset_100, actorWeight);
	collisionZmaxOffset = GetPercentageValue(collisionZmaxOffset_0, collisionZmaxOffset_100, actorWeight);
	collisionZminOffset = GetPercentageValue(collisionZminOffset_0, collisionZminOffset_100, actorWeight);

}

void Thing::updateConfig(Actor* actor, configEntry_t & centry, configEntry_t& centry0weight) {
	//100 weight
	stiffness_100 = centry["stiffness"];
	stiffness2_100 = centry["stiffness2"];
	damping_100 = centry["damping"];
	maxOffset_100 = centry["maxoffset"];
	if (maxOffset_100 >= 0.01f)
	{
		XmaxOffset_100 = maxOffset_100;
		XminOffset_100 = -maxOffset_100;
		YmaxOffset_100 = maxOffset_100;
		YminOffset_100 = -maxOffset_100;
		ZmaxOffset_100 = maxOffset_100;
		ZminOffset_100 = -maxOffset_100;
	}
	else
	{
		XmaxOffset_100 = centry["Xmaxoffset"];
		XminOffset_100 = centry["Xminoffset"];
		YmaxOffset_100 = centry["Ymaxoffset"];
		YminOffset_100 = centry["Yminoffset"];
		ZmaxOffset_100 = centry["Zmaxoffset"];
		ZminOffset_100 = centry["Zminoffset"];
	}
	timeTick_100 = centry["timetick"];
	if (timeTick_100 <= 1)
		timeTick_100 = 1;
	linearX_100 = centry["linearX"];
	linearY_100 = centry["linearY"];
	linearZ_100 = centry["linearZ"];
	rotationalXnew_100 = centry["rotational"];
	rotationalXnew_100 = centry["rotationalX"];
	rotationalYnew_100 = centry["rotationalY"];
	rotationalZnew_100 = centry["rotationalZ"];

	linearXrotationX_100 = centry["linearXrotationX"];
	linearXrotationY_100 = centry["linearXrotationY"];
	linearXrotationZ_100 = centry["linearXrotationZ"];
	linearYrotationX_100 = centry["linearYrotationX"];
	linearYrotationY_100 = centry["linearYrotationY"];
	linearYrotationZ_100 = centry["linearYrotationZ"];
	linearZrotationX_100 = centry["linearZrotationX"];
	linearZrotationY_100 = centry["linearZrotationY"];
	linearZrotationZ_100 = centry["linearZrotationZ"];

	if (centry.find("timeStep") != centry.end())
		timeStep_100 = centry["timeStep"];
	else 
		timeStep_100 = 1.0f;

	gravityBias_100 = centry["gravityBias"];
	gravityCorrection_100 = centry["gravityCorrection"];
	cogOffset_100 = centry["cogOffset"];

	gravityInvertedCorrection_100 = centry["gravityInvertedCorrection"];
	gravityInvertedCorrectionStart_100 = centry["gravityInvertedCorrectionStart"];
	gravityInvertedCorrectionEnd_100 = centry["gravityInvertedCorrectionEnd"];

	breastClothedPushup_100 = centry["breastClothedPushup"];
	breastLightArmoredPushup_100 = centry["breastLightArmoredPushup"];
	breastHeavyArmoredPushup_100 = centry["breastHeavyArmoredPushup"];

	breastClothedAmplitude_100 = centry["breastClothedAmplitude"];
	breastLightArmoredAmplitude_100 = centry["breastLightArmoredAmplitude"];
	breastHeavyArmoredAmplitude_100 = centry["breastHeavyArmoredAmplitude"];

	collisionFriction_100 = 1.0f - centry["collisionFriction"];
	if (collisionFriction_100 >= 1)
		collisionFriction_100 = 1.0f;
	else if (collisionFriction_100 <= 0)
		collisionFriction_100 = 0.0f;

	collisionPenetration_100 = 1.0f - centry["collisionPenetration"];
	if (collisionPenetration_100 >= 1)
		collisionPenetration_100 = 1.0f;
	else if (collisionPenetration_100 <= 0)
		collisionPenetration_100 = 0.0f;

	collisionXmaxOffset_100 = centry["collisionXmaxoffset"];
	collisionXminOffset_100 = centry["collisionXminoffset"];
	collisionYmaxOffset_100 = centry["collisionYmaxoffset"];
	collisionYminOffset_100 = centry["collisionYminoffset"];
	collisionZmaxOffset_100 = centry["collisionZmaxoffset"];
	collisionZminOffset_100 = centry["collisionZminoffset"];



	//0 weight
	stiffness_0 = centry0weight["stiffness"];
	stiffness2_0 = centry0weight["stiffness2"];
	damping_0 = centry0weight["damping"];
	maxOffset_0 = centry0weight["maxoffset"];
	if (maxOffset_0 >= 0.01f)
	{
		XmaxOffset_0 = maxOffset_0;
		XminOffset_0 = -maxOffset_0;
		YmaxOffset_0 = maxOffset_0;
		YminOffset_0 = -maxOffset_0;
		ZmaxOffset_0 = maxOffset_0;
		ZminOffset_0 = -maxOffset_0;
	}
	else
	{
		XmaxOffset_0 = centry0weight["Xmaxoffset"];
		XminOffset_0 = centry0weight["Xminoffset"];
		YmaxOffset_0 = centry0weight["Ymaxoffset"];
		YminOffset_0 = centry0weight["Yminoffset"];
		ZmaxOffset_0 = centry0weight["Zmaxoffset"];
		ZminOffset_0 = centry0weight["Zminoffset"];
	}
	timeTick_0 = centry0weight["timetick"];
	if (timeTick_0 <= 1)
		timeTick_0 = 1;
	linearX_0 = centry0weight["linearX"];
	linearY_0 = centry0weight["linearY"];
	linearZ_0 = centry0weight["linearZ"];
	rotationalXnew_0 = centry0weight["rotational"];
	rotationalXnew_0 = centry0weight["rotationalX"];
	rotationalYnew_0 = centry0weight["rotationalY"];
	rotationalZnew_0 = centry0weight["rotationalZ"];

	linearXrotationX_0 = centry["linearXrotationX"];
	linearXrotationY_0 = centry["linearXrotationY"];
	linearXrotationZ_0 = centry["linearXrotationZ"];
	linearYrotationX_0 = centry["linearYrotationX"];
	linearYrotationY_0 = centry["linearYrotationY"];
	linearYrotationZ_0 = centry["linearYrotationZ"];
	linearZrotationX_0 = centry["linearZrotationX"];
	linearZrotationY_0 = centry["linearZrotationY"];
	linearZrotationZ_0 = centry["linearZrotationZ"];

	if (centry0weight.find("timeStep") != centry0weight.end())
		timeStep_0 = centry0weight["timeStep"];
	else
		timeStep_0 = 1.0f;

	gravityBias_0 = centry0weight["gravityBias"];
	gravityCorrection_0 = centry0weight["gravityCorrection"];
	cogOffset_0 = centry0weight["cogOffset"];

	gravityInvertedCorrection_0 = centry0weight["gravityInvertedCorrection"];
	gravityInvertedCorrectionStart_0 = centry0weight["gravityInvertedCorrectionStart"];
	gravityInvertedCorrectionEnd_0 = centry0weight["gravityInvertedCorrectionEnd"];

	breastClothedPushup_0 = centry0weight["breastClothedPushup"];
	breastLightArmoredPushup_0 = centry0weight["breastLightArmoredPushup"];
	breastHeavyArmoredPushup_0 = centry0weight["breastHeavyArmoredPushup"];

	breastClothedAmplitude_0 = centry0weight["breastClothedAmplitude"];
	breastLightArmoredAmplitude_0 = centry0weight["breastLightArmoredAmplitude"];
	breastHeavyArmoredAmplitude_0 = centry0weight["breastHeavyArmoredAmplitude"];

	collisionFriction_0 = 1.0f - centry0weight["collisionFriction"];
	if (collisionFriction_0 >= 1)
		collisionFriction_0 = 1.0f;
	else if (collisionFriction_0 <= 0)
		collisionFriction_0 = 0.0f;

	collisionPenetration_0 = 1.0f - centry0weight["collisionPenetration"];
	if (collisionPenetration_0 >= 1)
		collisionPenetration_0 = 1.0f;
	else if (collisionPenetration_0 <= 0)
		collisionPenetration_0 = 0.0f;

	collisionXmaxOffset_0 = centry0weight["collisionXmaxoffset"];
	collisionXminOffset_0 = centry0weight["collisionXminoffset"];
	collisionYmaxOffset_0 = centry0weight["collisionYmaxoffset"];
	collisionYminOffset_0 = centry0weight["collisionYminoffset"];
	collisionZmaxOffset_0 = centry0weight["collisionZmaxoffset"];
	collisionZminOffset_0 = centry0weight["collisionZminoffset"];


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

void Thing::update(Actor* actor) {

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

	float varGravityBias = gravityBias;

	const float nodeScale = obj->m_worldTransform.scale;

	const float ScaleMult = 1 / nodeScale;

	float varLinearX = linearX * ScaleMult;
	float varLinearY = linearY * ScaleMult;
	float varLinearZ = linearZ * ScaleMult;
	float varRotationalXnew = rotationalXnew;
	float varRotationalYnew = rotationalYnew;
	float varRotationalZnew = rotationalZnew;

	NiPoint3 playerPos = (*g_thePlayer)->loadedState->node->m_worldTransform * NiPoint3(0, cogOffset, 0);

	int collisionCheckCount = 0;


	std::vector<int> thingIdList;
	std::vector<int> hashIdList;

	NiPoint3 newPos = oldWorldPos;

	NiPoint3 posDelta = emptyPoint;

	NiPoint3 target = obj->m_parent->m_worldTransform.pos;

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
						if (IsLeftBreastBone)
						{
							if (armor->keyword.HasKeyword(KeywordAsNakadL))
							{
								isClothed = false;
								isLightArmor = false;
								isHeavyArmor = false;
							}
							else if (!(isHeavyArmor = (armor->keyword.HasKeyword(KeywordAsHeavyL))) &&
								!(isLightArmor = (armor->keyword.HasKeyword(KeywordAsLightL))) &&
								!(isClothed = (armor->keyword.HasKeyword(KeywordAsClothingL))))
							{
								isHeavyArmor = (armor->keyword.HasKeyword(KeywordArmorHeavy));
								isLightArmor = (armor->keyword.HasKeyword(KeywordArmorLight));
								isClothed = (armor->keyword.HasKeyword(KeywordArmorClothing));
							}

							isNoPushUp = (armor->keyword.HasKeyword(KeywordNoPushUpL));

						}
						else if (IsRightBreastBone)
						{
							if (armor->keyword.HasKeyword(KeywordAsNakadR))
							{
								isClothed = false;
								isLightArmor = false;
								isHeavyArmor = false;
							}
							else if (!(isHeavyArmor = (armor->keyword.HasKeyword(KeywordAsHeavyR))) &&
								!(isLightArmor = (armor->keyword.HasKeyword(KeywordAsLightR))) &&
								!(isClothed = (armor->keyword.HasKeyword(KeywordAsClothingR))))
							{
								isHeavyArmor = (armor->keyword.HasKeyword(KeywordArmorHeavy));
								isLightArmor = (armor->keyword.HasKeyword(KeywordArmorLight));
								isClothed = (armor->keyword.HasKeyword(KeywordArmorClothing));
							}

							isNoPushUp = (armor->keyword.HasKeyword(KeywordNoPushUpR));
						}
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
				if (!isNoPushUp)
					varGravityCorrection = varGravityCorrection + breastHeavyArmoredPushup;
				forceAmplitude = breastHeavyArmoredAmplitude;
			}
			else if (isLightArmor)
			{
				if (!isNoPushUp)
					varGravityCorrection = varGravityCorrection + breastLightArmoredPushup;
				forceAmplitude = breastLightArmoredAmplitude;
			}
			else if (isClothed)
			{
				if (!isNoPushUp)
					varGravityCorrection = varGravityCorrection + breastClothedPushup;
				forceAmplitude = breastClothedAmplitude;
			}
		}
	}

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
		if (GroundCollisionEnabled)
		{
			if (skipHighheelCheck > 0)
			{
				if (skipHighheelCheck <= 0)
				{
					NiAVObject* highheelobj;
					highheelobj = loadedState->node->GetObjectByName(&highheel.data);

					if (highheelobj)
					{
						highheelOffset = highheelobj->m_localTransform.pos.z; //Get ground by NPC Root node
					}
				}
				skipHighheelCheck--; //When the equipment is switched, the high heel offset is not applied immediately, so wait some frames
			}

			NiAVObject* groundobj;
			groundobj = loadedState->node->GetObjectByName(&GroundReferenceBone.data);
			if (groundobj)
			{
				auto groundWPos = groundobj->m_worldTransform.pos;
				groundPos = groundWPos.z - highheelOffset; //Get ground by NPC Root node
			}
		}
		//LOG("Before Maybe Collision Stuff Start");
		NiPoint3 maybePos = newPos + posDelta + (objRotation * (thingDefaultPos * nodeScale)); //add missing local pos

		//After cbp movement collision detection
		thingIdList.clear();
		for (int i = 0; i < thingCollisionSpheres.size(); i++)
		{
			thingCollisionSpheres[i].worldPos = (maybePos + objRotation * thingCollisionSpheres[i].offset100);
			hashIdList = GetHashIdsFromPos(thingCollisionSpheres[i].worldPos - playerPos, thingCollisionSpheres[i].radius100);
			for (int m = 0; m < hashIdList.size(); m++)
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
							thingCollisionSpheres[l].worldPos = (maybePos + objRotation * thingCollisionSpheres[l].offset100) + collisionVector;
						}
					}
					lastcollisionVector = collisionVector;

					bool colliding = false;
					collisionDiff = partitions[id].partitionCollisions[i].CheckCollision(colliding, thingCollisionSpheres, thingCollisionCapsules, timeTick, originalDeltaT, false, groundPos);
					if (colliding)
					{
						velocity *= collisionPenetration;
						maybeNot = true;
						collisionVector = collisionVector + collisionDiff;
					}

					collisionCheckCount++;
				}
			}
		}

		newPos = newPos + posDelta;

		if (maybeNot)
		{
			collisionVector.x = clamp(collisionVector.x, collisionXminOffset, collisionXmaxOffset);
			collisionVector.y = clamp(collisionVector.y, collisionYminOffset, collisionYmaxOffset);
			collisionVector.z = clamp(collisionVector.z, collisionZminOffset, collisionZmaxOffset);
			collisionVector = collisionVector * collisionFriction;
			IsThereCollision = true;
		}

		//LOG("After Maybe Collision Stuff End");
	}
	else
	{
		newPos = newPos + posDelta;
	}
	
	// clamp the difference to stop the breast severely lagging at low framerates
	NiPoint3 newdiff = newPos - target;

	//Logging
	if (logging != 0)
	{
		TESObjectREFR* actorRef = DYNAMIC_CAST(actor, Actor, TESObjectREFR);
		if (actorRef)
		{
			LOG("%s - %s - Thing.update() %s - Diff: %g %g %g - Collision: %s - CheckCount: %d", CALL_MEMBER_FN(actorRef, GetReferenceName)(), IsActorMale(actor) ? "Male" : "Female", boneName.data, diff.x, diff.y, diff.z, IsThereCollision ? (maybeNot ? "mYES" : "YES") : "no", collisionCheckCount);
		}
	}
	
	oldWorldPos = newdiff + collisionVector + target;

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

	newdiff.x = clamp(newdiff.x, XminOffset, XmaxOffset);
	newdiff.y = clamp(newdiff.y, YminOffset, YmaxOffset);
	newdiff.z = clamp(newdiff.z - varGravityCorrection, ZminOffset, ZmaxOffset) + varGravityCorrection;

	auto ldiff = invRot * newdiff;
	auto Idiffcol = invRot * collisionVector;
		
	oldWorldPos = (obj->m_parent->m_worldTransform.rot * ldiff) + target;
		
	obj->m_localTransform.pos.x = thingDefaultPos.x + ldiff.x * varLinearX + Idiffcol.x;
	obj->m_localTransform.pos.y = thingDefaultPos.y + ldiff.y * varLinearY + Idiffcol.y;
	obj->m_localTransform.pos.z = thingDefaultPos.z + ldiff.z * varLinearZ + Idiffcol.z;

	auto rdiffXnew = (ldiff + Idiffcol) * (varRotationalXnew);
	auto rdiffYnew = (ldiff + Idiffcol) * (varRotationalYnew);
	auto rdiffZnew = (ldiff + Idiffcol) * (varRotationalZnew);

	rdiffXnew.x *= linearXrotationX;
	rdiffXnew.y *= linearYrotationX;
	rdiffXnew.z *= linearZrotationX;

	rdiffYnew.x *= linearXrotationY;
	rdiffYnew.y *= linearYrotationY;
	rdiffYnew.z *= linearZrotationY;

	rdiffZnew.x *= linearXrotationZ;
	rdiffZnew.y *= linearYrotationZ;
	rdiffZnew.z *= linearZrotationZ;

	NiMatrix33 newRot;
	newRot.SetEulerAngles(rdiffYnew.x + rdiffYnew.y + rdiffYnew.z, rdiffZnew.x + rdiffZnew.y + rdiffZnew.z, rdiffXnew.x + rdiffXnew.y + rdiffXnew.z);
		
	obj->m_localTransform.rot = thingDefaultRot * newRot;
	

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

