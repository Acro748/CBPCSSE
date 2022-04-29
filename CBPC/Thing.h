#pragma once
#include <time.h>
#include <string>
#include <vector>

#include "skse64\NiNodes.h"
#include "skse64\PapyrusActorValueInfo.h"
#include "skse64\NiTypes.h"
#include "skse64\NiObjects.h"
#include "skse64\PapyrusForm.h"
#include "CollisionHub.h"


extern const char *leftPussy;
extern const char *rightPussy;

class Thing {
	BSFixedString boneName;
	NiPoint3 oldWorldPos;
	NiPoint3 oldWorldPosRot;
	NiPoint3 velocity;
	NiPoint3 velocityRot;
	clock_t time;

public:
	bool ActorCollisionsEnabled = true;
	bool GroundCollisionEnabled = true;
	bool VirtualCollisionEnabled = false;
	bool IsBreastBone = false;
	bool IsLeftBreastBone = false;
	bool IsRightBreastBone = false;
	bool IsBellyBone = false;

	float stiffness = 0.0f;
	float stiffnessX = 0.0f;
	float stiffnessY = 0.0f;
	float stiffnessZ = 0.0f;
	float stiffnessXRot = 0.0f;
	float stiffnessYRot = 0.0f;
	float stiffnessZRot = 0.0f;
	float stiffness2 = 0.0f;
	float stiffness2X = 0.0f;
	float stiffness2Y = 0.0f;
	float stiffness2Z = 0.0f;
	float stiffness2XRot = 0.0f;
	float stiffness2YRot = 0.0f;
	float stiffness2ZRot = 0.0f;
	float damping = 0.0f;
	float dampingX = 0.0f;
	float dampingY = 0.0f;
	float dampingZ = 0.0f;
	float dampingXRot = 0.0f;
	float dampingYRot = 0.0f;
	float dampingZRot = 0.0f;
	//float maxOffset = 0.0f;
	float XmaxOffset = 5.0f;
	float XminOffset = -5.0f;
	float YmaxOffset = 5.0f;
	float YminOffset = -5.0f;
	float ZmaxOffset = 5.0f;
	float ZminOffset = -5.0f;
	float XmaxOffsetRot = 5.0f;
	float XminOffsetRot = -5.0f;
	float YmaxOffsetRot = 5.0f;
	float YminOffsetRot = -5.0f;
	float ZmaxOffsetRot = 5.0f;
	float ZminOffsetRot = -5.0f;
	float XdefaultOffset = 0.0f;
	float YdefaultOffset = 0.0f;
	float ZdefaultOffset = 0.0f;
	float cogOffset = 0.0f; //no
	float gravityBias = 0.0f; //no
	float gravityCorrection = 0.0f; //no

	float timeTick = 4.0f;
	float timeTickRot = 0.0f;
	float linearX = 0.0f;
	float linearY = 0.0f;
	float linearZ = 0.0f;
	float rotationalXnew = 0.0f; //was originally rotational
	float rotationalYnew = 0.0f;
	float rotationalZnew = 0.0f;
	float linearXrotationX = 0.0f;
	float linearXrotationY = 1.0f;
	float linearXrotationZ = 0.0f;
	float linearYrotationX = 0.0f;
	float linearYrotationY = 0.0f;
	float linearYrotationZ = 1.0f;
	float linearZrotationX = 1.0f;
	float linearZrotationY = 0.0f;
	float linearZrotationZ = 0.0f;
	float timeStep = 1.0f;
	float timeStepRot = 0.0f;

	float linearXspreadforceY = 0.0f;
	float linearXspreadforceZ = 0.0f;
	float linearYspreadforceX = 0.0f;
	float linearYspreadforceZ = 0.0f;
	float linearZspreadforceX = 0.0f;
	float linearZspreadforceY = 0.0f;
	float rotationXspreadforceY = 0.0f;
	float rotationXspreadforceZ = 0.0f;
	float rotationYspreadforceX = 0.0f;
	float rotationYspreadforceZ = 0.0f;
	float rotationZspreadforceX = 0.0f;
	float rotationZspreadforceY = 0.0f;

	float forceMultipler = 1.0f;

	float gravityInvertedCorrection = 0.0f;
	float gravityInvertedCorrectionStart = 0.0f;
	float gravityInvertedCorrectionEnd = 0.0f;

	float breastClothedPushup = 0.0;
	float breastLightArmoredPushup = 0.0;
	float breastHeavyArmoredPushup = 0.0;

	float breastClothedAmplitude = 1.0;
	float breastLightArmoredAmplitude = 1.0;
	float breastHeavyArmoredAmplitude = 1.0;

	float collisionFriction = 0.8f;
	float collisionPenetration = 1.0f;
	float collisionMultipler = 1.0f;
	float collisionMultiplerRot = 1.0f;

	bool collisionElastic = false;

	float collisionXmaxOffset = 100.0f;
	float collisionXminOffset = -100.0f;
	float collisionYmaxOffset = 100.0f;
	float collisionYminOffset = -100.0f;
	float collisionZmaxOffset = 100.0f;
	float collisionZminOffset = -100.0f;

	//100 weight
	float stiffness_100 = 0.0f;
	float stiffnessX_100 = 0.0f;
	float stiffnessY_100 = 0.0f;
	float stiffnessZ_100 = 0.0f;
	float stiffnessXRot_100 = 0.0f;
	float stiffnessYRot_100 = 0.0f;
	float stiffnessZRot_100 = 0.0f;
	float stiffness2_100 = 0.0f;
	float stiffness2X_100 = 0.0f;
	float stiffness2Y_100 = 0.0f;
	float stiffness2Z_100 = 0.0f;
	float stiffness2XRot_100 = 0.0f;
	float stiffness2YRot_100 = 0.0f;
	float stiffness2ZRot_100 = 0.0f;
	float damping_100 = 0.0f;
	float dampingX_100 = 0.0f;
	float dampingY_100 = 0.0f;
	float dampingZ_100 = 0.0f;
	float dampingXRot_100 = 0.0f;
	float dampingYRot_100 = 0.0f;
	float dampingZRot_100 = 0.0f;
	float maxOffset_100 = 0.0f;
	float XmaxOffset_100 = 5.0f;
	float XminOffset_100 = -5.0f;
	float YmaxOffset_100 = 5.0f;
	float YminOffset_100 = -5.0f;
	float ZmaxOffset_100 = 5.0f;
	float ZminOffset_100 = -5.0f;
	float XmaxOffsetRot_100 = 5.0f;
	float XminOffsetRot_100 = -5.0f;
	float YmaxOffsetRot_100 = 5.0f;
	float YminOffsetRot_100 = -5.0f;
	float ZmaxOffsetRot_100 = 5.0f;
	float ZminOffsetRot_100 = -5.0f;
	float XdefaultOffset_100 = 0.0f;
	float YdefaultOffset_100 = 0.0f;
	float ZdefaultOffset_100 = 0.0f;
	float cogOffset_100 = 0.0f; //no
	float gravityBias_100 = 0.0f; //no
	float gravityCorrection_100 = 0.0f; //no

	float timeTick_100 = 4.0f;
	float timeTickRot_100 = 0.0f;
	float linearX_100 = 0.0f;
	float linearY_100 = 0.0f;
	float linearZ_100 = 0.0f;
	float rotationalXnew_100 = 0.0f; //was originally rotational
	float rotationalYnew_100 = 0.0f;
	float rotationalZnew_100 = 0.0f;
	float linearXrotationX_100 = 0.0f;
	float linearXrotationY_100 = 1.0f;
	float linearXrotationZ_100 = 0.0f;
	float linearYrotationX_100 = 0.0f;
	float linearYrotationY_100 = 0.0f;
	float linearYrotationZ_100 = 1.0f;
	float linearZrotationX_100 = 1.0f;
	float linearZrotationY_100 = 0.0f;
	float linearZrotationZ_100 = 0.0f;
	float timeStep_100 = 1.0f;
	float timeStepRot_100 = 0.0f;

	float linearXspreadforceY_100 = 0.0f;
	float linearXspreadforceZ_100 = 0.0f;
	float linearYspreadforceX_100 = 0.0f;
	float linearYspreadforceZ_100 = 0.0f;
	float linearZspreadforceX_100 = 0.0f;
	float linearZspreadforceY_100 = 0.0f;
	float rotationXspreadforceY_100 = 0.0f;
	float rotationXspreadforceZ_100 = 0.0f;
	float rotationYspreadforceX_100 = 0.0f;
	float rotationYspreadforceZ_100 = 0.0f;
	float rotationZspreadforceX_100 = 0.0f;
	float rotationZspreadforceY_100 = 0.0f;

	float forceMultipler_100 = 1.0f;

	float gravityInvertedCorrection_100 = 0.0f;
	float gravityInvertedCorrectionStart_100 = 0.0f;
	float gravityInvertedCorrectionEnd_100 = 0.0f;
	
	float breastClothedPushup_100 = 0.0f;
	float breastLightArmoredPushup_100 = 0.0f;
	float breastHeavyArmoredPushup_100 = 0.0f;
	
	float breastClothedAmplitude_100 = 1.0f;
	float breastLightArmoredAmplitude_100 = 1.0f;
	float breastHeavyArmoredAmplitude_100 = 1.0f;

	float collisionFriction_100 = 0.8f;
	float collisionPenetration_100 = 1.0f;
	float collisionMultipler_100 = 1.0f;
	float collisionMultiplerRot_100 = 1.0f;

	float collisionElastic_100 = 0.0f;

	float collisionXmaxOffset_100 = 100.0f;
	float collisionXminOffset_100 = -100.0f;
	float collisionYmaxOffset_100 = 100.0f;
	float collisionYminOffset_100 = -100.0f;
	float collisionZmaxOffset_100 = 100.0f;
	float collisionZminOffset_100 = -100.0f;

	//0 weight
	float stiffness_0 = 0.0f;
	float stiffnessX_0 = 0.0f;
	float stiffnessY_0 = 0.0f;
	float stiffnessZ_0 = 0.0f;
	float stiffnessXRot_0 = 0.0f;
	float stiffnessYRot_0 = 0.0f;
	float stiffnessZRot_0 = 0.0f;
	float stiffness2_0 = 0.0f;
	float stiffness2X_0 = 0.0f;
	float stiffness2Y_0 = 0.0f;
	float stiffness2Z_0 = 0.0f;
	float stiffness2XRot_0 = 0.0f;
	float stiffness2YRot_0 = 0.0f;
	float stiffness2ZRot_0 = 0.0f;
	float damping_0 = 0.0f;
	float dampingX_0 = 0.0f;
	float dampingY_0 = 0.0f;
	float dampingZ_0 = 0.0f;
	float dampingXRot_0 = 0.0f;
	float dampingYRot_0 = 0.0f;
	float dampingZRot_0 = 0.0f;
	float maxOffset_0 = 0.0f; 
	float XmaxOffset_0 = 5.0f;
	float XminOffset_0 = -5.0f;
	float YmaxOffset_0 = 5.0f;
	float YminOffset_0 = -5.0f;
	float ZmaxOffset_0 = 5.0f;
	float ZminOffset_0 = -5.0f;
	float XmaxOffsetRot_0 = 5.0f;
	float XminOffsetRot_0 = -5.0f;
	float YmaxOffsetRot_0 = 5.0f;
	float YminOffsetRot_0 = -5.0f;
	float ZmaxOffsetRot_0 = 5.0f;
	float ZminOffsetRot_0 = -5.0f;
	float XdefaultOffset_0 = 0.0f;
	float YdefaultOffset_0 = 0.0f;
	float ZdefaultOffset_0 = 0.0f;
	float cogOffset_0 = 0.0f; //no
	float gravityBias_0 = 0.0f; //no
	float gravityCorrection_0 = 0.0f; //no

	float timeTick_0 = 4.0f;
	float timeTickRot_0 = 0.0f;
	float linearX_0 = 0.0f;
	float linearY_0 = 0.0f;
	float linearZ_0 = 0.0f;
	float rotationalXnew_0 = 0.0f; //was originally rotational
	float rotationalYnew_0 = 0.0f;
	float rotationalZnew_0 = 0.0f;
	float linearXrotationX_0 = 0.0f;
	float linearXrotationY_0 = 1.0f;
	float linearXrotationZ_0 = 0.0f;
	float linearYrotationX_0 = 0.0f;
	float linearYrotationY_0 = 0.0f;
	float linearYrotationZ_0 = 1.0f;
	float linearZrotationX_0 = 1.0f;
	float linearZrotationY_0 = 0.0f;
	float linearZrotationZ_0 = 0.0f;
	float timeStep_0 = 1.0f;
	float timeStepRot_0 = 0.0f;

	float linearXspreadforceY_0 = 0.0f;
	float linearXspreadforceZ_0 = 0.0f;
	float linearYspreadforceX_0 = 0.0f;
	float linearYspreadforceZ_0 = 0.0f;
	float linearZspreadforceX_0 = 0.0f;
	float linearZspreadforceY_0 = 0.0f;
	float rotationXspreadforceY_0 = 0.0f;
	float rotationXspreadforceZ_0 = 0.0f;
	float rotationYspreadforceX_0 = 0.0f;
	float rotationYspreadforceZ_0 = 0.0f;
	float rotationZspreadforceX_0 = 0.0f;
	float rotationZspreadforceY_0 = 0.0f;

	float forceMultipler_0 = 1.0f;

	float gravityInvertedCorrection_0 = 0.0f;
	float gravityInvertedCorrectionStart_0 = 0.0f;
	float gravityInvertedCorrectionEnd_0 = 0.0f;

	float breastClothedPushup_0 = 0.0f;
	float breastLightArmoredPushup_0 = 0.0f;
	float breastHeavyArmoredPushup_0 = 0.0f;

	float breastClothedAmplitude_0 = 1.0f;
	float breastLightArmoredAmplitude_0 = 1.0f;
	float breastHeavyArmoredAmplitude_0 = 1.0f;

	float collisionFriction_0 = 0.8f;
	float collisionPenetration_0 = 1.0f;
	float collisionMultipler_0 = 1.0f;
	float collisionMultiplerRot_0 = 1.0f;

	float collisionElastic_0 = 0.0f;

	float collisionXmaxOffset_0 = 100.0f;
	float collisionXminOffset_0 = -100.0f;
	float collisionYmaxOffset_0 = 100.0f;
	float collisionYminOffset_0 = -100.0f;
	float collisionZmaxOffset_0 = 100.0f;
	float collisionZminOffset_0 = -100.0f;

	Thing(Actor *actor, NiAVObject *obj, BSFixedString &name);
	~Thing();

	Actor *ownerActor;
	NiAVObject * node;

	float actorWeight = 50;

	float thing_bellybulgemultiplier = 2.0f;
	float thing_bellybulgeposmultiplier = -3.0f;
	float thing_bellybulgemax = 10.0f;
	float thing_bellybulgeposlowest = -12.0f;
	std::vector<std::string> thing_bellybulgelist = { "Genitals01" };
	float thing_bellyBulgeReturnTime = 1.5f;
	float thing_vaginaOpeningLimit = 5.0f;
	float thing_vaginaOpeningMultiplier = 4.0f;

	std::vector<std::string> IgnoredCollidersList;
	std::vector<std::string> IgnoredSelfCollidersList;
	bool IgnoreAllSelfColliders = false;


	void updateConfigValues(Actor* actor);
	void updateConfig(Actor* actor, configEntry_t &centry, configEntry_t& centry0weight);
	void dump();
	
	void update(Actor *actor, std::shared_mutex &thing_SetNode_lock, std::shared_mutex &thing_ReadNode_lock, std::shared_mutex &thing_Refresh_node_lock);
	void updatePelvis(Actor *actor, std::shared_mutex& thing_SetNode_lock, std::shared_mutex& thing_ReadNode_lock, std::shared_mutex& thing_Refresh_node_lock);
	bool ApplyBellyBulge(Actor * actor, std::shared_mutex& thing_SetNode_lock, std::shared_mutex& thing_ReadNode_lock);
	void CalculateDiffVagina(NiPoint3 &collisionDiff, float opening, bool isleftandright, bool leftORback);
	void reset();

	static float remap(float value, float start1, float stop1, float start2, float stop2) 
	{
		return start2 + (stop2 - start2) * ((value - start1) / (stop1 - start1));
	}

	//Collision Stuff
	//--------------------------------------------------------------

	//Performance skip
	int skipFramesCount = 0;
	int skipFramesPelvisCount = 0;
	bool collisionOnLastFrame = false;


	NiPoint3 lastColliderPosition = emptyPoint;

	std::vector<Sphere> thingCollisionSpheres;
	std::vector<Capsule> thingCollisionCapsules;

	std::vector<Collision> ownColliders;

	float scaleWeight = 1.0f;


	std::vector<Sphere> CreateThingCollisionSpheres(Actor* actor, std::string nodeName);
	std::vector<Capsule> CreateThingCollisionCapsules(Actor * actor, std::string nodeName);

	float movementMultiplier = 0.8f;

	bool updatePussyFirstRun = true;
	NiPoint3 leftPussyDefaultPos;
	NiPoint3 rightPussyDefaultPos;
	NiPoint3 backPussyDefaultPos;
	NiPoint3 frontPussyDefaultPos;

	bool updateBellyFirstRun = true;
	NiPoint3 bellyDefaultPos;

	bool updateThingFirstRun = true;
	NiPoint3 thingDefaultPos;
	NiMatrix33 thingDefaultRot;

	float actorBaseScale = 1.0f;
	float nodeScale = 1.0f;

	//Extra variables
	NiPoint3 collisionBuffer = emptyPoint;
	NiPoint3 collisionSync = emptyPoint;

	NiPoint3 collisionInertia = emptyPoint;
	NiPoint3 collisionInertiaRot = emptyPoint;
	float multiplerInertia = 0.0f;
	float multiplerInertiaRot = 0.0f;

	CollisionConfigs CollisionConfig;

	float lastMaxOffsetY = 0.0f;
	float lastMaxOffsetZ = 0.0f;
	int bellyBulgeCountDown = 0;
	float oldBulgeY = 0.0f;

	int frameCounts = 0;

	bool isClothed = false;
	bool isLightArmor = false;
	bool isHeavyArmor = false;
	bool isNoPushUp = false;
	int skipArmorCheck = 0;
	float forceAmplitude = 1.0f;

	float varGravityCorrection = -1 * gravityCorrection;
};
