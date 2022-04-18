#include "Thing.h"

BSFixedString leftPus("NPC L Pussy02");
BSFixedString rightPus("NPC R Pussy02");
BSFixedString backPus("VaginaB1");
BSFixedString frontPus("Clitoral1");
BSFixedString belly("HDT Belly");
BSFixedString pelvis("NPC Pelvis [Pelv]");
BSFixedString spine1("NPC Spine1 [Spn1]");
BSFixedString highheel("NPC");

//## thing_map_lock
// Maps are sorted every edit time, so if it is parallel processing then a high probability of overloading

std::shared_mutex thing_map_lock;

Thing::Thing(Actor * actor, NiAVObject *obj, BSFixedString &name)
	: boneName(name)
	, velocity(NiPoint3(0, 0, 0))
	, velocityRot(NiPoint3(0, 0, 0))
{
	if (actor)
	{
		if (actor->loadedState && actor->loadedState->node)
		{
			//NiAVObject* obj = actor->loadedState->node->GetObjectByName(&name.data);
			if (obj)
			{
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

	oldWorldPos = obj->m_worldTransform.pos;
	oldWorldPosRot = obj->m_worldTransform.pos;
	time = clock();

	IsLeftBreastBone = ContainsNoCase(boneName.data, "L Breast");
	IsRightBreastBone = ContainsNoCase(boneName.data, "R Breast");
	IsBreastBone = ContainsNoCase(boneName.data, "Breast");
	IsBellyBone = strcmp(boneName.data, belly.data) == 0;

	if (IsBellyBone)
	{
		//thingCollisionSpheres = CreateThingCollisionSpheres(actor, pelvis.data);
		//thingCollisionCapsules = CreateThingCollisionCapsules(actor, pelvis.data);
		thingCollisionSpheres = CreateThingCollisionSpheres(actor, spine1.data); //suggest reading comments on the bellybullge function
		thingCollisionCapsules = CreateThingCollisionCapsules(actor, spine1.data);
	}
	else
	{
		thingCollisionSpheres = CreateThingCollisionSpheres(actor, name.data);
		thingCollisionCapsules = CreateThingCollisionCapsules(actor, name.data);
	}

	if(updateThingFirstRun)
	{
		updateThingFirstRun = false;

		auto mypair = std::make_pair(actor->baseForm->formID, name.data);

		thing_map_lock.lock();
		std::map<std::pair<UInt32, const char*>, NiPoint3>::const_iterator posMap = thingDefaultPosList.find(mypair);

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
		thing_map_lock.unlock();
		collisionBuffer = emptyPoint;
		collisionSync = emptyPoint;
	}

	skipFramesCount = collisionSkipFrames;
	skipFramesPelvisCount = collisionSkipFramesPelvis;
}

Thing::~Thing() {
}

void RefreshNode(NiAVObject* node, std::shared_mutex& thing_Refresh_node_lock)
{
	if (node == nullptr || node->m_name == nullptr)
		return;

	if (std::find(noJitterFixNodesList.begin(), noJitterFixNodesList.end(), node->m_name) != noJitterFixNodesList.end())
		return;

	NiAVObject::ControllerUpdateContext ctx;
	ctx.flags = 0;
	ctx.delta = 0;

	thing_Refresh_node_lock.lock();
	node->UpdateWorldData(&ctx);
	thing_Refresh_node_lock.unlock();
}

std::vector<Sphere> Thing::CreateThingCollisionSpheres(Actor * actor, std::string nodeName)
{
	auto actorRef = DYNAMIC_CAST(actor, Actor, TESObjectREFR);
	
	actorWeight = CALL_MEMBER_FN(actorRef, GetWeight)();

	concurrency::concurrent_vector<ConfigLine>* AffectedNodesListPtr;

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

	concurrency::parallel_for(size_t(0), AffectedNodesListPtr->size(), [&](size_t i)
	{
		if (AffectedNodesListPtr->at(i).NodeName == nodeName)
		{
			spheres = AffectedNodesListPtr->at(i).CollisionSpheres;
			IgnoredCollidersList = AffectedNodesListPtr->at(i).IgnoredColliders;
			IgnoredSelfCollidersList = AffectedNodesListPtr->at(i).IgnoredSelfColliders;
			IgnoreAllSelfColliders = AffectedNodesListPtr->at(i).IgnoreAllSelfColliders;
			for (int j = 0; j < spheres.size(); j++)
			{
				spheres[j].offset0 = GetPointFromPercentage(spheres[j].offset0, spheres[j].offset100, actorWeight);

				spheres[j].radius0 = GetPercentageValue(spheres[j].radius0, spheres[j].radius100, actorWeight);

				spheres[j].radius100pwr2 = spheres[j].radius0 * spheres[j].radius0;
			}
		}
	});
	return spheres;
}

std::vector<Capsule> Thing::CreateThingCollisionCapsules(Actor* actor, std::string nodeName)
{
	auto actorRef = DYNAMIC_CAST(actor, Actor, TESObjectREFR);

	actorWeight = CALL_MEMBER_FN(actorRef, GetWeight)();

	concurrency::concurrent_vector<ConfigLine>* AffectedNodesListPtr;

	const char* actorrefname = "";
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

	std::vector<Capsule> capsules;

	concurrency::parallel_for(size_t(0), AffectedNodesListPtr->size(), [&](size_t i)
	{
		if (AffectedNodesListPtr->at(i).NodeName == nodeName)
		{
			capsules = AffectedNodesListPtr->at(i).CollisionCapsules;
			IgnoredCollidersList = AffectedNodesListPtr->at(i).IgnoredColliders;
			IgnoredSelfCollidersList = AffectedNodesListPtr->at(i).IgnoredSelfColliders;
			IgnoreAllSelfColliders = AffectedNodesListPtr->at(i).IgnoreAllSelfColliders;
			for (int j = 0; j < capsules.size(); j++)
			{
				capsules[j].End1_offset0 = GetPointFromPercentage(capsules[j].End1_offset0, capsules[j].End1_offset100, actorWeight);

				capsules[j].End1_radius0 = GetPercentageValue(capsules[j].End1_radius0, capsules[j].End1_radius100, actorWeight);

				capsules[j].End1_radius100pwr2 = capsules[j].End1_radius0 * capsules[j].End1_radius0;

				capsules[j].End2_offset0 = GetPointFromPercentage(capsules[j].End2_offset0, capsules[j].End2_offset100, actorWeight);

				capsules[j].End2_radius0 = GetPercentageValue(capsules[j].End2_radius0, capsules[j].End2_radius100, actorWeight);

				capsules[j].End2_radius100pwr2 = capsules[j].End2_radius0 * capsules[j].End2_radius0;
			}
		}
	});
	return capsules;
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
	stiffness = GetPercentageValue(stiffness_0, stiffness_100, actorWeight);
	stiffnessX = GetPercentageValue(stiffnessX_0, stiffnessX_100, actorWeight);
	stiffnessY = GetPercentageValue(stiffnessY_0, stiffnessY_100, actorWeight);
	stiffnessZ = GetPercentageValue(stiffnessZ_0, stiffnessZ_100, actorWeight);
	stiffnessXRot = GetPercentageValue(stiffnessXRot_0, stiffnessXRot_100, actorWeight);
	stiffnessYRot = GetPercentageValue(stiffnessYRot_0, stiffnessYRot_100, actorWeight);
	stiffnessZRot = GetPercentageValue(stiffnessZRot_0, stiffnessZRot_100, actorWeight);
	stiffness2 = GetPercentageValue(stiffness2_0, stiffness2_100, actorWeight);
	stiffness2X = GetPercentageValue(stiffness2X_0, stiffness2X_100, actorWeight);
	stiffness2Y = GetPercentageValue(stiffness2Y_0, stiffness2Y_100, actorWeight);
	stiffness2Z = GetPercentageValue(stiffness2Z_0, stiffness2Z_100, actorWeight);
	stiffness2XRot = GetPercentageValue(stiffness2XRot_0, stiffness2XRot_100, actorWeight);
	stiffness2YRot = GetPercentageValue(stiffness2YRot_0, stiffness2YRot_100, actorWeight);
	stiffness2ZRot = GetPercentageValue(stiffness2ZRot_0, stiffness2ZRot_100, actorWeight);
	damping = GetPercentageValue(damping_0, damping_100, actorWeight);
	dampingX = GetPercentageValue(dampingX_0, dampingX_100, actorWeight);
	dampingY = GetPercentageValue(dampingY_0, dampingY_100, actorWeight);
	dampingZ = GetPercentageValue(dampingZ_0, dampingZ_100, actorWeight);
	dampingXRot = GetPercentageValue(dampingXRot_0, dampingXRot_100, actorWeight);
	dampingYRot = GetPercentageValue(dampingYRot_0, dampingYRot_100, actorWeight);
	dampingZRot = GetPercentageValue(dampingZRot_0, dampingZRot_100, actorWeight);

	//maxOffset = GetPercentageValue(maxOffset_0, maxOffset_100, actorWeight);
	XmaxOffset = GetPercentageValue(XmaxOffset_0, XmaxOffset_100, actorWeight);
	XminOffset = GetPercentageValue(XminOffset_0, XminOffset_100, actorWeight);
	YmaxOffset = GetPercentageValue(YmaxOffset_0, YmaxOffset_100, actorWeight);
	YminOffset = GetPercentageValue(YminOffset_0, YminOffset_100, actorWeight);
	ZmaxOffset = GetPercentageValue(ZmaxOffset_0, ZmaxOffset_100, actorWeight);
	ZminOffset = GetPercentageValue(ZminOffset_0, ZminOffset_100, actorWeight);
	XmaxOffsetRot = GetPercentageValue(XmaxOffsetRot_0, XmaxOffsetRot_100, actorWeight);
	XminOffsetRot = GetPercentageValue(XminOffsetRot_0, XminOffsetRot_100, actorWeight);
	YmaxOffsetRot = GetPercentageValue(YmaxOffsetRot_0, YmaxOffsetRot_100, actorWeight);
	YminOffsetRot = GetPercentageValue(YminOffsetRot_0, YminOffsetRot_100, actorWeight);
	ZmaxOffsetRot = GetPercentageValue(ZmaxOffsetRot_0, ZmaxOffsetRot_100, actorWeight);
	ZminOffsetRot = GetPercentageValue(ZminOffsetRot_0, ZminOffsetRot_100, actorWeight);
	XdefaultOffset = GetPercentageValue(XdefaultOffset_0, XdefaultOffset_100, actorWeight);
	YdefaultOffset = GetPercentageValue(YdefaultOffset_0, YdefaultOffset_100, actorWeight);
	ZdefaultOffset = GetPercentageValue(ZdefaultOffset_0, ZdefaultOffset_100, actorWeight);
	cogOffset = GetPercentageValue(cogOffset_0, cogOffset_100, actorWeight);
	gravityBias = GetPercentageValue(gravityBias_0, gravityBias_100, actorWeight);
	gravityCorrection = GetPercentageValue(gravityCorrection_0, gravityCorrection_100, actorWeight);
	varGravityCorrection = -1 * gravityCorrection;

	timeTick = GetPercentageValue(timeTick_0, timeTick_100, actorWeight);
	timeTickRot = GetPercentageValue(timeTickRot_0, timeTickRot_100, actorWeight);
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
	timeStepRot = GetPercentageValue(timeStepRot_0, timeStepRot_100, actorWeight);

	linearXspreadforceY = GetPercentageValue(linearXspreadforceY_0, linearXspreadforceY_100, actorWeight);
	linearXspreadforceZ = GetPercentageValue(linearXspreadforceZ_0, linearXspreadforceZ_100, actorWeight);
	linearYspreadforceX = GetPercentageValue(linearYspreadforceX_0, linearYspreadforceX_100, actorWeight);
	linearYspreadforceZ = GetPercentageValue(linearYspreadforceZ_0, linearYspreadforceZ_100, actorWeight);
	linearZspreadforceX = GetPercentageValue(linearZspreadforceX_0, linearZspreadforceX_100, actorWeight);
	linearZspreadforceY = GetPercentageValue(linearZspreadforceY_0, linearZspreadforceY_100, actorWeight);
	linearXspreadforceYRot = GetPercentageValue(linearXspreadforceYRot_0, linearXspreadforceYRot_100, actorWeight);
	linearXspreadforceZRot = GetPercentageValue(linearXspreadforceZRot_0, linearXspreadforceZRot_100, actorWeight);
	linearYspreadforceXRot = GetPercentageValue(linearYspreadforceXRot_0, linearYspreadforceXRot_100, actorWeight);
	linearYspreadforceZRot = GetPercentageValue(linearYspreadforceZRot_0, linearYspreadforceZRot_100, actorWeight);
	linearZspreadforceXRot = GetPercentageValue(linearZspreadforceXRot_0, linearZspreadforceXRot_100, actorWeight);
	linearZspreadforceYRot = GetPercentageValue(linearZspreadforceYRot_0, linearZspreadforceYRot_100, actorWeight);

	forceMultipler = GetPercentageValue(forceMultipler_0, forceMultipler_100, actorWeight);

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
	collisionMultipler = GetPercentageValue(collisionMultipler_0, collisionMultipler_100, actorWeight);
	if (collisionMultipler >= 1.01f || collisionMultipler <= 0.99f)
		VirtualCollisionEnabled = true;
	collisionMultiplerRot = GetPercentageValue(collisionMultiplerRot_0, collisionMultiplerRot_100, actorWeight);

	float collisionReactionValue = GetPercentageValue(collisionElastic_0, collisionElastic_100, actorWeight);
	if (collisionReactionValue > 0.5f)
		collisionElastic = true;
	else
		collisionElastic = false;

	collisionXmaxOffset = GetPercentageValue(collisionXmaxOffset_0, collisionXmaxOffset_100, actorWeight);
	collisionXminOffset = GetPercentageValue(collisionXminOffset_0, collisionXminOffset_100, actorWeight);
	collisionYmaxOffset = GetPercentageValue(collisionYmaxOffset_0, collisionYmaxOffset_100, actorWeight);
	collisionYminOffset = GetPercentageValue(collisionYminOffset_0, collisionYminOffset_100, actorWeight);
	collisionZmaxOffset = GetPercentageValue(collisionZmaxOffset_0, collisionZmaxOffset_100, actorWeight);
	collisionZminOffset = GetPercentageValue(collisionZminOffset_0, collisionZminOffset_100, actorWeight);


	CollisionConfig.IsElasticCollision = collisionElastic;

	CollisionConfig.RotationalX = NiPoint3(linearXrotationX, linearYrotationX, linearZrotationX) * rotationalXnew;
	CollisionConfig.RotationalY = NiPoint3(linearXrotationY, linearYrotationY, linearZrotationY) * rotationalYnew;
	CollisionConfig.RotationalZ = NiPoint3(linearXrotationZ, linearYrotationZ, linearZrotationZ) * rotationalZnew;

	CollisionConfig.CollisionMaxOffset = NiPoint3(collisionXmaxOffset, collisionYmaxOffset, collisionZmaxOffset);
	CollisionConfig.CollisionMinOffset = NiPoint3(collisionXminOffset, collisionYminOffset, collisionZminOffset);

}

void Thing::updateConfig(Actor* actor, configEntry_t & centry, configEntry_t& centry0weight) {
	//100 weight

	stiffness_100 = centry["stiffness"];
	stiffnessX_100 = centry["stiffnessX"];
	stiffnessY_100 = centry["stiffnessY"];
	stiffnessZ_100 = centry["stiffnessZ"];
	if (stiffness_100 >= 0.001f && stiffnessX_100 < 0.001f && stiffnessY_100 < 0.001f && stiffnessZ_100 < 0.001f)
	{
		stiffnessX_100 = stiffness_100;
		stiffnessY_100 = stiffness_100;
		stiffnessZ_100 = stiffness_100;
	}
	stiffnessXRot_100 = centry["stiffnessXRot"];
	stiffnessYRot_100 = centry["stiffnessYRot"];
	stiffnessZRot_100 = centry["stiffnessZRot"];
	if (stiffness_100 >= 0.001f && stiffnessXRot_100 < 0.001f && stiffnessYRot_100 < 0.001f && stiffnessZRot_100 < 0.001f)
	{
		stiffnessXRot_100 = stiffness_100;
		stiffnessYRot_100 = stiffness_100;
		stiffnessZRot_100 = stiffness_100;
	}
	stiffness2_100 = centry["stiffness2"];
	stiffness2X_100 = centry["stiffness2X"];
	stiffness2Y_100 = centry["stiffness2Y"];
	stiffness2Z_100 = centry["stiffness2Z"];
	if (stiffness2_100 >= 0.001f && stiffness2X_100 < 0.001f && stiffness2Y_100 < 0.001f && stiffness2Z_100 < 0.001f)
	{
		stiffness2X_100 = stiffness2_100;
		stiffness2Y_100 = stiffness2_100;
		stiffness2Z_100 = stiffness2_100;
	}
	stiffness2XRot_100 = centry["stiffness2XRot"];
	stiffness2YRot_100 = centry["stiffness2YRot"];
	stiffness2ZRot_100 = centry["stiffness2ZRot"];
	if (stiffness2_100 >= 0.001f && stiffness2XRot_100 < 0.001f && stiffness2YRot_100 < 0.001f && stiffness2ZRot_100 < 0.001f)
	{
		stiffness2XRot_100 = stiffness2_100;
		stiffness2YRot_100 = stiffness2_100;
		stiffness2ZRot_100 = stiffness2_100;
	}
	damping_100 = centry["damping"];
	dampingX_100 = centry["dampingX"];
	dampingY_100 = centry["dampingY"];
	dampingZ_100 = centry["dampingZ"];
	if (damping_100 >= 0.001f && dampingX_100 < 0.001f && dampingY_100 < 0.001f && dampingZ_100 < 0.001f)
	{
		dampingX_100 = damping_100;
		dampingY_100 = damping_100;
		dampingZ_100 = damping_100;
	}
	dampingXRot_100 = centry["dampingXRot"];
	dampingYRot_100 = centry["dampingYRot"];
	dampingZRot_100 = centry["dampingZRot"];
	if (dampingXRot_100 < 0.001f && dampingYRot_100 < 0.001f && dampingZRot_100 < 0.001f)
	{
		dampingXRot_100 = dampingX_100;
		dampingYRot_100 = dampingY_100;
		dampingZRot_100 = dampingZ_100;
	}
	maxOffset_100 = centry["maxoffset"];
	if (maxOffset_100 >= 0.01f)
	{
		XmaxOffset_100 = maxOffset_100;
		XminOffset_100 = -maxOffset_100;
		YmaxOffset_100 = maxOffset_100;
		YminOffset_100 = -maxOffset_100;
		ZmaxOffset_100 = maxOffset_100;
		ZminOffset_100 = -maxOffset_100;
		XmaxOffsetRot_100 = maxOffset_100;
		XminOffsetRot_100 = -maxOffset_100;
		YmaxOffsetRot_100 = maxOffset_100;
		YminOffsetRot_100 = -maxOffset_100;
		ZmaxOffsetRot_100 = maxOffset_100;
		ZminOffsetRot_100 = -maxOffset_100;
	}
	else
	{
		XmaxOffset_100 = centry["Xmaxoffset"];
		XminOffset_100 = centry["Xminoffset"];
		YmaxOffset_100 = centry["Ymaxoffset"];
		YminOffset_100 = centry["Yminoffset"];
		ZmaxOffset_100 = centry["Zmaxoffset"];
		ZminOffset_100 = centry["Zminoffset"];

		XmaxOffsetRot_100 = centry["XmaxoffsetRot"];
		XminOffsetRot_100 = centry["XminoffsetRot"];
		YmaxOffsetRot_100 = centry["YmaxoffsetRot"];
		YminOffsetRot_100 = centry["YminoffsetRot"];
		ZmaxOffsetRot_100 = centry["ZmaxoffsetRot"];
		ZminOffsetRot_100 = centry["ZminoffsetRot"];

		if (XmaxOffsetRot_100 < 0.001f && XminOffsetRot_100 < 0.001f && YmaxOffsetRot_100 < 0.001f && YminOffsetRot_100 < 0.001f && ZmaxOffsetRot_100 < 0.001f && ZminOffsetRot_100 < 0.001f)
		{
			XmaxOffsetRot_100 = XmaxOffset_100;
			XminOffsetRot_100 = XminOffset_100;
			YmaxOffsetRot_100 = YmaxOffset_100;
			YminOffsetRot_100 = YminOffset_100;
			ZmaxOffsetRot_100 = ZmaxOffset_100;
			ZminOffsetRot_100 = ZminOffset_100;
		}
	}
	XdefaultOffset_100 = centry["Xdefaultoffset_100"];
	YdefaultOffset_100 = centry["Ydefaultoffset_100"];
	ZdefaultOffset_100 = centry["Zdefaultoffset_100"];

	timeTick_100 = centry["timetick"];
	if (timeTick_100 <= 1.0f)
		timeTick_100 = 1.0f;
	timeTickRot_100 = centry["timetickRot"];
	if (timeTickRot_100 < 1.0f)
		timeTickRot_100 = timeTick_100;

	linearX_100 = centry["linearX"];
	linearY_100 = centry["linearY"];
	linearZ_100 = centry["linearZ"];

	rotationalXnew_100 = centry["rotational"];
	if (rotationalXnew_100 < 0.0001f)
	{
		rotationalXnew_100 = centry["rotationalX"];
	}
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

	timeStep_100 = centry["timeStep"];
	timeStepRot_100 = centry["timeStepRot"];
	if (timeStepRot_100 < 0.001f)
		timeStepRot_100 = timeStep_100;

	linearXspreadforceY_100 = centry["linearXspreadforceY"];
	linearXspreadforceZ_100 = centry["linearXspreadforceZ"];
	linearYspreadforceX_100 = centry["linearYspreadforceX"];
	linearYspreadforceZ_100 = centry["linearYspreadforceZ"];
	linearZspreadforceX_100 = centry["linearZspreadforceX"];
	linearZspreadforceY_100 = centry["linearZspreadforceY"];

	linearXspreadforceYRot_100 = centry["linearXspreadforceYRot"];
	linearXspreadforceZRot_100 = centry["linearXspreadforceZRot"];
	linearYspreadforceXRot_100 = centry["linearYspreadforceXRot"];
	linearYspreadforceZRot_100 = centry["linearYspreadforceZRot"];
	linearZspreadforceXRot_100 = centry["linearZspreadforceXRot"];
	linearZspreadforceYRot_100 = centry["linearZspreadforceYRot"];

	forceMultipler_100 = centry["forceMultipler"];

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
	if (collisionFriction_100 < 0.0f)
		collisionFriction_100 = 0.0f;
	else if (collisionFriction_100 > 1.0f)
		collisionFriction_100 = 1.0f;

	collisionPenetration_100 = 1.0f - centry["collisionPenetration"];
	if (collisionPenetration_100 < 0.0f)
		collisionPenetration_100 = 0.0f;
	else if (collisionPenetration_100 > 1.0f)
		collisionPenetration_100 = 1.0f;

	collisionMultipler_100 = centry["collisionMultipler"];

	collisionMultiplerRot_100 = centry["collisionMultiplerRot"];
	collisionElastic_100 = centry["collisionElastic"];

	collisionXmaxOffset_100 = centry["collisionXmaxoffset"];
	collisionXminOffset_100 = centry["collisionXminoffset"];
	collisionYmaxOffset_100 = centry["collisionYmaxoffset"];
	collisionYminOffset_100 = centry["collisionYminoffset"];
	collisionZmaxOffset_100 = centry["collisionZmaxoffset"];
	collisionZminOffset_100 = centry["collisionZminoffset"];



	//0 weight
	stiffness_0 = centry0weight["stiffness"];
	stiffnessX_0 = centry0weight["stiffnessX"];
	stiffnessY_0 = centry0weight["stiffnessY"];
	stiffnessZ_0 = centry0weight["stiffnessZ"];
	if (stiffness_0 >= 0.001f && stiffnessX_0 < 0.001f && stiffnessY_0 < 0.001f && stiffnessZ_0 < 0.001f)
	{
		stiffnessX_0 = stiffness_0;
		stiffnessY_0 = stiffness_0;
		stiffnessZ_0 = stiffness_0;
	}
	stiffnessXRot_0 = centry0weight["stiffnessXRot"];
	stiffnessYRot_0 = centry0weight["stiffnessYRot"];
	stiffnessZRot_0 = centry0weight["stiffnessZRot"];
	if (stiffness_0 >= 0.001f && stiffnessXRot_0 < 0.001f && stiffnessYRot_0 < 0.001f && stiffnessZRot_0 < 0.001f)
	{
		stiffnessXRot_0 = stiffness_0;
		stiffnessYRot_0 = stiffness_0;
		stiffnessZRot_0 = stiffness_0;
	}
	stiffness2_0 = centry0weight["stiffness2"];
	stiffness2X_0 = centry0weight["stiffness2X"];
	stiffness2Y_0 = centry0weight["stiffness2Y"];
	stiffness2Z_0 = centry0weight["stiffness2Z"];
	if (stiffness2_0 >= 0.001f && stiffness2X_0 < 0.001f && stiffness2Y_0 < 0.001f && stiffness2Z_0 < 0.001f)
	{
		stiffness2X_0 = stiffness2_0;
		stiffness2Y_0 = stiffness2_0;
		stiffness2Z_0 = stiffness2_0;
	}
	stiffness2XRot_0 = centry0weight["stiffness2XRot"];
	stiffness2YRot_0 = centry0weight["stiffness2YRot"];
	stiffness2ZRot_0 = centry0weight["stiffness2ZRot"];
	if (stiffness2_0 >= 0.001f && stiffness2XRot_0 < 0.001f && stiffness2YRot_0 < 0.001f && stiffness2ZRot_0 < 0.001f)
	{
		stiffness2XRot_0 = stiffness2_0;
		stiffness2YRot_0 = stiffness2_0;
		stiffness2ZRot_0 = stiffness2_0;
	}
	damping_0 = centry0weight["damping"];
	dampingX_0 = centry0weight["dampingX"];
	dampingY_0 = centry0weight["dampingY"];
	dampingZ_0 = centry0weight["dampingZ"];
	if (damping_0 >= 0.001f && dampingX_0 < 0.001f && dampingY_0 < 0.001f && dampingZ_0 < 0.001f)
	{
		dampingX_0 = damping_0;
		dampingY_0 = damping_0;
		dampingZ_0 = damping_0;
	}
	dampingXRot_0 = centry0weight["dampingXRot"];
	dampingYRot_0 = centry0weight["dampingYRot"];
	dampingZRot_0 = centry0weight["dampingZRot"];
	if (damping_0 >= 0.001f && dampingXRot_0 < 0.001f && dampingYRot_0 < 0.001f && dampingZRot_0 < 0.001f)
	{
		dampingXRot_0 = damping_0;
		dampingYRot_0 = damping_0;
		dampingZRot_0 = damping_0;
	}
	maxOffset_0 = centry0weight["maxoffset"];
	if (maxOffset_0 >= 0.01f)
	{
		XmaxOffset_0 = maxOffset_0;
		XminOffset_0 = -maxOffset_0;
		YmaxOffset_0 = maxOffset_0;
		YminOffset_0 = -maxOffset_0;
		ZmaxOffset_0 = maxOffset_0;
		ZminOffset_0 = -maxOffset_0;
		XmaxOffsetRot_0 = maxOffset_0;
		XminOffsetRot_0 = -maxOffset_0;
		YmaxOffsetRot_0 = maxOffset_0;
		YminOffsetRot_0 = -maxOffset_0;
		ZmaxOffsetRot_0 = maxOffset_0;
		ZminOffsetRot_0 = -maxOffset_0;
	}
	else
	{
		XmaxOffset_0 = centry0weight["Xmaxoffset"];
		XminOffset_0 = centry0weight["Xminoffset"];
		YmaxOffset_0 = centry0weight["Ymaxoffset"];
		YminOffset_0 = centry0weight["Yminoffset"];
		ZmaxOffset_0 = centry0weight["Zmaxoffset"];
		ZminOffset_0 = centry0weight["Zminoffset"];

		XmaxOffsetRot_0 = centry0weight["XmaxoffsetRot"];
		XminOffsetRot_0 = centry0weight["XminoffsetRot"];
		YmaxOffsetRot_0 = centry0weight["YmaxoffsetRot"];
		YminOffsetRot_0 = centry0weight["YminoffsetRot"];
		ZmaxOffsetRot_0 = centry0weight["ZmaxoffsetRot"];
		ZminOffsetRot_0 = centry0weight["ZminoffsetRot"];

		if (XmaxOffsetRot_0 < 0.001f && XminOffsetRot_0 < 0.001f && YmaxOffsetRot_0 < 0.001f && YminOffsetRot_0 < 0.001f && ZmaxOffsetRot_0 < 0.001f && ZminOffsetRot_0 < 0.001f)
		{
			XmaxOffsetRot_0 = XmaxOffset_0;
			XminOffsetRot_0 = XminOffset_0;
			YmaxOffsetRot_0 = YmaxOffset_0;
			YminOffsetRot_0 = YminOffset_0;
			ZmaxOffsetRot_0 = ZmaxOffset_0;
			ZminOffsetRot_0 = ZminOffset_0;
		}
	}
	XdefaultOffset_0 = centry0weight["Xdefaultoffset_0"];
	YdefaultOffset_0 = centry0weight["Ydefaultoffset_0"];
	ZdefaultOffset_0 = centry0weight["Zdefaultoffset_0"];

	timeTick_0 = centry0weight["timetick"];
	if (timeTick_0 <= 1.0f)
		timeTick_0 = 1.0f;
	timeTickRot_0 = centry0weight["timetickRot"];
	if (timeTickRot_0 < 1.0f)
		timeTickRot_0 = timeTick_0;
	linearX_0 = centry0weight["linearX"];
	linearY_0 = centry0weight["linearY"];
	linearZ_0 = centry0weight["linearZ"];

	rotationalXnew_0 = centry0weight["rotational"];
	if (rotationalXnew_0 < 0.0001f)
	{
		rotationalXnew_0 = centry0weight["rotationalX"];
	}
	rotationalYnew_0 = centry0weight["rotationalY"];
	rotationalZnew_0 = centry0weight["rotationalZ"];

	linearXrotationX_0 = centry0weight["linearXrotationX"];
	linearXrotationY_0 = centry0weight["linearXrotationY"];
	linearXrotationZ_0 = centry0weight["linearXrotationZ"];
	linearYrotationX_0 = centry0weight["linearYrotationX"];
	linearYrotationY_0 = centry0weight["linearYrotationY"];
	linearYrotationZ_0 = centry0weight["linearYrotationZ"];
	linearZrotationX_0 = centry0weight["linearZrotationX"];
	linearZrotationY_0 = centry0weight["linearZrotationY"];
	linearZrotationZ_0 = centry0weight["linearZrotationZ"];

	timeStep_0 = centry0weight["timeStep"];
	timeStepRot_0 = centry0weight["timeStepRot"];
	if (timeStepRot_0 < 0.001f)
		timeStepRot_0 = timeStep_0;

	linearXspreadforceY_0 = centry0weight["linearXspreadforceY"];
	linearXspreadforceZ_0 = centry0weight["linearXspreadforceZ"];
	linearYspreadforceX_0 = centry0weight["linearYspreadforceX"];
	linearYspreadforceZ_0 = centry0weight["linearYspreadforceZ"];
	linearZspreadforceX_0 = centry0weight["linearZspreadforceX"];
	linearZspreadforceY_0 = centry0weight["linearZspreadforceY"];

	linearXspreadforceYRot_0 = centry0weight["linearXspreadforceYRot"];
	linearXspreadforceZRot_0 = centry0weight["linearXspreadforceZRot"];
	linearYspreadforceXRot_0 = centry0weight["linearYspreadforceXRot"];
	linearYspreadforceZRot_0 = centry0weight["linearYspreadforceZRot"];
	linearZspreadforceXRot_0 = centry0weight["linearZspreadforceXRot"];
	linearZspreadforceYRot_0 = centry0weight["linearZspreadforceYRot"];

	forceMultipler_0 = centry0weight["forceMultipler"];

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
	if (collisionFriction_0 < 0.0f)
		collisionFriction_0 = 0.0f;
	else if (collisionFriction_0 > 1.0f)
		collisionFriction_0 = 1.0f;

	collisionPenetration_0 = 1.0f - centry0weight["collisionPenetration"];
	if (collisionPenetration_0 < 0.0f)
		collisionPenetration_0 = 0.0f;
	else if (collisionPenetration_0 > 1.0f)
		collisionPenetration_0 = 1.0f;

	collisionMultipler_0 = centry0weight["collisionMultipler"];
	collisionMultiplerRot_0 = centry0weight["collisionMultiplerRot"];

	collisionElastic_0 = centry0weight["collisionElastic"];

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

void Thing::updatePelvis(Actor *actor, std::shared_mutex& thing_SetNode_lock, std::shared_mutex& thing_ReadNode_lock, std::shared_mutex& thing_Refresh_node_lock)
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

	thing_ReadNode_lock.lock();
	NiAVObject* leftPusObj = loadedState->node->GetObjectByName(&leftPus.data);
	NiAVObject* rightPusObj = loadedState->node->GetObjectByName(&rightPus.data);
	NiAVObject* backPusObj = loadedState->node->GetObjectByName(&backPus.data);
	NiAVObject* frontPusObj = loadedState->node->GetObjectByName(&frontPus.data);
	NiAVObject* pelvisObj = loadedState->node->GetObjectByName(&pelvis.data);
	thing_ReadNode_lock.unlock();

	if (!leftPusObj || !rightPusObj || !backPusObj || !frontPusObj || !pelvisObj)
	{
		return;
	}
	else
	{
		if(updatePussyFirstRun)
		{
			updatePussyFirstRun = false;
			
			auto leftpair = std::make_pair(actor->baseForm->formID, leftPus.data);
			thing_map_lock.lock();
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

			auto backpair = std::make_pair(actor->baseForm->formID, backPus.data);
			posMap = thingDefaultPosList.find(backpair);

			if (posMap == thingDefaultPosList.end())
			{
				//Add it to the list
				backPussyDefaultPos = backPusObj->m_localTransform.pos;
				thingDefaultPosList[backpair] = backPussyDefaultPos;
				LOG("Adding %s to default list for %08x -> %g %g %g", backPus.data, actor->baseForm->formID, backPussyDefaultPos.x, backPussyDefaultPos.y, backPussyDefaultPos.z);

			}
			else
			{
				backPussyDefaultPos = posMap->second;
			}

			auto frontpair = std::make_pair(actor->baseForm->formID, frontPus.data);
			posMap = thingDefaultPosList.find(frontpair);

			if (posMap == thingDefaultPosList.end())
			{
				//Add it to the list
				frontPussyDefaultPos = frontPusObj->m_localTransform.pos;
				thingDefaultPosList[frontpair] = frontPussyDefaultPos;
				LOG("Adding %s to default list for %08x -> %g %g %g", frontPus.data, actor->baseForm->formID, frontPussyDefaultPos.x, frontPussyDefaultPos.y, frontPussyDefaultPos.z);

			}
			else
			{
				frontPussyDefaultPos = posMap->second;
			}
			thing_map_lock.unlock();
			LOG_INFO("Left pussy default pos -> %g %g %g , Right pussy default pos ->  %g %g %g , Back pussy default pos ->  %g %g %g , Front pussy default pos ->  %g %g %g", leftPussyDefaultPos.x, leftPussyDefaultPos.y, leftPussyDefaultPos.z, rightPussyDefaultPos.x, rightPussyDefaultPos.y, rightPussyDefaultPos.z, backPussyDefaultPos.x, backPussyDefaultPos.y, backPussyDefaultPos.z, frontPussyDefaultPos.x, frontPussyDefaultPos.y, frontPussyDefaultPos.z);
		
		//	CollisionSetUp.IsTriggerCollision = true;
		}
		
		//There's nothing problem with editing, but if editing once then all node world positions are updated.
		//so it seems that a high probability of overloading if it is processed by parallel processing.
		thing_SetNode_lock.lock();
		leftPusObj->m_localTransform.pos = leftPussyDefaultPos;
		rightPusObj->m_localTransform.pos = rightPussyDefaultPos;
		backPusObj->m_localTransform.pos = backPussyDefaultPos;
		frontPusObj->m_localTransform.pos = frontPussyDefaultPos;
		thing_SetNode_lock.unlock();
	}

	if (!ActorCollisionsEnabled)
	{
		return;
	}

	// Collision Stuff Start
	NiPoint3 collisionVector = emptyPoint;

	NiMatrix33 pelvisRotation;
	NiPoint3 pelvisPosition;
	float pelvisScale;

	pelvisRotation = pelvisObj->m_worldTransform.rot;
	pelvisPosition = pelvisObj->m_worldTransform.pos;
	pelvisScale = pelvisObj->m_worldTransform.scale;

	std::vector<int> thingIdList;
	std::vector<int> hashIdList;
	NiPoint3 playerPos = (*g_thePlayer)->loadedState->node->m_worldTransform.pos;
	for (int i = 0; i < thingCollisionSpheres.size(); i++)
	{
		thingCollisionSpheres[i].offset100 = thingCollisionSpheres[i].offset0 * pelvisScale;
		thingCollisionSpheres[i].worldPos = pelvisPosition + (pelvisRotation * thingCollisionSpheres[i].offset100);
		thingCollisionSpheres[i].radius100 = thingCollisionSpheres[i].radius0 * pelvisScale;
		thingCollisionSpheres[i].radius100pwr2 = thingCollisionSpheres[i].radius100 * thingCollisionSpheres[i].radius100;
		hashIdList = GetHashIdsFromPos(thingCollisionSpheres[i].worldPos - playerPos, thingCollisionSpheres[i].radius100);
		for (int m = 0; m<hashIdList.size(); m++)
		{
			if (!(std::find(thingIdList.begin(), thingIdList.end(), hashIdList[m]) != thingIdList.end()))
			{
				thingIdList.emplace_back(hashIdList[m]);
			}
		}
	}	
	for (int i = 0; i < thingCollisionCapsules.size(); i++)
	{
		thingCollisionCapsules[i].End1_offset100 = thingCollisionCapsules[i].End1_offset0 * pelvisScale;
		thingCollisionCapsules[i].End1_worldPos = pelvisPosition + (pelvisRotation* thingCollisionCapsules[i].End1_offset100);
		thingCollisionCapsules[i].End1_radius100 = thingCollisionCapsules[i].End1_radius0 * pelvisScale;
		thingCollisionCapsules[i].End1_radius100pwr2 = thingCollisionCapsules[i].End1_radius100 * thingCollisionCapsules[i].End1_radius100;
		thingCollisionCapsules[i].End2_offset100 = thingCollisionCapsules[i].End2_offset0 * pelvisScale;
		thingCollisionCapsules[i].End2_worldPos = pelvisPosition + (pelvisRotation* thingCollisionCapsules[i].End2_offset100);
		thingCollisionCapsules[i].End2_radius100 = thingCollisionCapsules[i].End2_radius0 * pelvisScale;
		thingCollisionCapsules[i].End2_radius100pwr2 = thingCollisionCapsules[i].End2_radius100 * thingCollisionCapsules[i].End2_radius100;
		hashIdList = GetHashIdsFromPos((thingCollisionCapsules[i].End1_worldPos + thingCollisionCapsules[i].End2_worldPos) * 0.5f - playerPos
			, (thingCollisionCapsules[i].End1_radius100 + thingCollisionCapsules[i].End2_radius100) * 0.5f);
		for (int m = 0; m<hashIdList.size(); m++)
		{
			if (!(std::find(thingIdList.begin(), thingIdList.end(), hashIdList[m]) != thingIdList.end()))
			{
				thingIdList.emplace_back(hashIdList[m]);
			}
		}
	}	


	NiPoint3 collisionDiff = emptyPoint;

	CollisionConfig.maybePos = pelvisPosition;
	CollisionConfig.cnvRot = pelvisObj->m_parent->m_worldTransform.rot;
	CollisionConfig.objRot = pelvisRotation;
	CollisionConfig.invRot = pelvisObj->m_parent->m_worldTransform.rot.Transpose();
		
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

				//now not that do reach max value just by get closer and just affected by the collider size
				partitions[id].partitionCollisions[i].CheckPelvisCollision(collisionDiff, thingCollisionSpheres, thingCollisionCapsules, CollisionConfig);
			}
		}
	}

	// Collision Stuff End
	
	NiPoint3 leftVector = collisionDiff;
	NiPoint3 rightVector = collisionDiff;
	NiPoint3 backVector = collisionDiff;
	NiPoint3 frontVector = collisionDiff;

	float opening = distance(collisionDiff, emptyPoint);

	CalculateDiffVagina(leftVector, opening, true, true);
	CalculateDiffVagina(rightVector, opening, true, false);
	CalculateDiffVagina(backVector, opening, false, true);
	CalculateDiffVagina(frontVector, opening, false, false);

	NormalizeNiPoint(leftVector, thing_vaginaOpeningLimit*-1.0f, thing_vaginaOpeningLimit);
	NormalizeNiPoint(rightVector, thing_vaginaOpeningLimit*-1.0f, thing_vaginaOpeningLimit);
	backVector.y = clamp(backVector.y, thing_vaginaOpeningLimit*-0.5f, thing_vaginaOpeningLimit*0.5f);
	backVector.z = clamp(backVector.z, thing_vaginaOpeningLimit*-0.165f, thing_vaginaOpeningLimit*0.165f);
	frontVector.y = clamp(frontVector.y, thing_vaginaOpeningLimit*-0.125f, thing_vaginaOpeningLimit*0.125f);
	frontVector.z = clamp(frontVector.z, thing_vaginaOpeningLimit*-0.25f, thing_vaginaOpeningLimit*0.25f);
	
	thing_SetNode_lock.lock();
	leftPusObj->m_localTransform.pos = leftPussyDefaultPos + leftVector;
	rightPusObj->m_localTransform.pos = rightPussyDefaultPos + rightVector;
	backPusObj->m_localTransform.pos = backPussyDefaultPos + backVector;
	frontPusObj->m_localTransform.pos = frontPussyDefaultPos + frontVector;
	thing_SetNode_lock.unlock();

	RefreshNode(leftPusObj, thing_Refresh_node_lock);
	RefreshNode(rightPusObj, thing_Refresh_node_lock);
	RefreshNode(backPusObj, thing_Refresh_node_lock);
	RefreshNode(frontPusObj, thing_Refresh_node_lock);
	/*QueryPerformanceCounter(&endingTime);
	elapsedMicroseconds.QuadPart = endingTime.QuadPart - startingTime.QuadPart;
	elapsedMicroseconds.QuadPart *= 1000000000LL;
	elapsedMicroseconds.QuadPart /= frequency.QuadPart;
	LOG("Thing.updatePelvis() Update Time = %lld ns\n", elapsedMicroseconds.QuadPart);*/
}

bool Thing::ApplyBellyBulge(Actor * actor, std::shared_mutex& thing_SetNode_lock, std::shared_mutex& thing_ReadNode_lock)
{
	if (!(*g_thePlayer) || !(*g_thePlayer)->loadedState || !(*g_thePlayer)->loadedState->node)
	{
		return false;
	}

	NiPoint3 collisionVector = emptyPoint;
	
	NiMatrix33 bulgenodeRotation;
	NiPoint3 bulgenodePosition;
	float bulgenodeScale;

	thing_ReadNode_lock.lock();
	NiAVObject* bellyObj = actor->loadedState->node->GetObjectByName(&belly.data);
	NiAVObject* bulgeObj = actor->loadedState->node->GetObjectByName(&spine1.data);
	thing_ReadNode_lock.unlock();

	if (!bellyObj || !bulgeObj)
		return false;

	if(updateBellyFirstRun)
	{
		updateBellyFirstRun = false;

		auto mypair = std::make_pair(actor->baseForm->formID, belly.data);
		thing_map_lock.lock();
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
		thing_map_lock.unlock();
		LOG_INFO("Belly default pos -> %g %g %g", bellyDefaultPos.x, bellyDefaultPos.y, bellyDefaultPos.z);

	//	CollisionSetUp.IsTriggerCollision = true;
	}

	bulgenodeRotation = bulgeObj->m_worldTransform.rot;
	bulgenodePosition = bulgeObj->m_worldTransform.pos;
	bulgenodeScale = bulgeObj->m_worldTransform.scale;

	std::vector<int> thingIdList;
	std::vector<int> hashIdList;

	NiPoint3 playerPos = (*g_thePlayer)->loadedState->node->m_worldTransform.pos;

	for (int i = 0; i < thingCollisionSpheres.size(); i++)
	{
		thingCollisionSpheres[i].offset100 = thingCollisionSpheres[i].offset0 * bulgenodeScale;
		thingCollisionSpheres[i].worldPos = bulgenodePosition + (bulgenodeRotation * thingCollisionSpheres[i].offset100);
		thingCollisionSpheres[i].radius100 = thingCollisionSpheres[i].radius0 * bulgenodeScale;
		thingCollisionSpheres[i].radius100pwr2 = thingCollisionSpheres[i].radius100 * thingCollisionSpheres[i].radius100;
		hashIdList = GetHashIdsFromPos(thingCollisionSpheres[i].worldPos - playerPos, thingCollisionSpheres[i].radius100);
		for (int m = 0; m<hashIdList.size(); m++)
		{
			if (!(std::find(thingIdList.begin(), thingIdList.end(), hashIdList[m]) != thingIdList.end()))
			{
				thingIdList.emplace_back(hashIdList[m]);
			}
		}
	}
	for (int i = 0; i < thingCollisionCapsules.size(); i++)
	{
		thingCollisionCapsules[i].End1_offset100 = thingCollisionCapsules[i].End1_offset0 * bulgenodeScale;
		thingCollisionCapsules[i].End1_worldPos = bulgenodePosition + (bulgenodeRotation * thingCollisionCapsules[i].End1_offset100);
		thingCollisionCapsules[i].End1_radius100 = thingCollisionCapsules[i].End1_radius0 * bulgenodeScale;
		thingCollisionCapsules[i].End1_radius100pwr2 = thingCollisionCapsules[i].End1_radius100 * thingCollisionCapsules[i].End1_radius100;
		thingCollisionCapsules[i].End2_offset100 = thingCollisionCapsules[i].End2_offset0 * bulgenodeScale;
		thingCollisionCapsules[i].End2_worldPos = bulgenodePosition + (bulgenodeRotation * thingCollisionCapsules[i].End2_offset100);
		thingCollisionCapsules[i].End2_radius100 = thingCollisionCapsules[i].End2_radius0 * bulgenodeScale;
		thingCollisionCapsules[i].End2_radius100pwr2 = thingCollisionCapsules[i].End2_radius100 * thingCollisionCapsules[i].End2_radius100;
		hashIdList = GetHashIdsFromPos((thingCollisionCapsules[i].End1_worldPos + thingCollisionCapsules[i].End2_worldPos) * 0.5f - playerPos
			, (thingCollisionCapsules[i].End1_radius100 + thingCollisionCapsules[i].End2_radius100) * 0.5f);
		for (int m = 0; m<hashIdList.size(); m++)
		{
			if (!(std::find(thingIdList.begin(), thingIdList.end(), hashIdList[m]) != thingIdList.end()))
			{
				thingIdList.emplace_back(hashIdList[m]);
			}
		}
	}

	NiPoint3 collisionDiff = emptyPoint;

	CollisionConfig.maybePos = bulgenodePosition;
	CollisionConfig.cnvRot = bulgeObj->m_parent->m_worldTransform.rot;
	CollisionConfig.objRot = bulgenodeRotation;
	CollisionConfig.invRot = bulgeObj->m_parent->m_worldTransform.rot.Transpose();

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
					bool isColliding = false;

	//				if (partitions[id].partitionCollisions[i].colliderNodeName.find(thing_bellybulgelist[m]) != std::string::npos)
	//				{
						InterlockedIncrement(&callCount);

						partitions[id].partitionCollisions[i].CollidedWeight = actorWeight;

						//now not that do reach max value just by get closer and just affected by the collider size
						isColliding = partitions[id].partitionCollisions[i].CheckPelvisCollision(collisionDiff, thingCollisionSpheres, thingCollisionCapsules, CollisionConfig);

						if (isColliding)
						{
							genitalPenetration = true;
						}
	//				}

				}
			}
		}
	}

	const float opening = distance(collisionDiff, emptyPoint) * 2.0f;

	if (opening > 0)
	{
		if (thing_bellybulgemultiplier > 0 && genitalPenetration)
		{
			LOG("Belly bulge %f, %f, %f", collisionDiff.x, collisionDiff.y, collisionDiff.z);

			//LOG("opening:%g", opening);
	//		bellyBulgeCountDown = 1000;
			
			float horPos = opening * thing_bellybulgemultiplier;
			horPos = clamp(horPos, 0.0f, thing_bellybulgemax);
			float lowPos = (thing_bellybulgeposlowest / thing_bellybulgemax) * horPos;


			if (lastMaxOffsetY < horPos)
			{
				lastMaxOffsetY = abs(horPos);
				lastMaxOffsetZ = abs(lowPos);
			}

			thing_SetNode_lock.lock();
			bellyObj->m_localTransform.pos.y = bellyDefaultPos.y + horPos;
			bellyObj->m_localTransform.pos.z = bellyDefaultPos.z + lowPos;
			thing_SetNode_lock.unlock();

			//float vertPos = opening * bellybulgeposmultiplier;
			//vertPos = clamp(vertPos, bellybulgeposlowest, 0.0f);
			LOG("belly bulge vert:%g horiz:%g", lowPos, horPos);
			return true;
		}
	}
	return false;
}

void Thing::update(Actor* actor, std::shared_mutex& thing_SetNode_lock, std::shared_mutex& thing_ReadNode_lock, std::shared_mutex& thing_Refresh_node_lock) {

	bool collisionsOn = true;

	float Friction = 1.0f;
	if (collisionOnLastFrame)
	{
		Friction = collisionFriction;
	}

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

	float fpsCorrection = 1.0f;

	if (fpsCorrectionEnabled)
	{
		if (deltaT > fps60Tick * 4) deltaT = fps60Tick * 4; //edited
		if (deltaT < fps60Tick * 0.25) deltaT = fps60Tick * 0.25; //edited

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

	NiAVObject* obj;
	auto loadedState = actor->loadedState;

#ifdef RUNTIME_VR_VERSION_1_4_15
	if ((*g_thePlayer) && actor == (*g_thePlayer)) //To check if we can support VR Body
	{
		NiNode* rootNodeTP = (*g_thePlayer)->GetNiRootNode(0);

		NiNode* rootNodeFP = (*g_thePlayer)->GetNiRootNode(2);

		NiNode* mostInterestingRoot = (rootNodeFP != nullptr) ? rootNodeFP : rootNodeTP;

		thing_ReadNode_lock.lock();
		obj = ni_cast(mostInterestingRoot->GetObjectByName(&boneName.data), NiNode);
		thing_ReadNode_lock.unlock();

		//	objRotation = mostInterestingRoot->GetAsNiNode()->m_worldTransform.rot;
	}
	else
	{
#endif
		if (!loadedState || !loadedState->node) {
			LOG("No loaded state for actor %08x\n", actor->formID);
			return;
		}
		thing_ReadNode_lock.lock();
		obj = loadedState->node->GetObjectByName(&boneName.data);
		thing_ReadNode_lock.unlock();

#ifdef RUNTIME_VR_VERSION_1_4_15	
	}
#endif
	if (!obj)
	{
		return;
	}

	if (IsBellyBone && ActorCollisionsEnabled && thing_bellybulgemultiplier > 0)
	{
		if (ApplyBellyBulge(actor, thing_SetNode_lock, thing_ReadNode_lock))
		{
			RefreshNode(obj, thing_Refresh_node_lock);
			return;
		}
	}

	if (!obj->m_parent)
		return;

	nodeScale = obj->m_worldTransform.scale;

	bool IsThereCollision = false;
	bool maybeNot = false;
	NiPoint3 collisionDiff = emptyPoint;
	long originalDeltaT = deltaT;
	long deltaTRot = deltaT;
	NiPoint3 collisionVector = emptyPoint;

	float varGravityBias = gravityBias;

	float varLinearX = linearX;
	float varLinearY = linearY;
	float varLinearZ = linearZ;
	float varRotationalXnew = rotationalXnew;
	float varRotationalYnew = rotationalYnew;
	float varRotationalZnew = rotationalZnew;

	NiPoint3 playerPos = (*g_thePlayer)->loadedState->node->m_worldTransform * NiPoint3(0, cogOffset, 0);

	int collisionCheckCount = 0;

	NiPoint3 newPos = oldWorldPos;
	NiPoint3 newPosRot = oldWorldPosRot;

	NiPoint3 posDelta = emptyPoint;
	NiPoint3 posDeltaRot = emptyPoint;

	NiPoint3 target = obj->m_parent->m_worldTransform.pos;

	if (IsBreastBone) //other bones don't need to edited gravity by NPC Spine2 [Spn2] node
	{
		//Get the reference bone to know which way the breasts are orientated
		thing_ReadNode_lock.lock();
		NiAVObject* breastGravityReferenceBone = loadedState->node->GetObjectByName(&breastGravityReferenceBoneString.data);
		thing_ReadNode_lock.unlock();

		//Code sent by KheiraDjet(modified)
		if (breastGravityReferenceBone != nullptr)
		{
			//Get the orientation (here the Z element of the rotation matrix (1.0 when standing up, -1.0 when upside down))			
			float gravityRatio = (breastGravityReferenceBone->m_worldTransform.rot.data[2][2] + 1.0f) * 0.5f;

			//Remap the value from 0.0 => 1.0 to user defined values and clamps it
			gravityRatio = remap(gravityRatio, gravityInvertedCorrectionStart, gravityInvertedCorrectionEnd, 0.0, 1.0);
			gravityRatio = clamp(gravityRatio, 0.0f, 1.0f);

			//Calculate the resulting gravity
			varGravityCorrection = (gravityRatio * gravityCorrection) + ((1.0 - gravityRatio) * gravityInvertedCorrection);

			//Determine which armor the actor is wearing
			if (skipArmorCheck <= 0) //This is a little heavy, check only on equip/unequip events
			{
				forceAmplitude = 1.0f;

//				thing_armorKeyword_lock.lock();
				TESForm* wornForm = papyrusActor::GetWornForm(actor, 0x00000004);

				if (wornForm != nullptr)
				{
					TESObjectARMO* armor = DYNAMIC_CAST(wornForm, TESForm, TESObjectARMO);
					if (armor != nullptr)
					{
						bool IsAsNaked = false;
						isHeavyArmor = false;
						isLightArmor = false;
						isClothed = false;
						isNoPushUp = false;
						auto keywords = armor->keyword.keywords;
						if (keywords)
						{
						if (IsLeftBreastBone)
						{
								for (UInt32 index = 0; index < armor->keyword.numKeywords; index++)
							{
									if (!keywords[index])
										continue;
									if (strcmp(keywords[index]->keyword.Get(), KeywordNameAsNakedL.data) == 0)
							{
										IsAsNaked = true;
								}
									else if (strcmp(keywords[index]->keyword.Get(), KeywordNameAsHeavyL.data) == 0)
									{
										isHeavyArmor = true;
									}
									else if (strcmp(keywords[index]->keyword.Get(), KeywordNameAsLightL.data) == 0)
										{
										isLightArmor = true;
									}
									else if (strcmp(keywords[index]->keyword.Get(), KeywordNameAsClothingL.data) == 0)
							{
										isClothed = true;
									}
									else if (strcmp(keywords[index]->keyword.Get(), KeywordNameNoPushUpL.data) == 0)
								{
										isNoPushUp = true;
								}

							}
						}
						else if (IsRightBreastBone)
						{
								for (UInt32 index = 0; index < armor->keyword.numKeywords; index++)
							{
									if (!keywords[index])
										continue;
									if (strcmp(keywords[index]->keyword.Get(), KeywordNameAsNakedR.data) == 0)
							{
										IsAsNaked = true;
								}
									else if (strcmp(keywords[index]->keyword.Get(), KeywordNameAsHeavyR.data) == 0)
							{
										isHeavyArmor = true;
									}
									else if (strcmp(keywords[index]->keyword.Get(), KeywordNameAsLightR.data) == 0)
									{
										isLightArmor = true;
									}
									else if (strcmp(keywords[index]->keyword.Get(), KeywordNameAsClothingR.data) == 0)
								{
										isClothed = true;
									}
									else if (strcmp(keywords[index]->keyword.Get(), KeywordNameNoPushUpR.data) == 0)
										{
										isNoPushUp = true;
										}
									}
								}
						}

						if (IsAsNaked)
							{
							isHeavyArmor = false;
							isLightArmor = false;
							isClothed = false;
										}
						else if (!isHeavyArmor && !isLightArmor && !isClothed)
								{
								isHeavyArmor = (armor->keyword.HasKeyword(KeywordArmorHeavy));
								isLightArmor = (armor->keyword.HasKeyword(KeywordArmorLight));
								isClothed = (armor->keyword.HasKeyword(KeywordArmorClothing));

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
				skipArmorCheck--;

//				thing_armorKeyword_lock.unlock();
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
	else //other nodes are based on parent node
	{
		//Get the orientation (here the Z element of the rotation matrix (1.0 when standing up, -1.0 when upside down))			
		float gravityRatio = (obj->m_parent->m_worldTransform.rot.data[2][2] + 1.0f) * 0.5f;

		//Remap the value from 0.0 => 1.0 to user defined values and clamps it
		gravityRatio = remap(gravityRatio, gravityInvertedCorrectionStart, gravityInvertedCorrectionEnd, 0.0, 1.0);
		gravityRatio = clamp(gravityRatio, 0.0f, 1.0f);

		//Calculate the resulting gravity
		varGravityCorrection = (gravityRatio * gravityCorrection) + ((1.0 - gravityRatio) * gravityInvertedCorrection);
	}

	//Offset to move Center of Mass make rotaional motion more significant  
	NiPoint3 diff = (target - oldWorldPos) * forceMultipler;
	NiPoint3 diffRot = (target - oldWorldPosRot) * forceMultipler;

	//It is not recommended to use, When used, there is a high possibility that the movement will be adversely affected due to min/maxoffset
	//diff += NiPoint3(0, 0, varGravityCorrection) * (fpsCorrectionEnabled ? fpsCorrection : 1.0f);

	if (fabs(diff.x) > 100 || fabs(diff.y) > 100 || fabs(diff.z) > 100) //prevent shakes
	{
		//logger.error("transform reset\n");
		thing_SetNode_lock.lock();
		obj->m_localTransform.pos = thingDefaultPos;
		obj->m_localTransform.rot = thingDefaultRot;
		thing_SetNode_lock.unlock();

		oldWorldPos = target;
		oldWorldPosRot = target;
		velocity = emptyPoint;
		velocityRot = emptyPoint;
		time = clock();
		return;
	}

	// move the bones based on the supplied weightings
	// Convert the world translations into local coordinates
	auto invRot = obj->m_parent->m_worldTransform.rot.Transpose();

	NiPoint3 forceGravityBias = invRot * (NiPoint3(0, 0, varGravityBias) / fpsCorrection);
	
	NiPoint3 ldiff = emptyPoint;
	NiPoint3 ldiffRot = emptyPoint;
	NiMatrix33 newRot;

	concurrency::parallel_invoke(
		[&] { //linear calculation
		// Compute the "Spring" Force
		float timeMultiplier = timeTick / (float)deltaT;
		diff = invRot * (diff * timeMultiplier);

		NiPoint3 stiffnessXYZ = NiPoint3(stiffnessX, stiffnessY, stiffnessZ);
		NiPoint3 stiffness2XYZ = NiPoint3(stiffness2X, stiffness2Y, stiffness2Z);
		NiPoint3 dampingXYZ = NiPoint3(dampingX, dampingY, dampingZ);

		NiPoint3 diff2(diff.x* diff.x* sgn(diff.x), diff.y* diff.y* sgn(diff.y), diff.z* diff.z* sgn(diff.z));
		NiPoint3 force = (NiPoint3((diff.x * stiffnessXYZ.x) + (diff2.x * stiffness2XYZ.x)
			, (diff.y * stiffnessXYZ.y) + (diff2.y * stiffness2XYZ.y)
			, (diff.z * stiffnessXYZ.z) + (diff2.z * stiffness2XYZ.z))
			- forceGravityBias);

		//showPos(diff);
		//showPos(force);

		velocity = invRot * velocity;

		do {
			// Assume mass is 1, so Accelleration is Force, can vary mass by changinf force
			//velocity = (velocity + (force * timeStep)) * (1 - (damping * timeStep));

			velocity = velocity * Friction; //Fixes that when becomes unstable collisions during colliding at low or unstable FPS

			velocity.x = (velocity.x + (force.x * timeStep)) - (velocity.x * (dampingXYZ.x * timeStep));
			velocity.y = (velocity.y + (force.y * timeStep)) - (velocity.y * (dampingXYZ.y * timeStep));
			velocity.z = (velocity.z + (force.z * timeStep)) - (velocity.z * (dampingXYZ.z * timeStep));

			posDelta += velocity * timeStep;

			deltaT -= timeTick;
		} while (deltaT >= timeTick);

		velocity = obj->m_parent->m_worldTransform.rot * velocity;

		newPos = newPos + obj->m_parent->m_worldTransform.rot * (posDelta * fpsCorrection);

		// clamp the difference to stop the breast severely lagging at low framerates
		NiPoint3 newdiff = newPos - target;

		varLinearX = varLinearX * forceAmplitude;
		varLinearY = varLinearY * forceAmplitude;
		varLinearZ = varLinearZ * forceAmplitude;

		ldiff = invRot * newdiff;
		auto beforeldiff = ldiff;

		ldiff.x = clamp(ldiff.x, XminOffset, XmaxOffset);
		ldiff.y = clamp(ldiff.y, YminOffset, YmaxOffset);
		ldiff.z = clamp(ldiff.z, ZminOffset, ZmaxOffset);

		//It can allows the force of dissipated by min/maxoffsets to be spread in different directions
		beforeldiff = beforeldiff - ldiff;

		ldiff.x = ldiff.x + ((beforeldiff.y * linearXspreadforceY) + (beforeldiff.z * linearXspreadforceZ));
		ldiff.y = ldiff.y + ((beforeldiff.x * linearYspreadforceX) + (beforeldiff.z * linearYspreadforceZ));
		ldiff.z = ldiff.z + ((beforeldiff.x * linearZspreadforceX) + (beforeldiff.y * linearZspreadforceY));

		ldiff.x = clamp(ldiff.x, XminOffset, XmaxOffset);
		ldiff.y = clamp(ldiff.y, YminOffset, YmaxOffset);
		ldiff.z = clamp(ldiff.z, ZminOffset, ZmaxOffset);

		//same the clamp(diff.z - varGravityCorrection, -maxOffset, maxOffset) + varGravityCorrection
		//this is the reason for the endless shaking when unstable fps in v1.4.1x
		ldiff = ldiff + (invRot * NiPoint3(0, 0, varGravityCorrection));
	},
		[&] { // rotation calculation

		float timeMultiplierRot = timeTickRot / (float)deltaTRot;
		diffRot = invRot * (diffRot * timeMultiplierRot);

		NiPoint3 stiffnessXYZRot = NiPoint3(stiffnessXRot, stiffnessYRot, stiffnessZRot);
		NiPoint3 stiffness2XYZRot = NiPoint3(stiffness2XRot, stiffness2YRot, stiffness2ZRot);
		NiPoint3 dampingXYZRot = NiPoint3(dampingXRot, dampingYRot, dampingZRot);

		NiPoint3 diff2Rot(diffRot.x * diffRot.x * sgn(diffRot.x), diffRot.y * diffRot.y * sgn(diffRot.y), diffRot.z * diffRot.z * sgn(diffRot.z));
		NiPoint3 forceRot = (NiPoint3((diffRot.x * stiffnessXYZRot.x) + (diff2Rot.x * stiffness2XYZRot.x)
			, (diffRot.y * stiffnessXYZRot.y) + (diff2Rot.y * stiffness2XYZRot.y)
			, (diffRot.z * stiffnessXYZRot.z) + (diff2Rot.z * stiffness2XYZRot.z))
			- forceGravityBias);

		velocityRot = invRot * velocityRot;

		do {
			// Assume mass is 1, so Accelleration is Force, can vary mass by changinf force
			//velocity = (velocity + (force * timeStep)) * (1 - (damping * timeStep));

			velocityRot = velocityRot * Friction; //Fixes that when becomes unstable collisions during colliding at low or unstable FPS

			velocityRot.x = (velocityRot.x + (forceRot.x * timeStepRot)) - (velocityRot.x * (dampingXYZRot.x * timeStepRot));
			velocityRot.y = (velocityRot.y + (forceRot.y * timeStepRot)) - (velocityRot.y * (dampingXYZRot.y * timeStepRot));
			velocityRot.z = (velocityRot.z + (forceRot.z * timeStepRot)) - (velocityRot.z * (dampingXYZRot.z * timeStepRot));

			posDeltaRot += velocityRot * timeStepRot;

			deltaTRot -= timeTickRot;
		} while (deltaTRot >= timeTickRot);

		velocityRot = obj->m_parent->m_worldTransform.rot * velocityRot;

		newPosRot = newPosRot + obj->m_parent->m_worldTransform.rot * (posDeltaRot * fpsCorrection);

		NiPoint3 newdiffRot = newPosRot - target;
		
		varRotationalXnew = varRotationalXnew * forceAmplitude;
		varRotationalYnew = varRotationalYnew * forceAmplitude;
		varRotationalZnew = varRotationalZnew * forceAmplitude;

		ldiffRot = invRot * newdiffRot;
		auto beforeldiffRot = ldiffRot;

		ldiffRot.x = clamp(ldiffRot.x, XminOffsetRot, XmaxOffsetRot);
		ldiffRot.y = clamp(ldiffRot.y, YminOffsetRot, YmaxOffsetRot);
		ldiffRot.z = clamp(ldiffRot.z, ZminOffsetRot, ZmaxOffsetRot);

		beforeldiffRot = beforeldiffRot - ldiffRot;

		ldiffRot.x = ldiffRot.x + ((beforeldiffRot.y * linearXspreadforceYRot) + (beforeldiffRot.z * linearXspreadforceZRot));
		ldiffRot.y = ldiffRot.y + ((beforeldiffRot.x * linearYspreadforceXRot) + (beforeldiffRot.z * linearYspreadforceZRot));
		ldiffRot.z = ldiffRot.z + ((beforeldiffRot.x * linearZspreadforceXRot) + (beforeldiffRot.y * linearZspreadforceYRot));

		ldiffRot.x = clamp(ldiffRot.x, XminOffsetRot, XmaxOffsetRot);
		ldiffRot.y = clamp(ldiffRot.y, YminOffsetRot, YmaxOffsetRot);
		ldiffRot.z = clamp(ldiffRot.z, ZminOffsetRot, ZmaxOffsetRot);

		ldiffRot = ldiffRot + (invRot * NiPoint3(0, 0, varGravityCorrection));

		auto rdiffXnew = ldiffRot * varRotationalXnew;
		auto rdiffYnew = ldiffRot * varRotationalYnew;
		auto rdiffZnew = ldiffRot * varRotationalZnew;

		rdiffXnew.x *= linearXrotationX;
		rdiffXnew.y *= linearYrotationX;
		rdiffXnew.z *= linearZrotationX;

		rdiffYnew.x *= linearXrotationY;
		rdiffYnew.y *= linearYrotationY;
		rdiffYnew.z *= linearZrotationY;

		rdiffZnew.x *= linearXrotationZ;
		rdiffZnew.y *= linearYrotationZ;
		rdiffZnew.z *= linearZrotationZ;

		newRot.SetEulerAngles(rdiffYnew.x + rdiffYnew.y + rdiffYnew.z, rdiffZnew.x + rdiffZnew.y + rdiffZnew.z, rdiffXnew.x + rdiffXnew.y + rdiffXnew.z);
	});

	///#### physics calculate done
	///#### collision calculate start

	NiPoint3 ldiffcol = emptyPoint;
	NiPoint3 ldiffGcol = emptyPoint;
	NiPoint3 maybeIdiffcol = emptyPoint;

	if (collisionsOn && ActorCollisionsEnabled)
	{
		std::vector<int> thingIdList;
		std::vector<int> hashIdList;

		NiPoint3 GroundCollisionVector = emptyPoint;

		//The rotation of the previous frame due to collisions should not be used
		NiMatrix33 objRotation = obj->m_parent->m_worldTransform.rot * thingDefaultRot * newRot;

		//LOG("Before Maybe Collision Stuff Start");
		auto maybeldiff = ldiff;
		maybeldiff.x = maybeldiff.x * varLinearX;
		maybeldiff.y = maybeldiff.y * varLinearY;
		maybeldiff.z = maybeldiff.z * varLinearZ;

		NiPoint3 maybePos = target + (obj->m_parent->m_worldTransform.rot * (maybeldiff + (thingDefaultPos * nodeScale))); //add missing local pos

		//After cbp movement collision detection
		for (int i = 0; i < thingCollisionSpheres.size(); i++)
		{
			thingCollisionSpheres[i].offset100 = thingCollisionSpheres[i].offset0 * nodeScale;
			thingCollisionSpheres[i].worldPos = maybePos + (objRotation * thingCollisionSpheres[i].offset100);
			thingCollisionSpheres[i].radius100 = thingCollisionSpheres[i].radius0 * nodeScale;
			thingCollisionSpheres[i].radius100pwr2 = thingCollisionSpheres[i].radius100 * thingCollisionSpheres[i].radius100;
			hashIdList = GetHashIdsFromPos(thingCollisionSpheres[i].worldPos - playerPos, thingCollisionSpheres[i].radius100);
			for (int m = 0; m < hashIdList.size(); m++)
			{
				if (!(std::find(thingIdList.begin(), thingIdList.end(), hashIdList[m]) != thingIdList.end()))
				{
					thingIdList.emplace_back(hashIdList[m]);
				}
			}
		}
		for (int i = 0; i < thingCollisionCapsules.size(); i++)
		{
			thingCollisionCapsules[i].End1_offset100 = thingCollisionCapsules[i].End1_offset0 * nodeScale;
			thingCollisionCapsules[i].End1_worldPos = maybePos + (objRotation * thingCollisionCapsules[i].End1_offset100);
			thingCollisionCapsules[i].End1_radius100 = thingCollisionCapsules[i].End1_radius0 * nodeScale;
			thingCollisionCapsules[i].End1_radius100pwr2 = thingCollisionCapsules[i].End1_radius100 * thingCollisionCapsules[i].End1_radius100;
			thingCollisionCapsules[i].End2_offset100 = thingCollisionCapsules[i].End2_offset0 * nodeScale;
			thingCollisionCapsules[i].End2_worldPos = maybePos + (objRotation * thingCollisionCapsules[i].End2_offset100);
			thingCollisionCapsules[i].End2_radius100 = thingCollisionCapsules[i].End2_radius0 * nodeScale;
			thingCollisionCapsules[i].End2_radius100pwr2 = thingCollisionCapsules[i].End2_radius100 * thingCollisionCapsules[i].End2_radius100;
			hashIdList = GetHashIdsFromPos((thingCollisionCapsules[i].End2_worldPos + thingCollisionCapsules[i].End2_worldPos) * 0.5f - playerPos
				, (thingCollisionCapsules[i].End1_radius100 + thingCollisionCapsules[i].End2_radius100) * 0.5f);
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

		CollisionConfig.maybePos = maybePos;
		CollisionConfig.cnvRot = obj->m_parent->m_worldTransform.rot;
		CollisionConfig.objRot = objRotation;
		CollisionConfig.invRot = invRot;

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

					bool colliding = false;
					colliding = partitions[id].partitionCollisions[i].CheckCollision(collisionVector, thingCollisionSpheres, thingCollisionCapsules, CollisionConfig, false);
					if (colliding)
					{
						maybeNot = true;
					}

					collisionCheckCount++;
				}
			}
		}

		//ground collision	
		if (GroundCollisionEnabled)
		{
			NiAVObject* groundobj, * highheelobj;
			thing_ReadNode_lock.lock();
			groundobj = loadedState->node->GetObjectByName(&GroundReferenceBone.data);
			highheelobj = loadedState->node->GetObjectByName(&highheel.data);
			thing_ReadNode_lock.unlock();

			if (groundobj)
			{
				float groundPos = groundobj->m_worldTransform.pos.z; //Get ground by NPC Root [Root] node

				if (highheelobj)
				{
					groundPos = groundPos - highheelobj->m_localTransform.pos.z; //Get highheel offset by NPC node
				}

				float bottomPos = groundPos;
				float bottomRadius = 0.0f;

				for (int l = 0; l < thingCollisionSpheres.size(); l++)
				{
					if (thingCollisionSpheres[l].worldPos.z - thingCollisionSpheres[l].radius100 < bottomPos - bottomRadius)
					{
						bottomPos = thingCollisionSpheres[l].worldPos.z;
						bottomRadius = thingCollisionSpheres[l].radius100;
					}
				}

				for (int m = 0; m < thingCollisionCapsules.size(); m++)
				{
					if (thingCollisionCapsules[m].End1_worldPos.z - thingCollisionCapsules[m].End1_radius100 < thingCollisionCapsules[m].End2_worldPos.z - thingCollisionCapsules[m].End2_radius100)
					{
						if (thingCollisionCapsules[m].End1_worldPos.z - thingCollisionCapsules[m].End1_radius100 < bottomPos - bottomRadius)
						{
							bottomPos = thingCollisionCapsules[m].End1_worldPos.z;
							bottomRadius = thingCollisionCapsules[m].End1_radius100;
						}
					}
					else
					{
						if (thingCollisionCapsules[m].End2_worldPos.z - thingCollisionCapsules[m].End2_radius100 < bottomPos - bottomRadius)
						{
							bottomPos = thingCollisionCapsules[m].End2_worldPos.z;
							bottomRadius = thingCollisionCapsules[m].End2_radius100;
						}
					}
				}

				if (bottomPos - bottomRadius < groundPos)
				{
					maybeNot = true;

					float Scalar = groundPos - (bottomPos - bottomRadius);

					//it can allow only force up to the radius for doesn't get crushed by the ground
					if (Scalar > bottomRadius)
					{
						if (Scalar < bottomRadius * 1.5f)
							Scalar = bottomRadius;
						else
							Scalar = 0;
					}

					GroundCollisionVector = NiPoint3(0, 0, Scalar);
				}
			}
		}

		if (maybeNot)
		{
			collisionOnLastFrame = maybeNot;

			ldiffcol = invRot * (collisionVector * collisionPenetration);
			ldiffGcol = invRot * (GroundCollisionVector * collisionPenetration);

			//Add more collision force for weak bone weights but virtually for maintain collision by node position
			//For example, if a node has a bone weight value of about 0.1, that shape seems actually moves by 0.1 even if the node moves by 1
			//However, simply applying the multipler then changes the actual node position,so that's making the collisions out of sync
			//Therefore to make perfect collision
			//it seems to be pushed out as much as colliding to the naked eye, but the actual position of the colliding node must be maintained original position
			maybeIdiffcol = (ldiffcol + ldiffGcol) * collisionMultipler;

			//add collision vector buffer of one frame to some reduce jitter and add softness by collision
			//be particularly useful for both nodes colliding that defined in both affected and collider nodes
			auto maybeldiffcoltmp = maybeIdiffcol;
			maybeIdiffcol = (maybeIdiffcol + collisionBuffer) * 0.5;
			collisionBuffer = maybeldiffcoltmp;

			//set to collision sync for the node that has both affected node and collider node
			collisionSync = obj->m_parent->m_worldTransform.rot * (ldiffcol + ldiffGcol - maybeIdiffcol);

			auto rcoldiffXnew = (ldiffcol + ldiffGcol) * collisionMultiplerRot * varRotationalXnew;
			auto rcoldiffYnew = (ldiffcol + ldiffGcol) * collisionMultiplerRot * varRotationalYnew;
			auto rcoldiffZnew = (ldiffcol + ldiffGcol) * collisionMultiplerRot * varRotationalZnew;

			rcoldiffXnew.x *= linearXrotationX;
			rcoldiffXnew.y *= linearYrotationX;
			rcoldiffXnew.z *= linearZrotationX;

			rcoldiffYnew.x *= linearXrotationY;
			rcoldiffYnew.y *= linearYrotationY;
			rcoldiffYnew.z *= linearZrotationY;

			rcoldiffZnew.x *= linearXrotationZ;
			rcoldiffZnew.y *= linearYrotationZ;
			rcoldiffZnew.z *= linearZrotationZ;

			NiMatrix33 newcolRot;
			newcolRot.SetEulerAngles(rcoldiffYnew.x + rcoldiffYnew.y + rcoldiffYnew.z, rcoldiffZnew.x + rcoldiffZnew.y + rcoldiffZnew.z, rcoldiffXnew.x + rcoldiffXnew.y + rcoldiffXnew.z);

			newRot = newRot * newcolRot;
		}
		else
		{
			collisionSync = emptyPoint;
		}
		///#### collision calculate done

		//LOG("After Maybe Collision Stuff End");
	}
	//Now the collision accuracy is completely
	//But can't easy to resolve the cause for the collisions to become unstable when low or unstable FPS if applied the "collisionElastic"
	//Maybe the best answer is dividing one frame by the missing frame count then need to additional repeat calculations
	//e.g) if fps is 30 then need to maintained 60 calculations like in 60 fps, therefore It guarantees 60 fps of calculation by performing calculations twice per frame
	//But rather than this, maybe there's a better way to do it like way that actual results is not perfect but pefomance is better or way that tricks... like edit velocity...


	//Logging
	if (logging != 0)
	{
		TESObjectREFR* actorRef = DYNAMIC_CAST(actor, Actor, TESObjectREFR);
		if (actorRef)
		{
			LOG("%s - %s - Thing.update() %s - Diff: %g %g %g - Collision: %s - CheckCount: %d", CALL_MEMBER_FN(actorRef, GetReferenceName)(), IsActorMale(actor) ? "Male" : "Female", boneName.data, diff.x, diff.y, diff.z, IsThereCollision ? (maybeNot ? "mYES" : "YES") : "no", collisionCheckCount);
		}
	}

	//logger.error("set positions\n");

	//If put the result of collision into the next frame, the quality of collision and movement will improve
	//but if that part is almost exclusively for collisions like vagina, it's better not to reflect the result of collision into physics
	//### To be free from unstable FPS, have to remove the varGravityCorrection from the next frame
	if (collisionElastic && maybeNot)
	{
		oldWorldPos = (obj->m_parent->m_worldTransform.rot * (ldiff + ldiffcol)) + target - NiPoint3(0, 0, varGravityCorrection);
		oldWorldPosRot = (obj->m_parent->m_worldTransform.rot * (ldiffRot + ldiffcol)) + target - NiPoint3(0, 0, varGravityCorrection);
	}
	else
	{
		oldWorldPos = (obj->m_parent->m_worldTransform.rot * ldiff) + target - NiPoint3(0, 0, varGravityCorrection);
		oldWorldPosRot = (obj->m_parent->m_worldTransform.rot * ldiffRot) + target - NiPoint3(0, 0, varGravityCorrection);
	}

	thing_SetNode_lock.lock();
	obj->m_localTransform.pos.x = thingDefaultPos.x + XdefaultOffset + (ldiff.x * varLinearX) + maybeIdiffcol.x;
	obj->m_localTransform.pos.y = thingDefaultPos.y + YdefaultOffset + (ldiff.y * varLinearY) + maybeIdiffcol.y;
	obj->m_localTransform.pos.z = thingDefaultPos.z + ZdefaultOffset + (ldiff.z * varLinearZ) + maybeIdiffcol.z;
	obj->m_localTransform.rot = thingDefaultRot * newRot;
	thing_SetNode_lock.unlock();

	RefreshNode(obj, thing_Refresh_node_lock);

	//logger.error("end update()\n");
	/*QueryPerformanceCounter(&endingTime);
	elapsedMicroseconds.QuadPart = endingTime.QuadPart - startingTime.QuadPart;
	elapsedMicroseconds.QuadPart *= 1000000000LL;
	elapsedMicroseconds.QuadPart /= frequency.QuadPart;
	LOG("Thing.update() Update Time = %lld ns\n", elapsedMicroseconds.QuadPart);*/
}

void Thing::CalculateDiffVagina(NiPoint3 &collisionDiff, float opening, bool isleftandright, bool leftORback)
{
	if (opening > 0)
	{
		if (isleftandright)
		{
			if (leftORback)
			{
				collisionDiff = NiPoint3(thing_vaginaOpeningMultiplier * -1, 0, 0) * (opening * 0.5f);
			}
			else
			{
				collisionDiff = NiPoint3(thing_vaginaOpeningMultiplier, 0, 0) * (opening * 0.5f);
			}
		}
		else
		{
			if (leftORback)
			{
				collisionDiff = NiPoint3(0, thing_vaginaOpeningMultiplier * -0.75f, thing_vaginaOpeningMultiplier * 0.25f) * (opening * 0.5f);
			}
			else
			{
				collisionDiff = NiPoint3(0, thing_vaginaOpeningMultiplier * 0.125f, thing_vaginaOpeningMultiplier * -0.25f) * (opening * 0.5f);
			}
		}
	}
	else
	{
		collisionDiff = emptyPoint;
	}
}
