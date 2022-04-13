#pragma once
#include <unordered_map>
#include <vector>
#include <iostream>
#include <string>
#include <fstream>

#include <shared_mutex>

#include "skse64/NiGeometry.h"
#include "skse64\GameReferences.h"
#include "skse64\PapyrusEvents.h"
#include "skse64\GameRTTI.h"
#include "skse64\GameSettings.h"
#include "skse64_common/skse_version.h"

#include "skse64/NiNodes.h"
#include "skse64/NiGeometry.h"
#include "skse64/NiRTTI.h"

#include "skse64/PapyrusNativeFunctions.h"
#include "skse64\PapyrusActor.h"
#include <skse64/PapyrusKeyword.h>

#include "Utility.hpp"
#include "skse64/GameMenus.h"
#include "skse64/GameData.h"
#include "skse64/GameEvents.h"

#include <atomic>

#ifdef RUNTIME_VR_VERSION_1_4_15
#include "skse64/openvr_1_0_12.h"
#endif

typedef std::unordered_map<std::string, float> configEntry_t;
typedef std::unordered_map<std::string, configEntry_t> config_t;

extern std::unordered_map<std::string, std::string> configMap;

extern int configReloadCount;
extern config_t config;
extern config_t config0weight;

extern int collisionSkipFrames;
extern int collisionSkipFramesPelvis;

extern std::unordered_map<std::string, bool> ActorNodeStoppedPhysicsMap;


//typedef std::unordered_map<std::string, bool> nodeCollisionMap;
//typedef std::unordered_map<std::string, NiPoint3> nodeCollisionAmountMap;
//extern std::unordered_map<Actor*, nodeCollisionMap> LeftBreastCollisionMap;
//extern std::unordered_map<Actor*, nodeCollisionMap> RightBreastCollisionMap;
//
//extern std::unordered_map<Actor*, nodeCollisionAmountMap> BreastCollisionAmountMap;

void loadConfig();

void GameLoad();

extern int gridsize;
extern int adjacencyValue;
extern int tuningModeCollision;
extern int malePhysics;
//extern int malePhysicsOnlyForExtraRaces;
extern float actorDistance;
extern float actorBounceDistance;
extern int actorAngle;

extern int inCombatActorCount;
extern int outOfCombatActorCount;


extern float cbellybulge;
extern float cbellybulgemax;
extern float cbellybulgeposlowest;
extern std::vector<std::string> bellybulgenodesList;

extern float vaginaOpeningLimit;
extern float vaginaOpeningMultiplier;
extern float anusOpeningLimit;
extern float anusOpeningMultiplier;
extern float bellyBulgeReturnTime;

//extern std::vector<std::string> extraRacesList;

extern int logging;

extern int useCamera;

extern int fpsCorrectionEnabled;

extern std::vector<std::string> noJitterFixNodesList;



//extern int useOldHook;

extern std::atomic<bool> dialogueMenuOpen;
extern std::atomic<bool> raceSexMenuClosed;
extern std::atomic<bool> raceSexMenuOpen;
extern std::atomic<bool> MainMenuOpen;
extern std::atomic<bool> consoleConfigReload;
extern std::atomic<bool> consoleCollisionReload;

extern std::string breastGravityReferenceBoneName;
extern BSFixedString breastGravityReferenceBoneString;

extern std::string GroundReferenceBoneName;
extern BSFixedString GroundReferenceBone;

extern BGSKeyword* KeywordArmorClothing;
extern BGSKeyword* KeywordArmorLight;
extern BGSKeyword* KeywordArmorHeavy;
extern BGSKeyword* KeywordActorTypeNPC;

extern BSFixedString KeywordNameAsNakedL;
extern BSFixedString KeywordNameAsNakedR;
extern BSFixedString KeywordNameAsClothingL;
extern BSFixedString KeywordNameAsClothingR;
extern BSFixedString KeywordNameAsLightL;
extern BSFixedString KeywordNameAsLightR;
extern BSFixedString KeywordNameAsHeavyL;
extern BSFixedString KeywordNameAsHeavyR;
extern BSFixedString KeywordNameNoPushUpL;
extern BSFixedString KeywordNameNoPushUpR;


extern UInt32 VampireLordBeastRaceFormId;


enum eLogLevels
{
	LOGLEVEL_ERR = 0,
	LOGLEVEL_WARN,
	LOGLEVEL_INFO,
};

void Log(const int msgLogLevel, const char * fmt, ...);

extern std::shared_mutex log_lock;

#define LOG(fmt, ...) Log(LOGLEVEL_WARN, fmt, ##__VA_ARGS__)
#define LOG_ERR(fmt, ...) Log(LOGLEVEL_ERR, fmt, ##__VA_ARGS__)
#define LOG_INFO(fmt, ...) Log(LOGLEVEL_INFO, fmt, ##__VA_ARGS__)


#ifdef RUNTIME_VR_VERSION_1_4_15
extern unsigned short hapticFrequency;
extern int hapticStrength;

extern bool leftHandedMode;
#endif

//Collision Stuff

struct Sphere
{
	NiPoint3 offset0 = NiPoint3(0, 0, 0);
	NiPoint3 offset100 = NiPoint3(0, 0, 0);
	double radius0 = 4.0;
	double radius100 = 4.0;
	double radius100pwr2 = 16.0;
	NiPoint3 worldPos = NiPoint3(0, 0, 0);
	std::string NodeName;
};

struct Capsule
{
	NiPoint3 End1_offset0 = NiPoint3(0, 0, 0);
	NiPoint3 End1_offset100 = NiPoint3(0, 0, 0);
	NiPoint3 End1_worldPos = NiPoint3(0, 0, 0);
	double End1_radius0 = 4.0;
	double End1_radius100 = 4.0;
	double End1_radius100pwr2 = 16.0;
	NiPoint3 End2_offset0 = NiPoint3(0, 0, 0);
	NiPoint3 End2_offset100 = NiPoint3(0, 0, 0);
	NiPoint3 End2_worldPos = NiPoint3(0, 0, 0);
	double End2_radius0 = 4.0;
	double End2_radius100 = 4.0;
	double End2_radius100pwr2 = 16.0;

	std::string NodeName;
};

struct ConfigLine
{
	std::vector<Sphere> CollisionSpheres;
	std::vector<Capsule> CollisionCapsules;
	std::string NodeName;
	std::vector<std::string> IgnoredColliders;
	std::vector<std::string> IgnoredSelfColliders;
	bool IgnoreAllSelfColliders = false;
};

enum ConditionType
{
	IsRaceFormId,
	IsRaceName,
	ActorFormId, 
	ActorName,
	ActorWeightGreaterThan,
	IsMale,
	IsFemale,
	IsPlayer,
	IsInFaction,
	HasKeywordId,
	HasKeywordName,
	RaceHasKeywordId,
	RaceHasKeywordName,
	IsActorBase,
	IsPlayerTeammate,
	IsUnique,
	IsVoiceType,
	IsCombatStyle,
	IsClass
};

struct ConditionItem
{
	//multiple items
	std::vector<ConditionItem> OrItems;

	//single item
	bool single = true;
	bool Not = false;
	ConditionType type;
	UInt32 id;
	std::string str;
};

struct Conditions
{
	std::vector<ConditionItem> AndItems;
};

extern std::unordered_map<std::string, Conditions> nodeConditionsMap;


struct SpecificNPCConfig
{
	Conditions conditions;
	int ConditionPriority = 50;

	std::vector<std::string> AffectedNodeLines;
	std::vector<std::string> ColliderNodeLines;

	std::vector<ConfigLine> AffectedNodesList;

	std::vector<ConfigLine> ColliderNodesList;

	float cbellybulge;
	float cbellybulgemax;
	float cbellybulgeposlowest;
	std::vector<std::string> bellybulgenodesList;

	float bellyBulgeReturnTime = 1.5f;

	float vaginaOpeningLimit = 5.0f;
	float vaginaOpeningMultiplier = 4.0f;
};

extern std::vector<SpecificNPCConfig> specificNPCConfigList;


struct SpecificNPCBounceConfig
{
	Conditions conditions;
	int ConditionPriority = 50;

	config_t config;
	config_t config0weight;
};

extern std::vector<SpecificNPCBounceConfig> specificNPCBounceConfigList;

bool compareConfigs(const SpecificNPCConfig& config1, const SpecificNPCConfig& config2);
bool compareBounceConfigs(const SpecificNPCBounceConfig& config1, const SpecificNPCBounceConfig& config2);

bool GetSpecificNPCBounceConfigForActor(Actor* actor, SpecificNPCBounceConfig& snbc);
bool IsConfigActuallyAllocated(SpecificNPCBounceConfig snbc, std::string section); //If the config of that part is not set and just set to default, return false

std::vector<BGSKeyword*> GetKeywordListByString(BSFixedString keyword);
#ifdef RUNTIME_VR_VERSION_1_4_15
extern std::vector<std::string> PlayerNodeLines;
extern std::vector<ConfigLine> PlayerNodesList;   //Player nodes that can collide nodes
std::string GetWeaponTypeName(UInt8 kType);
void GetSettings();

extern float MeleeWeaponTranslateX;
extern float MeleeWeaponTranslateY;
extern float MeleeWeaponTranslateZ;
//Weapon Collision Stuff

struct Triangle
{
	NiPoint3 orga;
	NiPoint3 orgb;
	NiPoint3 orgc;

	NiPoint3 a;
	NiPoint3 b;
	NiPoint3 c;
};

struct WeaponConfigLine
{
	Triangle CollisionTriangle;
	std::string WeaponName;
};

extern std::vector<WeaponConfigLine> WeaponCollidersList; //Weapon colliders

std::vector<Triangle> GetCollisionTriangles(std::string name, UInt8 kType);

void ConfigWeaponLineSplitter(std::string &line, Triangle &newTriangle);

void LoadWeaponCollisionConfig();
#endif

class AllMenuEventHandler : public BSTEventSink <MenuOpenCloseEvent> {
public:
	virtual EventResult	ReceiveEvent(MenuOpenCloseEvent * evn, EventDispatcher<MenuOpenCloseEvent> * dispatcher);
};

extern AllMenuEventHandler menuEvent;

void MenuOpened(std::string name);

void MenuClosed(std::string name);





extern std::vector<std::string> AffectedNodeLines;
extern std::vector<std::string> ColliderNodeLines;



extern std::vector<ConfigLine> AffectedNodesList; //Nodes that can be collided with

extern std::vector<ConfigLine> ColliderNodesList; //Nodes that can collide nodes

void loadCollisionConfig();
void loadMasterConfig();
void loadExtraCollisionConfig();

void loadSystemConfig();

void ConfigLineSplitterSphere(std::string &line, Sphere &newSphere);
void ConfigLineSplitterCapsule(std::string &line, Capsule &newCapsule);

int GetConfigSettingsValue(std::string line, std::string &variable);
std::string GetConfigSettingsStringValue(std::string line, std::string& variable);
std::string GetConfigSettings2StringValues(std::string line, std::string& variable, std::string& value2);

float GetConfigSettingsFloatValue(std::string line, std::string &variable);

void printSpheresMessage(std::string message, std::vector<Sphere> spheres);

std::vector<std::string> ConfigLineVectorToStringVector(std::vector<ConfigLine> linesList);

//The basic unit is parallel processing, but some physics chain nodes need sequential loading
extern std::vector<std::vector<std::string>> affectedBones;

bool GetSpecificNPCConfigForActor(Actor * actor, SpecificNPCConfig &snc);

bool IsActorMale(Actor* actor);

bool ConditionCheck(Actor* actor, ConditionItem& condition);
bool CheckActorForConditions(Actor* actor, Conditions& conditions);

std::string GetActorNodeString(Actor* actor, BSFixedString nodeName);
std::string GetFormIdNodeString(UInt32 id, BSFixedString nodeName);


bool RegisterFuncs(VMClassRegistry* registry);
BSFixedString GetVersion(StaticFunctionTag* base);
BSFixedString GetVersionMinor(StaticFunctionTag* base);
BSFixedString GetVersionBeta(StaticFunctionTag* base);
void StartPhysics(StaticFunctionTag* base, Actor* actor, BSFixedString nodeName);
void StopPhysics(StaticFunctionTag* base, Actor* actor, BSFixedString nodeName);


extern std::string versionStr;
extern UInt32 version;


class TESEquipEventHandler : public BSTEventSink <TESEquipEvent>
{
public:
	virtual	EventResult ReceiveEvent(TESEquipEvent* evn, EventDispatcher<TESEquipEvent>* dispatcher);
};

extern EventDispatcher<TESEquipEvent>* g_TESEquipEventDispatcher;
extern TESEquipEventHandler g_TESEquipEventHandler;


