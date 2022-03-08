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
	NiPoint3 velocity;
	clock_t time;

public:
	bool ActorCollisionsEnabled = true;
	bool IsBreastBone = false;
	bool IsLeftBreastBone = false;
	bool IsRightBreastBone = false;
	bool IsBellyBone = false;

	float stiffness = 0.5f;
	float stiffness2 = 0.0f;
	float damping = 0.2f;
	float maxOffset = 5.0f;
	float cogOffset = 0.0f; //no
	float gravityBias = 0.0f; //no
	float gravityCorrection = 0.0f; //no

	float timeTick = 4.0f;
	float linearX = 0.0f;
	float linearY = 0.0f;
	float linearZ = 0.0f;
	float rotationalXnew = 0.1f; //was originally rotational
	float rotationalYnew = 0.1f;
	float rotationalZnew = 0.1f;
	float timeStep = 1.0f;

	float gravityInvertedCorrection = 0.0f;
	float gravityInvertedCorrectionStart = 0.0f;
	float gravityInvertedCorrectionEnd = 0.0f;

	float breastClothedPushup = 0.0;
	float breastLightArmoredPushup = 0.0;
	float breastHeavyArmoredPushup = 0.0;

	float breastClothedAmplitude = 1.0;
	float breastLightArmoredAmplitude = 1.0;
	float breastHeavyArmoredAmplitude = 1.0;

	//100 weight
	float stiffness_100 = 0.5f;
	float stiffness2_100 = 0.0f;
	float damping_100 = 0.2f;
	float maxOffset_100 = 5.0f;
	float cogOffset_100 = 0.0f; //no
	float gravityBias_100 = 0.0f; //no
	float gravityCorrection_100 = 0.0f; //no

	float timeTick_100 = 4.0f;
	float linearX_100 = 0.0f;
	float linearY_100 = 0.0f;
	float linearZ_100 = 0.0f;
	float rotationalXnew_100 = 0.1f; //was originally rotational
	float rotationalYnew_100 = 0.1f;
	float rotationalZnew_100 = 0.1f;
	float timeStep_100 = 1.0f;

	float gravityInvertedCorrection_100 = 0.0f;
	float gravityInvertedCorrectionStart_100 = 0.0f;
	float gravityInvertedCorrectionEnd_100 = 0.0f;
	
	float breastClothedPushup_100 = 0.0f;
	float breastLightArmoredPushup_100 = 0.0f;
	float breastHeavyArmoredPushup_100 = 0.0f;
	
	float breastClothedAmplitude_100 = 1.0f;
	float breastLightArmoredAmplitude_100 = 1.0f;
	float breastHeavyArmoredAmplitude_100 = 1.0f;

	//0 weight
	float stiffness_0 = 0.5f;
	float stiffness2_0 = 0.0f;
	float damping_0 = 0.2f;
	float maxOffset_0 = 5.0f;
	float cogOffset_0 = 0.0f; //no
	float gravityBias_0 = 0.0f; //no
	float gravityCorrection_0 = 0.0f; //no

	float timeTick_0 = 4.0f;
	float linearX_0 = 0.0f;
	float linearY_0 = 0.0f;
	float linearZ_0 = 0.0f;
	float rotationalXnew_0 = 0.1f; //was originally rotational
	float rotationalYnew_0 = 0.1f;
	float rotationalZnew_0 = 0.1f;
	float timeStep_0 = 1.0f;

	float gravityInvertedCorrection_0 = 0.0f;
	float gravityInvertedCorrectionStart_0 = 0.0f;
	float gravityInvertedCorrectionEnd_0 = 0.0f;

	float breastClothedPushup_0 = 0.0f;
	float breastLightArmoredPushup_0 = 0.0f;
	float breastHeavyArmoredPushup_0 = 0.0f;

	float breastClothedAmplitude_0 = 1.0f;
	float breastLightArmoredAmplitude_0 = 1.0f;
	float breastHeavyArmoredAmplitude_0 = 1.0f;

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


	void Thing::updateConfigValues(Actor* actor);
	void updateConfig(Actor* actor, configEntry_t &centry, configEntry_t& centry0weight);
	void dump();
	
	void update(Actor *actor);
	void updatePelvis(Actor *actor);
	bool ApplyBellyBulge(Actor * actor);
	void CalculateDiffVagina(NiPoint3 &collisionDiff, float opening, bool left);
	void reset();

	static float Thing::remap(float value, float start1, float stop1, float start2, float stop2) 
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


	std::vector<Sphere> CreateThingCollisionSpheres(Actor* actor, std::string nodeName, float nodescale);
	std::vector<Capsule> CreateThingCollisionCapsules(Actor * actor, std::string nodeName, float nodescale);

	float movementMultiplier = 0.8f;

	bool updatePussyFirstRun = true;
	NiPoint3 leftPussyDefaultPos;
	NiPoint3 rightPussyDefaultPos;

	bool updateBellyFirstRun = true;
	NiPoint3 bellyDefaultPos;

	bool updateThingFirstRun = true;
	NiPoint3 thingDefaultPos;
	NiMatrix33 thingDefaultRot;


	//Extra variables
	float lastMaxOffsetY = 0.0f;
	float lastMaxOffsetZ = 0.0f;
	int bellyBulgeCountDown = 0;
	float oldBulgeY = 0.0f;

	int frameCounts = 0;

	bool isClothed = false;
	bool isLightArmor = false;
	bool isHeavyArmor = false;
	int skipArmorCheck = 0;
	float forceAmplitude = 1.0f;

	float varGravityCorrection = -1 * gravityCorrection;

	//FPS corrction
	float fps60Tick = 1.0f / 60.0f * 1000.0f;
	float fpsCorrection = 1.0f;
};