#include "config.h"
#include <skse64/PapyrusActor.cpp>
#include <skse64/PapyrusGame.cpp>
#include <common/ICriticalSection.h>


std::string versionStr = "1.5.0";
UInt32 version = 0x010500;


#pragma warning(disable : 4996)
#ifdef RUNTIME_VR_VERSION_1_4_15
std::vector<std::string> PlayerNodeLines;
std::vector<ConfigLine> PlayerNodesList;   //Player nodes that can collide nodes
std::vector<WeaponConfigLine> WeaponCollidersList; //Weapon colliders
float MeleeWeaponTranslateX = 0.0;
float MeleeWeaponTranslateY = -5.0;
float MeleeWeaponTranslateZ = 0.0;

unsigned short hapticFrequency = 15;
int hapticStrength = 1;
bool leftHandedMode = false;
#endif
//std::vector<std::string> extraRacesList;

std::vector<std::string> AffectedNodeLines;
std::vector<std::string> ColliderNodeLines;



std::vector<ConfigLine> AffectedNodesList; //Nodes that can be collided with

std::vector<ConfigLine> ColliderNodesList; //Nodes that can collide nodes

//The basic unit is parallel processing, but some physics chain nodes need sequential loading
std::vector<std::vector<std::string>> affectedBones;



std::vector<SpecificNPCConfig> specificNPCConfigList;



std::vector<SpecificNPCBounceConfig> specificNPCBounceConfigList;


std::unordered_map<std::string, bool> ActorNodeStoppedPhysicsMap;

//std::unordered_map<Actor*, nodeCollisionMap> LeftBreastCollisionMap;
//std::unordered_map<Actor*, nodeCollisionMap> RightBreastCollisionMap;
//
//std::unordered_map<Actor*, nodeCollisionAmountMap> BreastCollisionAmountMap;

int configReloadCount = 0;
int gridsize = 25;
int adjacencyValue = 5;
int tuningModeCollision = 0;
int malePhysics = 0;
//int malePhysicsOnlyForExtraRaces = 0;

float actorDistance = 4194304.0f;
float actorBounceDistance = 16777216.0f;
int inCombatActorCount = 10;
int outOfCombatActorCount = 30;

int actorAngle = 180;

float cbellybulge = 2;
float cbellybulgemax = 10;
float cbellybulgeposlowest = -12;

float bellyBulgeReturnTime = 1.5f;
std::vector<std::string> bellybulgenodesList;

float vaginaOpeningLimit = 5.0f;
float vaginaOpeningMultiplier = 4.0f;

int logging = 0;

int useCamera = 1;

int fpsCorrectionEnabled = 0;

std::vector<std::string> noJitterFixNodesList = {
"NPC Genitals01 [Gen01]",
"NPC Genitals02 [Gen02]",
"NPC Genitals03 [Gen03]",
"NPC Genitals04 [Gen04]",
"NPC Genitals05 [Gen05]",
"NPC Genitals06 [Gen06]"
};


//int useOldHook = 1;

std::atomic<bool> dialogueMenuOpen = false;
std::atomic<bool> raceSexMenuClosed = false;
std::atomic<bool> raceSexMenuOpen = false;
std::atomic<bool> MainMenuOpen = false;
std::atomic<bool> consoleConfigReload = false;
std::atomic<bool> consoleCollisionReload = false;

std::unordered_map<std::string, std::string> configMap;
std::unordered_map<std::string, Conditions> nodeConditionsMap;
config_t config;
config_t config0weight;

int collisionSkipFrames = 0; //0
int collisionSkipFramesPelvis = 5; //5


std::string breastGravityReferenceBoneName = "NPC Spine2 [Spn2]";
BSFixedString breastGravityReferenceBoneString;

std::string GroundReferenceBoneName = "NPC Root [Root]";
BSFixedString GroundReferenceBone;

UInt32 KeywordArmorLightFormId = 0x06BBD3;
BGSKeyword* KeywordArmorLight;

UInt32 KeywordArmorHeavyFormId = 0x06BBD2;
BGSKeyword* KeywordArmorHeavy;

UInt32 KeywordArmorClothingFormId = 0x06BBE8;
BGSKeyword* KeywordArmorClothing;

UInt32 KeywordActorTypeNPCFormId = 0x0013794;
BGSKeyword* KeywordActorTypeNPC;

BSFixedString KeywordNameAsNakedL = "CBPCAsNakedL";

BSFixedString KeywordNameAsNakedR = "CBPCAsNakedR";

BSFixedString KeywordNameAsClothingL = "CBPCAsClothingL";

BSFixedString KeywordNameAsClothingR = "CBPCAsClothingR";

BSFixedString KeywordNameAsLightL = "CBPCAsLightL";

BSFixedString KeywordNameAsLightR = "CBPCAsLightR";

BSFixedString KeywordNameAsHeavyL = "CBPCAsHeavyL";

BSFixedString KeywordNameAsHeavyR = "CBPCAsHeavyR";

BSFixedString KeywordNameNoPushUpL = "CBPCNoPushUpL";

BSFixedString KeywordNameNoPushUpR = "CBPCNoPushUpR";

UInt32 VampireLordBeastRaceFormId = 0x0200283A;

bool compareConfigs(const SpecificNPCConfig& config1, const SpecificNPCConfig& config2)
{
	return config1.ConditionPriority > config2.ConditionPriority
		|| (config1.ConditionPriority == config2.ConditionPriority && config1.conditions.AndItems.size() > config2.conditions.AndItems.size());
}

bool compareBounceConfigs(const SpecificNPCBounceConfig& config1, const SpecificNPCBounceConfig& config2)
{
	return config1.ConditionPriority > config2.ConditionPriority
		|| (config1.ConditionPriority == config2.ConditionPriority && config1.conditions.AndItems.size() > config2.conditions.AndItems.size());
}

UInt8 GetLoadedModIndex(const char* modName)
{
	DataHandler* dataHandler = DataHandler::GetSingleton();
	if (dataHandler != nullptr)
	{
		const ModInfo* modInfo = dataHandler->LookupModByName(modName);
		if (!modInfo)
			return 0xFF;

		#ifdef RUNTIME_VERSION_1_5_73
		if(!modInfo->IsActive())
			return 0xFF;
		#endif

		#ifndef RUNTIME_VR_VERSION_1_4_15
			#ifdef RUNTIME_VERSION_1_5_73
		if (modInfo->IsLight())
			return modInfo->lightIndex + papyrusGame::LIGHT_MOD_OFFSET;
			#endif
		#endif

		return modInfo->modIndex;
	}

	return -1;
}

UInt32 GetFormIdFromString(std::string str)
{
	std::transform(str.begin(), str.end(), str.begin(), ::tolower);

	trim(str);

	std::vector<std::string> splittedByPlugin = split(str, '|');

	std::string pluginNumber = "";
	int variableIndex = 0;

	if (splittedByPlugin.size() > 1)
	{
		DataHandler* dataHandler = DataHandler::GetSingleton();
		UInt8 modIndex = GetLoadedModIndex(splittedByPlugin[0].c_str());

		if (modIndex != 255)
		{
			pluginNumber = num2hex(modIndex, 2);
		}
		variableIndex = 1;
	}

	std::string var = splittedByPlugin[variableIndex];
		
	var = pluginNumber + var;
	UInt32 formId = getHex(var.c_str());

	return formId;
}

void DecideConditionType(std::string line, UInt32& id, std::string& str, ConditionType& type)
{
	std::vector<std::string> splittedMain = splitMulti(line, "()[]{}");

	if (splittedMain.size() > 0)
	{
		if (splittedMain[0] == "IsRaceFormId")
		{
			type = ConditionType::IsRaceFormId;
			if (splittedMain.size() > 1)
			{
				id = GetFormIdFromString(splittedMain[1]);
			}
		}
		else if (splittedMain[0] == "IsRaceName")
		{
			type = ConditionType::IsRaceName;
			if (splittedMain.size() > 1)
			{
				str = splittedMain[1];
			}
		}
		else if (splittedMain[0] == "ActorName")
		{
			type = ConditionType::ActorName;
			if (splittedMain.size() > 1)
			{
				str = splittedMain[1];
			}
		}
		else if (splittedMain[0] == "ActorFormId")
		{
			type = ConditionType::ActorFormId;
			if (splittedMain.size() > 1)
			{
				id = GetFormIdFromString(splittedMain[1]);
			}
		}
		else if (splittedMain[0] == "IsInFaction")
		{
			type = ConditionType::IsInFaction;
			if (splittedMain.size() > 1)
			{
				id = GetFormIdFromString(splittedMain[1]);
			}
		}
		else if (splittedMain[0] == "IsPlayerTeammate")
		{
			type = ConditionType::IsPlayerTeammate;
		}
		else if (splittedMain[0] == "IsFemale")
		{
			type = ConditionType::IsFemale;
		}
		else if (splittedMain[0] == "IsMale")
		{
			type = ConditionType::IsMale;
		}
		else if (splittedMain[0] == "IsPlayer")
		{
			type = ConditionType::IsPlayer;
		}
		else if (splittedMain[0] == "HasKeywordId")
		{
			type = ConditionType::HasKeywordId;
			if (splittedMain.size() > 1)
			{
				id = GetFormIdFromString(splittedMain[1]);
			}
		}
		else if (splittedMain[0] == "HasKeywordName")
		{
			type = ConditionType::HasKeywordName;
			if (splittedMain.size() > 1)
			{
				str = splittedMain[1];
			}
		}
		else if (splittedMain[0] == "RaceHasKeywordId")
		{
			type = ConditionType::RaceHasKeywordId;
			if (splittedMain.size() > 1)
			{
				id = GetFormIdFromString(splittedMain[1]);
			}
		}
		else if (splittedMain[0] == "RaceHasKeywordName")
		{
			type = ConditionType::RaceHasKeywordName;
			if (splittedMain.size() > 1)
			{
				str = splittedMain[1];
			}
		}
		else if (splittedMain[0] == "IsActorBase")
		{
			type = ConditionType::IsActorBase;
			if (splittedMain.size() > 1)
			{
				id = GetFormIdFromString(splittedMain[1]);
			}
		}
		else if (splittedMain[0] == "IsUnique")
		{
			type = ConditionType::IsUnique;
		}
		else if (splittedMain[0] == "IsVoiceType")
		{
			type = ConditionType::IsVoiceType;
			if (splittedMain.size() > 1)
			{
				id = GetFormIdFromString(splittedMain[1]);
			}
		}
		else if (splittedMain[0] == "IsCombatStyle")
		{
			type = ConditionType::IsCombatStyle;
			if (splittedMain.size() > 1)
			{
				id = GetFormIdFromString(splittedMain[1]);
			}
		}
		else if (splittedMain[0] == "IsClass")
		{
			type = ConditionType::IsClass;
			if (splittedMain.size() > 1)
			{
				id = GetFormIdFromString(splittedMain[1]);
			}
		}
	}
}

Conditions ParseConditions(std::string& str)
{
	Conditions newConditions;

	std::vector<std::string> splittedANDs = split(str, "AND");

	/*LOG("- %s AND splitted:", str.c_str());
	for each (std::string var in splittedANDs)
	{
		LOG("%s", var.c_str());
	}*/

	for (auto& strAnd : splittedANDs)
	{
		if (Contains(strAnd, " OR "))
		{
			std::vector<std::string> splittedORs = split(strAnd, "OR");

			/*LOG("- %s OR splitted:", strAnd.c_str());
			for each (std::string var in splittedORs)
			{
				LOG("%s", var.c_str());
			}*/

			ConditionItem cItem;

			for (auto& strOr : splittedORs)
			{
				ConditionItem oItem;
				if (stringStartsWith(strOr, "not "))
				{
					oItem.Not = true;

					strOr.erase(0, 3);

					trim(strOr);
				}

				oItem.single = true;
				DecideConditionType(strOr, oItem.id, oItem.str, oItem.type);

				cItem.OrItems.emplace_back(oItem);
			}
			cItem.single = false;

			newConditions.AndItems.emplace_back(cItem);
		}
		else
		{
			ConditionItem cItem;
			if (stringStartsWith(strAnd, "not "))
			{
				cItem.Not = true;

				strAnd.erase(0, 3);

				trim(strAnd);
			}

			cItem.single = true;
			DecideConditionType(strAnd, cItem.id, cItem.str, cItem.type);

			newConditions.AndItems.emplace_back(cItem);
		}
	}

	return newConditions;
}

void loadConfig() {
	//logger.info("loadConfig\n");

	//Console_Print("Reading CBP Config");
	std::string	runtimeDirectory = GetRuntimeDirectory();
	if (!runtimeDirectory.empty())
	{
		std::string configPath = runtimeDirectory + "Data\\SKSE\\Plugins\\";

		config.clear();
		config0weight.clear();
		specificNPCBounceConfigList.clear();

		//Set default values
		for (auto& it : configMap)
		{
			//100 weight
			config[it.second]["stiffness"] = 0.0f;
			config[it.second]["stiffness2"] = 0.0f;
			config[it.second]["damping"] = 0.0f;
			config[it.second]["maxoffset"] = 0.0f;
			config[it.second]["Xmaxoffset"] = 5.0f;
			config[it.second]["Xminoffset"] = -5.0f;
			config[it.second]["Ymaxoffset"] = 5.0f;
			config[it.second]["Yminoffset"] = -5.0f;
			config[it.second]["Zmaxoffset"] = 5.0f;
			config[it.second]["Zminoffset"] = -5.0f;
			config[it.second]["Xdefaultoffset"] = 0.0f;
			config[it.second]["Ydefaultoffset"] = 0.0f;
			config[it.second]["Zdefaultoffset"] = 0.0f;
			config[it.second]["cogOffset"] = 0.0f;
			config[it.second]["gravityBias"] = 0.0f;
			config[it.second]["gravityCorrection"] = 0.0f;
			config[it.second]["timetick"] = 4.0f;
			config[it.second]["linearX"] = 0.0f;
			config[it.second]["linearY"] = 0.0f;
			config[it.second]["linearZ"] = 0.0f;
			config[it.second]["rotationalX"] = 0.0f;
			config[it.second]["rotationalY"] = 0.0f;
			config[it.second]["rotationalZ"] = 0.0f;
			config[it.second]["linearXrotationX"] = 0.0f;
			config[it.second]["linearXrotationY"] = 1.0f;
			config[it.second]["linearXrotationZ"] = 0.0f;
			config[it.second]["linearYrotationX"] = 0.0f;
			config[it.second]["linearYrotationY"] = 0.0f;
			config[it.second]["linearYrotationZ"] = 1.0f;
			config[it.second]["linearZrotationX"] = 1.0f;
			config[it.second]["linearZrotationY"] = 0.0f;
			config[it.second]["linearZrotationZ"] = 0.0f;
			config[it.second]["timeStep"] = 1.0f;
			config[it.second]["linearXspreadforceY"] = 0.0f;
			config[it.second]["linearXspreadforceZ"] = 0.0f;
			config[it.second]["linearYspreadforceX"] = 0.0f;
			config[it.second]["linearYspreadforceZ"] = 0.0f;
			config[it.second]["linearZspreadforceX"] = 0.0f;
			config[it.second]["linearZspreadforceY"] = 0.0f;
			config[it.second]["forceMultipler"] = 1.0f;
			config[it.second]["gravityInvertedCorrection"] = 0.0f;
			config[it.second]["gravityInvertedCorrectionStart"] = 0.0f;
			config[it.second]["gravityInvertedCorrectionEnd"] = 0.0f;
			config[it.second]["breastClothedPushup"] = 0.0f;
			config[it.second]["breastLightArmoredPushup"] = 0.0f;
			config[it.second]["breastHeavyArmoredPushup"] = 0.0f;
			config[it.second]["breastClothedAmplitude"] = 1.0f;
			config[it.second]["breastLightArmoredAmplitude"] = 1.0f;
			config[it.second]["breastHeavyArmoredAmplitude"] = 1.0f;
			config[it.second]["collisionFriction"] = 0.2f;
			config[it.second]["collisionPenetration"] = 0.0f;
			config[it.second]["collisionMultipler"] = 1.0f;
			config[it.second]["collisionMultiplerRot"] = 1.0f;
			config[it.second]["collisionElastic"] = 0.0f;
			config[it.second]["collisionXmaxoffset"] = 100.0f;
			config[it.second]["collisionXminoffset"] = -100.0f;
			config[it.second]["collisionYmaxoffset"] = 100.0f;
			config[it.second]["collisionYminoffset"] = -100.0f;
			config[it.second]["collisionZmaxoffset"] = 100.0f;
			config[it.second]["collisionZminoffset"] = -100.0f;

			//0 weight
			config0weight[it.second]["stiffness"] = 0.0f;
			config0weight[it.second]["stiffness2"] = 0.0f;
			config0weight[it.second]["damping"] = 0.0f;
			config0weight[it.second]["maxoffset"] = 0.0f;
			config0weight[it.second]["Xmaxoffset"] = 5.0f;
			config0weight[it.second]["Xminoffset"] = -5.0f;
			config0weight[it.second]["Ymaxoffset"] = 5.0f;
			config0weight[it.second]["Yminoffset"] = -5.0f;
			config0weight[it.second]["Zmaxoffset"] = 5.0f;
			config0weight[it.second]["Zminoffset"] = -5.0f;
			config0weight[it.second]["Xdefaultoffset"] = 0.0f;
			config0weight[it.second]["Ydefaultoffset"] = 0.0f;
			config0weight[it.second]["Zdefaultoffset"] = 0.0f;
			config0weight[it.second]["cogOffset"] = 0.0f;
			config0weight[it.second]["gravityBias"] = 0.0f; 
			config0weight[it.second]["gravityCorrection"] = 0.0f;
			config0weight[it.second]["timetick"] = 4.0f;
			config0weight[it.second]["linearX"] = 0.0f;
			config0weight[it.second]["linearY"] = 0.0f;
			config0weight[it.second]["linearZ"] = 0.0f;
			config0weight[it.second]["rotationalX"] = 0.0f;
			config0weight[it.second]["rotationalY"] = 0.0f;
			config0weight[it.second]["rotationalZ"] = 0.0f;
			config0weight[it.second]["linearXrotationX"] = 0.0f;
			config0weight[it.second]["linearXrotationY"] = 1.0f;
			config0weight[it.second]["linearXrotationZ"] = 0.0f;
			config0weight[it.second]["linearYrotationX"] = 0.0f;
			config0weight[it.second]["linearYrotationY"] = 0.0f;
			config0weight[it.second]["linearYrotationZ"] = 1.0f;
			config0weight[it.second]["linearZrotationX"] = 1.0f;
			config0weight[it.second]["linearZrotationY"] = 0.0f;
			config0weight[it.second]["linearZrotationZ"] = 0.0f;
			config0weight[it.second]["timeStep"] = 1.0f;
			config0weight[it.second]["linearXspreadforceY"] = 0.0f;
			config0weight[it.second]["linearXspreadforceZ"] = 0.0f;
			config0weight[it.second]["linearYspreadforceX"] = 0.0f;
			config0weight[it.second]["linearYspreadforceZ"] = 0.0f;
			config0weight[it.second]["linearZspreadforceX"] = 0.0f;
			config0weight[it.second]["linearZspreadforceY"] = 0.0f;
			config0weight[it.second]["forceMultipler"] = 1.0f;
			config0weight[it.second]["gravityInvertedCorrection"] = 0.0f;
			config0weight[it.second]["gravityInvertedCorrectionStart"] = 0.0f;
			config0weight[it.second]["gravityInvertedCorrectionEnd"] = 0.0f;
			config0weight[it.second]["breastClothedPushup"] = 0.0f;
			config0weight[it.second]["breastLightArmoredPushup"] = 0.0f;
			config0weight[it.second]["breastHeavyArmoredPushup"] = 0.0f;
			config0weight[it.second]["breastClothedAmplitude"] = 1.0f;
			config0weight[it.second]["breastLightArmoredAmplitude"] = 1.0f;
			config0weight[it.second]["breastHeavyArmoredAmplitude"] = 1.0f;
			config0weight[it.second]["collisionFriction"] = 0.2f;
			config0weight[it.second]["collisionPenetration"] = 0.0f;
			config0weight[it.second]["collisionMultipler"] = 1.0f;
			config0weight[it.second]["collisionMultiplerRot"] = 1.0f;
			config0weight[it.second]["collisionElastic"] = 0.0f;
			config0weight[it.second]["collisionXmaxoffset"] = 100.0f;
			config0weight[it.second]["collisionXminoffset"] = -100.0f;
			config0weight[it.second]["collisionYmaxoffset"] = 100.0f;
			config0weight[it.second]["collisionYminoffset"] = -100.0f;
			config0weight[it.second]["collisionZmaxoffset"] = 100.0f;
			config0weight[it.second]["collisionZminoffset"] = -100.0f;
		}

		auto configList = get_all_files_names_within_folder(configPath.c_str());
		bool configOpened = false;
		for (std::size_t i = 0; i < configList.size(); i++)
		{
			std::string filename = configList.at(i);

			if (filename == "." || filename == "..")
				continue;
					
			if (stringStartsWith(filename, "cbpconfig") && (stringEndsWith(filename, ".txt") || stringEndsWith(filename, ".ini")))
			{
				std::string msg = "File found: " + filename;
				LOG(msg.c_str());

				std::string filepath = configPath;
				filepath.append(filename);
				std::ifstream file(filepath);

				if (!file.is_open())
				{
					transform(filepath.begin(), filepath.end(), filepath.begin(), ::tolower);
					file.open(filepath);
				}

				configOpened = true;

				SpecificNPCBounceConfig newNPCBounceConfig;

				bool withConditions = false;

				std::string conditions;
				if (file.is_open())
				{
					std::string line;
					std::string currentSetting;
					while (std::getline(file, line))
					{
						//trim(line);
						skipComments(line);
						trim(line);
						if (line.length() > 0)
						{
							if (Contains(line, "="))
							{
								std::string variableName;
								std::string variableValue = GetConfigSettingsStringValue(line, variableName);
								if (variableName == "Conditions")
								{
									withConditions = true;
									conditions = variableValue;
									LOG("Conditioned bounce config: %s", conditions.c_str());

									if (withConditions)
									{
										//Set default values
										for (auto& it : configMap)
										{
											//100 weight
											newNPCBounceConfig.config[it.second]["stiffness"] = 0.0f;
											newNPCBounceConfig.config[it.second]["stiffness2"] = 0.0f;
											newNPCBounceConfig.config[it.second]["damping"] = 0.0f;
											newNPCBounceConfig.config[it.second]["maxoffset"] = 0.0f;
											newNPCBounceConfig.config[it.second]["Xmaxoffset"] = 5.0f;
											newNPCBounceConfig.config[it.second]["Xminoffset"] = -5.0f;
											newNPCBounceConfig.config[it.second]["Ymaxoffset"] = 5.0f;
											newNPCBounceConfig.config[it.second]["Yminoffset"] = -5.0f;
											newNPCBounceConfig.config[it.second]["Zmaxoffset"] = 5.0f;
											newNPCBounceConfig.config[it.second]["Zminoffset"] = -5.0f;
											newNPCBounceConfig.config[it.second]["Xdefaultoffset"] = 0.0f;
											newNPCBounceConfig.config[it.second]["Ydefaultoffset"] = 0.0f;
											newNPCBounceConfig.config[it.second]["Zdefaultoffset"] = 0.0f;
											newNPCBounceConfig.config[it.second]["cogOffset"] = 0.0f;
											newNPCBounceConfig.config[it.second]["gravityBias"] = 0.0f;
											newNPCBounceConfig.config[it.second]["gravityCorrection"] = 0.0f;
											newNPCBounceConfig.config[it.second]["timetick"] = 4.0f;
											newNPCBounceConfig.config[it.second]["linearX"] = 0.0f;
											newNPCBounceConfig.config[it.second]["linearY"] = 0.0f;
											newNPCBounceConfig.config[it.second]["linearZ"] = 0.0f;
											newNPCBounceConfig.config[it.second]["rotationalX"] = 0.0f;
											newNPCBounceConfig.config[it.second]["rotationalY"] = 0.0f;
											newNPCBounceConfig.config[it.second]["rotationalZ"] = 0.0f;
											newNPCBounceConfig.config[it.second]["linearXrotationX"] = 0.0f;
											newNPCBounceConfig.config[it.second]["linearXrotationY"] = 1.0f;
											newNPCBounceConfig.config[it.second]["linearXrotationZ"] = 0.0f;
											newNPCBounceConfig.config[it.second]["linearYrotationX"] = 0.0f;
											newNPCBounceConfig.config[it.second]["linearYrotationY"] = 0.0f;
											newNPCBounceConfig.config[it.second]["linearYrotationZ"] = 1.0f;
											newNPCBounceConfig.config[it.second]["linearZrotationX"] = 1.0f;
											newNPCBounceConfig.config[it.second]["linearZrotationY"] = 0.0f;
											newNPCBounceConfig.config[it.second]["linearZrotationZ"] = 0.0f;
											newNPCBounceConfig.config[it.second]["timeStep"] = 1.0f;
											newNPCBounceConfig.config[it.second]["linearXspreadforceY"] = 0.0f;
											newNPCBounceConfig.config[it.second]["linearXspreadforceZ"] = 0.0f;
											newNPCBounceConfig.config[it.second]["linearYspreadforceX"] = 0.0f;
											newNPCBounceConfig.config[it.second]["linearYspreadforceZ"] = 0.0f;
											newNPCBounceConfig.config[it.second]["linearZspreadforceX"] = 0.0f;
											newNPCBounceConfig.config[it.second]["linearZspreadforceY"] = 0.0f;
											newNPCBounceConfig.config[it.second]["forceMultipler"] = 1.0f;
											newNPCBounceConfig.config[it.second]["gravityInvertedCorrection"] = 0.0f;
											newNPCBounceConfig.config[it.second]["gravityInvertedCorrectionStart"] = 0.0f;
											newNPCBounceConfig.config[it.second]["gravityInvertedCorrectionEnd"] = 0.0f;
											newNPCBounceConfig.config[it.second]["breastClothedPushup"] = 0.0f;
											newNPCBounceConfig.config[it.second]["breastLightArmoredPushup"] = 0.0f;
											newNPCBounceConfig.config[it.second]["breastHeavyArmoredPushup"] = 0.0f;
											newNPCBounceConfig.config[it.second]["breastClothedAmplitude"] = 1.0f;
											newNPCBounceConfig.config[it.second]["breastLightArmoredAmplitude"] = 1.0f;
											newNPCBounceConfig.config[it.second]["breastHeavyArmoredAmplitude"] = 1.0f;
											newNPCBounceConfig.config[it.second]["collisionFriction"] = 0.2f;
											newNPCBounceConfig.config[it.second]["collisionPenetration"] = 0.0f;
											newNPCBounceConfig.config[it.second]["collisionMultipler"] = 1.0f;
											newNPCBounceConfig.config[it.second]["collisionMultiplerRot"] = 1.0f;
											newNPCBounceConfig.config[it.second]["collisionElastic"] = 0.0f;
											newNPCBounceConfig.config[it.second]["collisionXmaxoffset"] = 100.0f;
											newNPCBounceConfig.config[it.second]["collisionXminoffset"] = -100.0f;
											newNPCBounceConfig.config[it.second]["collisionYmaxoffset"] = 100.0f;
											newNPCBounceConfig.config[it.second]["collisionYminoffset"] = -100.0f;
											newNPCBounceConfig.config[it.second]["collisionZmaxoffset"] = 100.0f;
											newNPCBounceConfig.config[it.second]["collisionZminoffset"] = -100.0f;

											//0 weight
											newNPCBounceConfig.config0weight[it.second]["stiffness"] = 0.0f;
											newNPCBounceConfig.config0weight[it.second]["stiffness2"] = 0.0f;
											newNPCBounceConfig.config0weight[it.second]["damping"] = 0.0f;
											newNPCBounceConfig.config0weight[it.second]["maxoffset"] = 0.0f;
											newNPCBounceConfig.config0weight[it.second]["Xmaxoffset"] = 5.0f;
											newNPCBounceConfig.config0weight[it.second]["Xminoffset"] = -5.0f;
											newNPCBounceConfig.config0weight[it.second]["Ymaxoffset"] = 5.0f;
											newNPCBounceConfig.config0weight[it.second]["Yminoffset"] = -5.0f;
											newNPCBounceConfig.config0weight[it.second]["Zmaxoffset"] = 5.0f;
											newNPCBounceConfig.config0weight[it.second]["Zminoffset"] = -5.0f;
											newNPCBounceConfig.config0weight[it.second]["Xdefaultoffset"] = 0.0f;
											newNPCBounceConfig.config0weight[it.second]["Ydefaultoffset"] = 0.0f;
											newNPCBounceConfig.config0weight[it.second]["Zdefaultoffset"] = 0.0f;
											newNPCBounceConfig.config0weight[it.second]["cogOffset"] = 0.0f;
											newNPCBounceConfig.config0weight[it.second]["gravityBias"] = 0.0f;
											newNPCBounceConfig.config0weight[it.second]["gravityCorrection"] = 0.0f;
											newNPCBounceConfig.config0weight[it.second]["timetick"] = 4.0f;
											newNPCBounceConfig.config0weight[it.second]["linearX"] = 0.0f;
											newNPCBounceConfig.config0weight[it.second]["linearY"] = 0.0f;
											newNPCBounceConfig.config0weight[it.second]["linearZ"] = 0.0f;
											newNPCBounceConfig.config0weight[it.second]["rotationalX"] = 0.0f;
											newNPCBounceConfig.config0weight[it.second]["rotationalY"] = 0.0f;
											newNPCBounceConfig.config0weight[it.second]["rotationalZ"] = 0.0f;
											newNPCBounceConfig.config0weight[it.second]["linearXrotationX"] = 0.0f;
											newNPCBounceConfig.config0weight[it.second]["linearXrotationY"] = 1.0f;
											newNPCBounceConfig.config0weight[it.second]["linearXrotationZ"] = 0.0f;
											newNPCBounceConfig.config0weight[it.second]["linearYrotationX"] = 0.0f;
											newNPCBounceConfig.config0weight[it.second]["linearYrotationY"] = 0.0f;
											newNPCBounceConfig.config0weight[it.second]["linearYrotationZ"] = 1.0f;
											newNPCBounceConfig.config0weight[it.second]["linearZrotationX"] = 1.0f;
											newNPCBounceConfig.config0weight[it.second]["linearZrotationY"] = 0.0f;
											newNPCBounceConfig.config0weight[it.second]["linearZrotationZ"] = 0.0f;
											newNPCBounceConfig.config0weight[it.second]["timeStep"] = 1.0f;
											newNPCBounceConfig.config0weight[it.second]["linearXspreadforceY"] = 0.0f;
											newNPCBounceConfig.config0weight[it.second]["linearXspreadforceZ"] = 0.0f;
											newNPCBounceConfig.config0weight[it.second]["linearYspreadforceX"] = 0.0f;
											newNPCBounceConfig.config0weight[it.second]["linearYspreadforceZ"] = 0.0f;
											newNPCBounceConfig.config0weight[it.second]["linearZspreadforceX"] = 0.0f;
											newNPCBounceConfig.config0weight[it.second]["linearZspreadforceY"] = 0.0f;
											newNPCBounceConfig.config0weight[it.second]["forceMultipler"] = 1.0f;
											newNPCBounceConfig.config0weight[it.second]["gravityInvertedCorrection"] = 0.0f;
											newNPCBounceConfig.config0weight[it.second]["gravityInvertedCorrectionStart"] = 0.0f;
											newNPCBounceConfig.config0weight[it.second]["gravityInvertedCorrectionEnd"] = 0.0f;
											newNPCBounceConfig.config0weight[it.second]["breastClothedPushup"] = 0.0f;
											newNPCBounceConfig.config0weight[it.second]["breastLightArmoredPushup"] = 0.0f;
											newNPCBounceConfig.config0weight[it.second]["breastHeavyArmoredPushup"] = 0.0f;
											newNPCBounceConfig.config0weight[it.second]["breastClothedAmplitude"] = 1.0f;
											newNPCBounceConfig.config0weight[it.second]["breastLightArmoredAmplitude"] = 1.0f;
											newNPCBounceConfig.config0weight[it.second]["breastHeavyArmoredAmplitude"] = 1.0f;
											newNPCBounceConfig.config0weight[it.second]["collisionFriction"] = 0.2f;
											newNPCBounceConfig.config0weight[it.second]["collisionPenetration"] = 0.0f;
											newNPCBounceConfig.config0weight[it.second]["collisionMultipler"] = 1.0f;
											newNPCBounceConfig.config0weight[it.second]["collisionMultiplerRot"] = 1.0f;
											newNPCBounceConfig.config0weight[it.second]["collisionElastic"] = 0.0f;
											newNPCBounceConfig.config0weight[it.second]["collisionXmaxoffset"] = 100.0f;
											newNPCBounceConfig.config0weight[it.second]["collisionXminoffset"] = -100.0f;
											newNPCBounceConfig.config0weight[it.second]["collisionYmaxoffset"] = 100.0f;
											newNPCBounceConfig.config0weight[it.second]["collisionYminoffset"] = -100.0f;
											newNPCBounceConfig.config0weight[it.second]["collisionZmaxoffset"] = 100.0f;
											newNPCBounceConfig.config0weight[it.second]["collisionZminoffset"] = -100.0f;
										}
									}
								}
								else if (variableName == "Priority")
								{
									newNPCBounceConfig.ConditionPriority = GetConfigSettingsValue(line, variableName);
								}
							}
							else
							{
								std::vector<std::string> splittedMain = split(line, '.');

								if (splittedMain.size() > 1)
								{
									std::string tok0 = splittedMain[0];

									if (i != std::string::npos)
										line.erase(0, tok0.length()+1);

									std::vector<std::string> splitted = splitNonEmpty(line, ' ');

									std::string tok1 = "";
									std::string tok2 = "";
									std::string tok3 = "";
									if (splitted.size() > 0)
									{
										tok1 = splitted[0];

										if (splitted.size() > 1)
										{
											tok2 = splitted[1];

											if (splitted.size() > 2)
											{
												tok3 = splitted[2];
											}
										}
									}									

									if (withConditions)
									{
										if (tok0.size() > 0 && tok1.size() > 0 && tok2.size() > 0) {
											const float calcValue = atof(tok2.c_str());
											newNPCBounceConfig.config[tok0][tok1] = calcValue;
											newNPCBounceConfig.config0weight[tok0][tok1] = calcValue;
											LOG("Conditioned config[%s][%s] = %s", tok0.c_str(), tok1.c_str(), tok2.c_str());
										}
										if (tok0.size() > 0 && tok1.size() > 0 && tok3.size() > 0) {
											newNPCBounceConfig.config0weight[tok0][tok1] = atof(tok3.c_str());
											LOG("Conditioned 0 weight config[%s][%s] = %s", tok0.c_str(), tok1.c_str(), tok3.c_str());
										}
									}
									else
									{
										if (tok0.size() > 0 && tok1.size() > 0 && tok2.size() > 0) {
											const float calcValue = atof(tok2.c_str());
											config[tok0][tok1] = calcValue;
											config0weight[tok0][tok1] = calcValue;
											LOG("config[%s][%s] = %s", tok0.c_str(), tok1.c_str(), tok2.c_str());
										}
										if (tok0.size() > 0 && tok1.size() > 0 && tok3.size() > 0) {
											config0weight[tok0][tok1] = atof(tok3.c_str());
											LOG("0 weight config[%s][%s] = %s", tok0.c_str(), tok1.c_str(), tok3.c_str());
										}
									}
								}
							}
						}
					}
				}

				if (withConditions)
				{					
					newNPCBounceConfig.conditions = ParseConditions(conditions);
					specificNPCBounceConfigList.emplace_back(newNPCBounceConfig);
				}
			}
		}

		if (specificNPCBounceConfigList.size() > 1)
		{
			std::sort(specificNPCBounceConfigList.begin(), specificNPCBounceConfigList.end(), compareBounceConfigs);
		}

		if (!configOpened)
			configReloadCount = 0;

		configReloadCount = config["Tuning"]["rate"];
	}
}

void loadSystemConfig()
{
	std::string	runtimeDirectory = GetRuntimeDirectory();

	if (!runtimeDirectory.empty())
	{
		std::string filepath = runtimeDirectory + "Data\\SKSE\\Plugins\\CBPCSystem.ini";

		std::ifstream file(filepath);

		if (!file.is_open())
		{
			transform(filepath.begin(), filepath.end(), filepath.begin(), ::tolower);
			file.open(filepath);
		}

		if (file.is_open())
		{
			std::string line;
			std::string currentSetting;
			while (std::getline(file, line))
			{
				//trim(line);
				skipComments(line);
				trim(line);
				if (line.length() > 0)
				{
					if (line.substr(0, 1) == "[")
					{
						//newsetting
						currentSetting = line;
					}
					else
					{
						std::string variableName;
						
						int variableValue = GetConfigSettingsValue(line, variableName);
						if (variableName == "SkipFrames")
							collisionSkipFrames = variableValue;
						else if (variableName == "SkipFramesPelvis")
							collisionSkipFramesPelvis = variableValue;
						else if (variableName == "FpsCorrection")
							fpsCorrectionEnabled = variableValue;
						else if (variableName == "GridSize")
							gridsize = variableValue;
						else if (variableName == "AdjacencyValue")
							adjacencyValue = variableValue;
						else if (variableName == "ActorDistance")
						{
							float variableFloatValue = GetConfigSettingsFloatValue(line, variableName);
							actorDistance = variableFloatValue*variableFloatValue;
						}
						else if (variableName == "ActorBounceDistance")
						{
							float variableFloatValue = GetConfigSettingsFloatValue(line, variableName);
							actorBounceDistance = variableFloatValue * variableFloatValue;
						}
						else if (variableName == "ActorAngle")
						{
							actorAngle = variableValue;
#ifdef RUNTIME_VR_VERSION_1_4_15	
							if (actorAngle < 180)
							{
								actorAngle = 180;
							}
#endif
						}
						else if (variableName == "UseCamera")
							useCamera = variableValue;
						else if (variableName == "Logging")
						{
							logging = variableValue;
						}
						else if (variableName == "InCombatActorCount")
						{
							inCombatActorCount = variableValue;
						}
						else if (variableName == "OutOfCombatActorCount")
						{
							outOfCombatActorCount = variableValue;
						}
#ifdef RUNTIME_VR_VERSION_1_4_15	
						else if (variableName == "HapticFrequency")
							hapticFrequency = variableValue;
						else if (variableName == "HapticStrength")
							hapticStrength = variableValue;
#endif
					}
				}
			}

			LOG_ERR("System Config file is loaded successfully.");
			return;
		}
	}

	LOG_ERR("System Config file is not loaded.");
}

// Converts the lower bits of a FormID to a full FormID depending on plugin type
UInt32 GetFullFormID(const ModInfo* modInfo, UInt32 formLower)
{
	return (modInfo->modIndex << 24) | formLower;
}

void GameLoad()
{
	dialogueMenuOpen.store(false);
	raceSexMenuClosed.store(false);

	MenuManager* menuManager = MenuManager::GetSingleton();
	if (menuManager)
		menuManager->MenuOpenCloseEventDispatcher()->AddEventSink(&menuEvent);

	TESForm* keywordForm = LookupFormByID(KeywordArmorLightFormId);
	if (keywordForm)
		KeywordArmorLight = DYNAMIC_CAST(keywordForm, TESForm, BGSKeyword);

	keywordForm = LookupFormByID(KeywordArmorHeavyFormId);
	if (keywordForm)
		KeywordArmorHeavy = DYNAMIC_CAST(keywordForm, TESForm, BGSKeyword);

	keywordForm = LookupFormByID(KeywordArmorClothingFormId);
	if (keywordForm)
		KeywordArmorClothing = DYNAMIC_CAST(keywordForm, TESForm, BGSKeyword);

	keywordForm = LookupFormByID(KeywordActorTypeNPCFormId);
	if (keywordForm)
		KeywordActorTypeNPC = DYNAMIC_CAST(keywordForm, TESForm, BGSKeyword);












	DataHandler* dataHandler = DataHandler::GetSingleton();
	if (dataHandler)
	{
		const ModInfo* DawnguardModInfo = dataHandler->LookupModByName("Dawnguard.esm");

		if (DawnguardModInfo)
		{
			if (IsValidModIndex(DawnguardModInfo->modIndex))
			{
				VampireLordBeastRaceFormId = GetFullFormID(DawnguardModInfo, GetBaseFormID(0x0283A));
			}
		}
	}

	breastGravityReferenceBoneString = ReturnUsableString(breastGravityReferenceBoneName);

	GroundReferenceBone = ReturnUsableString(GroundReferenceBoneName);
}

void loadMasterConfig()
{
	std::string	runtimeDirectory = GetRuntimeDirectory();

	if (!runtimeDirectory.empty())
	{
		configMap.clear();
		nodeConditionsMap.clear();
		affectedBones.clear();
		
		std::string filepath = runtimeDirectory + "Data\\SKSE\\Plugins\\CBPCMasterConfig.txt";

		std::ifstream file(filepath);

		if (!file.is_open())
		{
			transform(filepath.begin(), filepath.end(), filepath.begin(), ::tolower);
			file.open(filepath);
		}
		
		if (file.is_open())
		{
			std::string line;
			std::string currentSetting;
			bool isChain = false;
			std::vector<std::string> affectedBonesList;
			while (std::getline(file, line))
			{
				//trim(line);
				skipComments(line);
				trim(line);
				if (line.length() > 0)
				{
					if (line.substr(0, 1) == "[")
					{
						//newsetting
						currentSetting = line;
					}
					else
					{
						if (currentSetting == "[Settings]")
						{
							std::string variableName;							
							int variableValue = GetConfigSettingsValue(line, variableName);

							if (variableName == "TuningMode")
								tuningModeCollision = variableValue;
							else if (variableName == "MalePhysics")
								malePhysics = variableValue;
							else if (variableName == "NoJitterFixNodes")
							{
								std::string variableStrValue = GetConfigSettingsStringValue(line, variableName);
								noJitterFixNodesList = split(variableStrValue, ',');
							}
						}
						else if (currentSetting == "[ConfigMap]")
						{
							if (line.substr(0, 1) == "<")
							{
								isChain = true;
							}
							else if (line.substr(0, 1) == ">")
							{
								isChain = false;
								affectedBones.emplace_back(affectedBonesList);
								affectedBonesList.clear();
							}
							else
							{
								std::string variableName;
								std::string conditions;
								std::string variableValue = GetConfigSettings2StringValues(line, variableName, conditions);
								bool isFound = false;
								for (int i = 0; i < affectedBones.size(); i++)
								{
									if (std::find(affectedBones.at(i).begin(), affectedBones.at(i).end(), variableName) != affectedBones.at(i).end())
									{
										isFound = true;
									}
								}

								if (!isFound)
								{
									affectedBonesList.emplace_back(variableName);
									if (!isChain)
									{
										affectedBones.emplace_back(affectedBonesList);
										affectedBonesList.clear();
									}
								}

								if (variableValue != "")
								{
									configMap[variableName] = variableValue;
									LOG("ConfigMap[%s] = %s = %s", variableName.c_str(), variableValue.c_str(), conditions.c_str());
								}
								if (conditions != "")
								{
									nodeConditionsMap[variableName] = ParseConditions(conditions);
								}
							}
						}
					}
				}
			}

			LOG_ERR("Master Config file is loaded successfully.");
		}

		std::string configPath = runtimeDirectory + "Data\\SKSE\\Plugins\\";

		auto configList = get_all_files_names_within_folder(configPath.c_str());

		for (int i = 0; i < configList.size(); i++)
		{
			std::string filename = configList.at(i);

			if (filename == "." || filename == "..")
				continue;

			if (stringStartsWith(filename, "cbpcmasterconfig") && filename != "CBPCMasterConfig.txt" && (stringEndsWith(filename, ".txt") || stringEndsWith(filename, ".ini")))
			{
				std::string msg = "File found: " + filename;
				LOG(msg.c_str());

				std::string filepath = configPath;
				filepath.append(filename);

				std::ifstream file(filepath);

				if (file.is_open())
				{
					std::string line;
					std::string currentSetting;
					bool isChain = false;
					std::vector<std::string> affectedBonesList;
					while (std::getline(file, line))
					{
						//trim(line);
						skipComments(line);
						trim(line);
						if (line.length() > 0)
						{
							if (line.substr(0, 1) == "[")
							{
								//newsetting
								currentSetting = line;
							}
							else
							{
								if (currentSetting == "[Settings]")
								{
									std::string variableName;
									int variableValue = GetConfigSettingsValue(line, variableName);

									if (variableName == "NoJitterFixNodes")
									{
										std::string variableStrValue = GetConfigSettingsStringValue(line, variableName);
										std::vector<std::string> newNodesList = split(variableStrValue, ',');
										noJitterFixNodesList.insert(std::end(noJitterFixNodesList), std::begin(newNodesList), std::end(newNodesList));
									}
								}
								else if (currentSetting == "[ConfigMap]")
								{
									if (line.substr(0, 1) == "<")
									{
										isChain = true;
									}
									else if (line.substr(0, 1) == ">")
									{
										isChain = false;
										affectedBones.emplace_back(affectedBonesList);
										affectedBonesList.clear();
									}
									else
									{
										std::string variableName;
										std::string conditions;
										std::string variableValue = GetConfigSettings2StringValues(line, variableName, conditions);
										bool isFound = false;
										for (int i = 0; i < affectedBones.size(); i++)
										{
											if (std::find(affectedBones.at(i).begin(), affectedBones.at(i).end(), variableName) != affectedBones.at(i).end())
											{
												isFound = true;
											}
										}

										if (!isFound)
										{
											affectedBonesList.emplace_back(variableName);
											if (!isChain)
											{
												affectedBones.emplace_back(affectedBonesList);
												affectedBonesList.clear();
											}
										}

										if (variableValue != "")
										{
											configMap[variableName] = variableValue;
											LOG("ConfigMap[%s] = %s = %s", variableName.c_str(), variableValue.c_str(), conditions.c_str());
										}
										if (conditions != "")
										{
											nodeConditionsMap[variableName] = ParseConditions(conditions);
										}
									}
								}
							}
						}
					}

					LOG_ERR("Extra Master Config file %s is loaded successfully.", filename.c_str());
				}
			}
		}
	}
}

void loadCollisionConfig()
{
	std::string	runtimeDirectory = GetRuntimeDirectory();

	if (!runtimeDirectory.empty())
	{
		#ifdef RUNTIME_VR_VERSION_1_4_15
		PlayerNodeLines.clear();
		PlayerNodesList.clear();
		#endif
		AffectedNodeLines.clear();
		ColliderNodeLines.clear();
		AffectedNodesList.clear();
		ColliderNodesList.clear();
		
		std::string filepath = runtimeDirectory + "Data\\SKSE\\Plugins\\CBPCollisionConfig.txt";
				
		std::ifstream file(filepath);

		if (!file.is_open())
		{
			transform(filepath.begin(), filepath.end(), filepath.begin(), ::tolower);
			file.open(filepath);
		}

		if (file.is_open())
		{
			std::string line;
			std::string currentSetting;
			while (std::getline(file, line))
			{
				//trim(line);
				skipComments(line);
				trim(line);
				if (line.length() > 0)
				{
					if (line.substr(0, 1) == "[")
					{
						//newsetting
						currentSetting = line;
					}
					else
					{
						if (currentSetting == "[ExtraOptions]")
						{
							std::string variableName;
							float variableValue = GetConfigSettingsFloatValue(line, variableName);
							if (variableName == "BellyBulge")
							{
								cbellybulge = variableValue;
							}
							else if (variableName == "BellyBulgeMax")
							{
								cbellybulgemax = variableValue;
							}
							else if (variableName == "BellyBulgePosLowest")
							{
								cbellybulgeposlowest = variableValue;
							}
							else if (variableName == "BellyBulgeNodes")
							{
								std::string variableStrValue = GetConfigSettingsStringValue(line, variableName);
								bellybulgenodesList = split(variableStrValue, ',');
							}
							else if (variableName == "VaginaOpeningLimit")
							{
								vaginaOpeningLimit = variableValue;
							}
							else if (variableName == "VaginaOpeningMultiplier")
							{
								vaginaOpeningMultiplier = variableValue;
							}
							else if (variableName == "BellyBulgeReturnTime")
							{
								bellyBulgeReturnTime = variableValue;
							}							
						}
						else if (currentSetting == "[AffectedNodes]")
						{
							std::vector<std::string> splittedList = splitMultiNonEmpty(line, ",()");
							if (splittedList.size() > 0)
							{
								AffectedNodeLines.emplace_back(splittedList[0]);
								ConfigLine newConfigLine;
								newConfigLine.NodeName = splittedList[0];
								if (splittedList.size() > 1)
								{
									for (int s = 1; s < splittedList.size(); s++)
									{
										if (splittedList[s] == "@")
										{
											newConfigLine.IgnoreAllSelfColliders = true;
										}
										else if (stringStartsWith(splittedList[s], "@"))
										{
											newConfigLine.IgnoredSelfColliders.emplace_back(splittedList[s].substr(1));
										}
										else
										{
											newConfigLine.IgnoredColliders.emplace_back(splittedList[s]);
										}
									}
								}
								AffectedNodesList.emplace_back(newConfigLine);
							}
						}
						else if (currentSetting == "[ColliderNodes]")
						{
							ColliderNodeLines.emplace_back(line);
							ConfigLine newConfigLine;
							newConfigLine.NodeName = line;
							ColliderNodesList.emplace_back(newConfigLine);
						}
						#ifdef RUNTIME_VR_VERSION_1_4_15
						else if (currentSetting == "[PlayerNodes]")
						{
							PlayerNodeLines.emplace_back(line);
							ConfigLine newConfigLine;
							newConfigLine.NodeName = line;
							PlayerNodesList.emplace_back(newConfigLine);
						}
						#endif
						else
						{
							Sphere newSphere; //type 0
							Capsule newCapsule; //type 1

							int type = 0;
							std::vector<std::string> LowHighWeight = split(line, '|');

							std::vector<std::string> PointSplitted;
							
							if (LowHighWeight.size() > 1)
								PointSplitted = split(LowHighWeight[0], '&');
							else
								PointSplitted = split(line, '&');

							if (PointSplitted.size() == 1)
							{
								ConfigLineSplitterSphere(line, newSphere);
								type = 0;
							}
							else if (PointSplitted.size() == 2)
							{
								ConfigLineSplitterCapsule(line, newCapsule);
								type = 1;
							}

							std::string trimmedSetting = gettrimmed(currentSetting);
							#ifdef RUNTIME_VR_VERSION_1_4_15
							if (std::find(PlayerNodeLines.begin(), PlayerNodeLines.end(), trimmedSetting) != PlayerNodeLines.end())
							{
								for (int i = 0; i < PlayerNodesList.size(); i++)
								{
									if (PlayerNodesList[i].NodeName == trimmedSetting)
									{
										if (type == 0)
										{
											newSphere.NodeName = PlayerNodesList[i].NodeName;
											PlayerNodesList[i].CollisionSpheres.emplace_back(newSphere);
										}
										else if (type == 1)
										{
											newCapsule.NodeName = PlayerNodesList[i].NodeName;
											PlayerNodesList[i].CollisionCapsules.emplace_back(newCapsule);
										}
										break;
									}
								}
							}
							#endif
							if (std::find(AffectedNodeLines.begin(), AffectedNodeLines.end(), trimmedSetting) != AffectedNodeLines.end())
							{
								for (int i = 0; i < AffectedNodesList.size(); i++)
								{
									if (AffectedNodesList[i].NodeName == trimmedSetting)
									{
										if (type == 0)
										{
											newSphere.NodeName = AffectedNodesList[i].NodeName;
											AffectedNodesList[i].CollisionSpheres.emplace_back(newSphere);
										}
										else if (type == 1)
										{
											newCapsule.NodeName = AffectedNodesList[i].NodeName;
											AffectedNodesList[i].CollisionCapsules.emplace_back(newCapsule);
										}
										break;
									}
								}
							}
							if (std::find(ColliderNodeLines.begin(), ColliderNodeLines.end(), trimmedSetting) != ColliderNodeLines.end())
							{
								for (int i = 0; i < ColliderNodesList.size(); i++)
								{
									if (ColliderNodesList[i].NodeName == trimmedSetting)
									{
										if (type == 0)
										{
											newSphere.NodeName = ColliderNodesList[i].NodeName;
											ColliderNodesList[i].CollisionSpheres.emplace_back(newSphere);
										}
										else if (type == 1)
										{
											newCapsule.NodeName = ColliderNodesList[i].NodeName;
											ColliderNodesList[i].CollisionCapsules.emplace_back(newCapsule);
										}
										break;
									}
								}
							}
						}
					}					
				}
			}
		}
		LOG_ERR("Collision Config file is loaded successfully.");
		return;
	}

	LOG_ERR("Collision Config file is not loaded.");
	return;
}

#ifdef RUNTIME_VR_VERSION_1_4_15
void GetSettings()
{	
	Setting* setting = GetINISetting("fMeleeWeaponTranslateX:VR");
	if (!setting || setting->GetType() != Setting::kType_Float)
		LOG("Failed to get fMeleeWeaponTranslateX from INI.");
	else
		MeleeWeaponTranslateX = setting->data.f32;

	setting = GetINISetting("fMeleeWeaponTranslateY:VR");
	if (!setting || setting->GetType() != Setting::kType_Float)
		LOG("Failed to get fMeleeWeaponTranslateY from INI.");
	else
		MeleeWeaponTranslateY = setting->data.f32;

	setting = GetINISetting("fMeleeWeaponTranslateZ:VR");
	if (!setting || setting->GetType() != Setting::kType_Float)
		LOG("Failed to get fMeleeWeaponTranslateZ from INI.");
	else
		MeleeWeaponTranslateZ = setting->data.f32;

	LOG("Melee Weapon Translate Values From ini: %g %g %g", MeleeWeaponTranslateX, MeleeWeaponTranslateY, MeleeWeaponTranslateZ);
}
#endif

void loadExtraCollisionConfig()
{
	std::string	runtimeDirectory = GetRuntimeDirectory();
	
	if (!runtimeDirectory.empty())
	{
		specificNPCConfigList.clear();

		std::string configPath = runtimeDirectory + "Data\\SKSE\\Plugins\\";
		
		auto configList = get_all_files_names_within_folder(configPath.c_str());
				
		for (int i = 0; i < configList.size(); i++)
		{
			std::string filename = configList.at(i);

			if (filename == "." || filename == "..")
				continue;

			if (stringStartsWith(filename, "cbpcollisionconfig") && filename != "CBPCollisionConfig.txt" && (stringEndsWith(filename, ".txt") || stringEndsWith(filename, ".ini")))
			{
				std::string msg = "File found: " + filename;
				LOG(msg.c_str());

				std::string filepath = configPath;
				filepath.append(filename);

				SpecificNPCConfig newNPCConfig;

				std::ifstream file(filepath);

				if (file.is_open())
				{
					std::string line;
					std::string currentSetting;
					while (std::getline(file, line))
					{
						//trim(line);
						skipComments(line);
						trim(line);
						if (line.length() > 0)
						{
							if (line.substr(0, 1) == "[")
							{
								//newsetting
								currentSetting = line;
							}
							else
							{
								if (currentSetting == "[Options]")
								{
									std::string variableName;
									std::string variableValue = GetConfigSettingsStringValue(line, variableName);
									if (variableName == "Conditions")
									{
										newNPCConfig.conditions = ParseConditions(variableValue);
									}
									else if (variableName == "Priority")
									{
										newNPCConfig.ConditionPriority = GetConfigSettingsValue(line, variableName);
									}
									else if (variableName == "Characters")
									{
										ConditionItem cItem;
										cItem.single = false;

										std::vector<std::string> charactersList = split(variableValue, ',');

										for (auto& characterName : charactersList)
										{
											ConditionItem oItem;
											oItem.single = true;
											oItem.str = characterName;
											oItem.Not = false;
											oItem.type = ConditionType::ActorName;
											cItem.OrItems.emplace_back(oItem);
										}
										newNPCConfig.conditions.AndItems.emplace_back(cItem);
									}
									else if (variableName == "Races")
									{
										ConditionItem cItem;
										cItem.single = false;

										std::vector<std::string> raceList = split(variableValue, ',');
										for (auto& raceName : raceList)
										{
											ConditionItem oItem;
											oItem.single = true;
											oItem.str = raceName;
											oItem.Not = false;
											oItem.type = ConditionType::IsRaceName;
											cItem.OrItems.emplace_back(oItem);
										}
										newNPCConfig.conditions.AndItems.emplace_back(cItem);
									}
								}
								else if (currentSetting == "[ExtraOptions]")
								{
									std::string variableName;
									float variableValue = GetConfigSettingsFloatValue(line, variableName);
									if (variableName == "BellyBulge")
									{
										newNPCConfig.cbellybulge = variableValue;
									}
									else if (variableName == "BellyBulgeMax")
									{
										newNPCConfig.cbellybulgemax = variableValue;
									}
									else if (variableName == "BellyBulgePosLowest")
									{
										newNPCConfig.cbellybulgeposlowest = variableValue;
									}
									else if (variableName == "BellyBulgeNodes")
									{
										std::string variableStrValue = GetConfigSettingsStringValue(line, variableName);
										newNPCConfig.bellybulgenodesList = split(variableStrValue, ',');
									}
									else if (variableName == "VaginaOpeningLimit")
									{
										newNPCConfig.vaginaOpeningLimit = variableValue;
									}
									else if (variableName == "VaginaOpeningMultiplier")
									{
										newNPCConfig.vaginaOpeningMultiplier = variableValue;
									}
									else if (variableName == "BellyBulgeReturnTime")
									{
										newNPCConfig.bellyBulgeReturnTime = variableValue;
									}
								}
								else if (currentSetting == "[AffectedNodes]")
								{
									std::vector<std::string> splittedList = splitMultiNonEmpty(line, ",()");
									if (splittedList.size() > 0)
									{
										newNPCConfig.AffectedNodeLines.emplace_back(splittedList[0]);
										ConfigLine newConfigLine;
										newConfigLine.NodeName = splittedList[0];
										if (splittedList.size() > 1)
										{
											for (int s = 1; s < splittedList.size(); s++)
											{
												if (splittedList[s] == "@")
												{
													newConfigLine.IgnoreAllSelfColliders = true;
												}
												else if (stringStartsWith(splittedList[s], "@"))
												{
													newConfigLine.IgnoredSelfColliders.emplace_back(splittedList[s].substr(1));
												}
												else
												{
													newConfigLine.IgnoredColliders.emplace_back(splittedList[s]);
												}
											}
										}
										newNPCConfig.AffectedNodesList.emplace_back(newConfigLine);
									}
								}
								else if (currentSetting == "[ColliderNodes]")
								{
									newNPCConfig.ColliderNodeLines.emplace_back(line);
									ConfigLine newConfigLine;
									newConfigLine.NodeName = line;
									newNPCConfig.ColliderNodesList.emplace_back(newConfigLine);
								}
								else
								{
									Sphere newSphere; //type 0
									Capsule newCapsule; //type 1

									int type = 0;
									std::vector<std::string> LowHighWeight = split(line, '|');

									std::vector<std::string> PointSplitted;

									if (LowHighWeight.size() == 1)
										PointSplitted = split(line, '&');
									else
										PointSplitted = split(LowHighWeight[0], '&');

									if (PointSplitted.size() == 1)
									{
										ConfigLineSplitterSphere(line, newSphere);
										type = 0;
									}
									else if (PointSplitted.size() == 2)
									{
										ConfigLineSplitterCapsule(line, newCapsule);
										type = 1;
									}

									std::string trimmedSetting = gettrimmed(currentSetting);

									if (std::find(newNPCConfig.AffectedNodeLines.begin(), newNPCConfig.AffectedNodeLines.end(), trimmedSetting) != newNPCConfig.AffectedNodeLines.end())
									{
										for (int i = 0; i < newNPCConfig.AffectedNodesList.size(); i++)
										{
											if (newNPCConfig.AffectedNodesList[i].NodeName == trimmedSetting)
											{
												if (type == 0)
													newNPCConfig.AffectedNodesList[i].CollisionSpheres.emplace_back(newSphere);
												else if (type == 1)
													newNPCConfig.AffectedNodesList[i].CollisionCapsules.emplace_back(newCapsule);
												break;
											}
										}
									}
									if (std::find(newNPCConfig.ColliderNodeLines.begin(), newNPCConfig.ColliderNodeLines.end(), trimmedSetting) != newNPCConfig.ColliderNodeLines.end())
									{
										for (int i = 0; i < newNPCConfig.ColliderNodesList.size(); i++)
										{
											if (newNPCConfig.ColliderNodesList[i].NodeName == trimmedSetting)
											{
												if (type == 0)
													newNPCConfig.ColliderNodesList[i].CollisionSpheres.emplace_back(newSphere);
												else if(type == 1)
													newNPCConfig.ColliderNodesList[i].CollisionCapsules.emplace_back(newCapsule);
												break;
											}
										}
									}
								}
							}
						}
					}
					if (newNPCConfig.conditions.AndItems.size() > 0)
					{
						specificNPCConfigList.emplace_back(newNPCConfig);
					}
				}
			}
		}

		if (specificNPCConfigList.size() > 1)
		{
			std::sort(specificNPCConfigList.begin(), specificNPCConfigList.end(), compareConfigs);
		}

		LOG_ERR("Specific collision config files(if any) are loaded successfully.");
		return;
	}

	LOG_ERR("Specific collision config files are not loaded.");
	return;
}

#ifdef RUNTIME_VR_VERSION_1_4_15
void LoadWeaponCollisionConfig()
{
	WeaponCollidersList.clear();
	std::string	runtimeDirectory = GetRuntimeDirectory();

	if (!runtimeDirectory.empty())
	{
		std::string configPath = runtimeDirectory + "Data\\SKSE\\Plugins\\CBPWeaponCollisionConfig.txt";

		std::ifstream file(configPath);
		std::string line;
		std::string currentSetting;
		while (std::getline(file, line))
		{
			trim(line);
			skipComments(line);
			trim(line);
			if (line.length() > 0)
			{
				if (line.substr(0, 1) == "[")
				{
					//newsetting
					currentSetting = line;					
				}
				else
				{					
					Triangle newTriangle;
					ConfigWeaponLineSplitter(line, newTriangle);

					WeaponConfigLine newLine;
					newLine.WeaponName = gettrimmed(currentSetting);					
					newLine.CollisionTriangle = newTriangle;	

					WeaponCollidersList.emplace_back(newLine);
				}
			}
		}

		LOG_ERR("Weapon Config file is loaded successfully.");
		return;
	}

	LOG_ERR("Weapon Config file is not loaded.");
	return;
}

void ConfigWeaponLineSplitter(std::string &line, Triangle &newTriangle)
{
	std::vector<std::string> pointsSplitted = split(line, '|');

	if (pointsSplitted.size() >= 3)
	{
		std::vector<std::string> splittedFloats = split(pointsSplitted[0], ',');
		if (splittedFloats.size() > 0)
		{
			newTriangle.a.x = (strtof(splittedFloats[0].c_str(), 0) + MeleeWeaponTranslateX);
		}
		if (splittedFloats.size() > 1)
		{
			newTriangle.a.y = (strtof(splittedFloats[1].c_str(), 0) + MeleeWeaponTranslateY);
		}
		if (splittedFloats.size() > 2)
		{
			newTriangle.a.z = (strtof(splittedFloats[2].c_str(), 0) + MeleeWeaponTranslateZ);
		}

		newTriangle.orga = newTriangle.a;

		splittedFloats = split(pointsSplitted[1], ',');
		if (splittedFloats.size() > 0)
		{
			newTriangle.b.x = (strtof(splittedFloats[0].c_str(), 0) + MeleeWeaponTranslateX);
		}
		if (splittedFloats.size() > 1)
		{
			newTriangle.b.y = (strtof(splittedFloats[1].c_str(), 0) + MeleeWeaponTranslateY);
		}
		if (splittedFloats.size() > 2)
		{
			newTriangle.b.z = (strtof(splittedFloats[2].c_str(), 0) + MeleeWeaponTranslateZ);
		}

		newTriangle.orgb = newTriangle.b;

		splittedFloats = split(pointsSplitted[2], ',');
		if (splittedFloats.size() > 0)
		{
			newTriangle.c.x = (strtof(splittedFloats[0].c_str(), 0) + MeleeWeaponTranslateX);
		}
		if (splittedFloats.size() > 1)
		{
			newTriangle.c.y = (strtof(splittedFloats[1].c_str(), 0) + MeleeWeaponTranslateY);
		}
		if (splittedFloats.size() > 2)
		{
			newTriangle.c.z = (strtof(splittedFloats[2].c_str(), 0) + MeleeWeaponTranslateZ);
		}
		newTriangle.orgc = newTriangle.c;
	}
}
#endif

void ConfigLineSplitterSphere(std::string &line, Sphere &newSphere)
{
	std::vector<std::string> lowHighSplitted = split(line, '|');
	if (lowHighSplitted.size() == 1)
	{
		std::vector<std::string> splittedFloats = split(lowHighSplitted[0], ',');
		if (splittedFloats.size()>0)
		{
			newSphere.offset0.x = strtof(splittedFloats[0].c_str(), 0);
			newSphere.offset100.x = newSphere.offset0.x;
		}
		if (splittedFloats.size()>1)
		{
			newSphere.offset0.y = strtof(splittedFloats[1].c_str(), 0);
			newSphere.offset100.y = newSphere.offset0.y;
		}
		if (splittedFloats.size()>2)
		{
			newSphere.offset0.z = strtof(splittedFloats[2].c_str(), 0);
			newSphere.offset100.z = newSphere.offset0.z;
		}
		if (splittedFloats.size()>3)
		{
			newSphere.radius0 = strtof(splittedFloats[3].c_str(), 0);
			newSphere.radius100 = newSphere.radius0;
		}
	}
	else if (lowHighSplitted.size() > 1)
	{
		std::vector<std::string> splittedFloats = split(lowHighSplitted[0], ',');
		if (splittedFloats.size()>0)
		{
			newSphere.offset0.x = strtof(splittedFloats[0].c_str(), 0);
		}
		if (splittedFloats.size()>1)
		{
			newSphere.offset0.y = strtof(splittedFloats[1].c_str(), 0);
		}
		if (splittedFloats.size()>2)
		{
			newSphere.offset0.z = strtof(splittedFloats[2].c_str(), 0);
		}
		if (splittedFloats.size()>3)
		{
			newSphere.radius0 = strtof(splittedFloats[3].c_str(), 0);
		}

		splittedFloats = split(lowHighSplitted[1], ',');
		if (splittedFloats.size()>0)
		{
			newSphere.offset100.x = strtof(splittedFloats[0].c_str(), 0);
		}
		if (splittedFloats.size()>1)
		{
			newSphere.offset100.y = strtof(splittedFloats[1].c_str(), 0);
		}
		if (splittedFloats.size()>2)
		{
			newSphere.offset100.z = strtof(splittedFloats[2].c_str(), 0);
		}
		if (splittedFloats.size()>3)
		{
			newSphere.radius100 = strtof(splittedFloats[3].c_str(), 0);
		}
	}
}

void ConfigLineSplitterCapsule(std::string &line, Capsule &newCapsules)
{
	std::vector<std::string> lowHighSplitted = split(line, '|');

	if (lowHighSplitted.size() == 1)
	{
		std::vector<std::string> EndPointSplitted = split(lowHighSplitted[0], '&');

		std::vector<std::string> splittedFloats = split(EndPointSplitted[0], ',');
		if (splittedFloats.size()>0)
		{
			newCapsules.End1_offset0.x = strtof(splittedFloats[0].c_str(), 0);
			newCapsules.End1_offset100.x = newCapsules.End1_offset0.x;
		}
		if (splittedFloats.size()>1)
		{
			newCapsules.End1_offset0.y = strtof(splittedFloats[1].c_str(), 0);
			newCapsules.End1_offset100.y = newCapsules.End1_offset0.y;
		}
		if (splittedFloats.size()>2)
		{
			newCapsules.End1_offset0.z = strtof(splittedFloats[2].c_str(), 0);
			newCapsules.End1_offset100.z = newCapsules.End1_offset0.z;
		}
		if (splittedFloats.size()>3)
		{
			newCapsules.End1_radius0 = strtof(splittedFloats[3].c_str(), 0);
			newCapsules.End1_radius100 = newCapsules.End1_radius0;
		}

		splittedFloats = split(EndPointSplitted[1], ',');
		if (splittedFloats.size() > 0)
		{
			newCapsules.End2_offset0.x = strtof(splittedFloats[0].c_str(), 0);
			newCapsules.End2_offset100.x = newCapsules.End2_offset0.x;
		}
		if (splittedFloats.size() > 1)
		{
			newCapsules.End2_offset0.y = strtof(splittedFloats[1].c_str(), 0);
			newCapsules.End2_offset100.y = newCapsules.End2_offset0.y;
		}
		if (splittedFloats.size() > 2)
		{
			newCapsules.End2_offset0.z = strtof(splittedFloats[2].c_str(), 0);
			newCapsules.End2_offset100.z = newCapsules.End2_offset0.z;
		}
		if (splittedFloats.size() > 3)
		{
			newCapsules.End2_radius0 = strtof(splittedFloats[3].c_str(), 0);
			newCapsules.End2_radius100 = newCapsules.End2_radius0;
		}
	}
	else if (lowHighSplitted.size() > 1)
	{
		std::vector<std::string> EndPointSplitted = split(lowHighSplitted[0], '&');

		std::vector<std::string> splittedFloats = split(EndPointSplitted[0], ',');
		if (splittedFloats.size()>0)
		{
			newCapsules.End1_offset0.x = strtof(splittedFloats[0].c_str(), 0);
		}
		if (splittedFloats.size()>1)
		{
			newCapsules.End1_offset0.y = strtof(splittedFloats[1].c_str(), 0);
		}
		if (splittedFloats.size()>2)
		{
			newCapsules.End1_offset0.z = strtof(splittedFloats[2].c_str(), 0);
		}
		if (splittedFloats.size()>3)
		{
			newCapsules.End1_radius0 = strtof(splittedFloats[3].c_str(), 0);
		}

		splittedFloats = split(EndPointSplitted[1], ',');
		if (splittedFloats.size()>0)
		{
			newCapsules.End2_offset0.x = strtof(splittedFloats[0].c_str(), 0);
		}
		if (splittedFloats.size()>1)
		{
			newCapsules.End2_offset0.y = strtof(splittedFloats[1].c_str(), 0);
		}
		if (splittedFloats.size()>2)
		{
			newCapsules.End2_offset0.z = strtof(splittedFloats[2].c_str(), 0);
		}
		if (splittedFloats.size()>3)
		{
			newCapsules.End2_radius0 = strtof(splittedFloats[3].c_str(), 0);
		}

		EndPointSplitted = split(lowHighSplitted[1], '&');

		splittedFloats = split(EndPointSplitted[0], ',');
		if (splittedFloats.size()>0)
		{
			newCapsules.End1_offset100.x = strtof(splittedFloats[0].c_str(), 0);
		}
		if (splittedFloats.size()>1)
		{
			newCapsules.End1_offset100.y = strtof(splittedFloats[1].c_str(), 0);
		}
		if (splittedFloats.size()>2)
		{
			newCapsules.End1_offset100.z = strtof(splittedFloats[2].c_str(), 0);
		}
		if (splittedFloats.size()>3)
		{
			newCapsules.End1_radius100 = strtof(splittedFloats[3].c_str(), 0);
		}

		splittedFloats = split(EndPointSplitted[1], ',');
		if (splittedFloats.size()>0)
		{
			newCapsules.End2_offset100.x = strtof(splittedFloats[0].c_str(), 0);
		}
		if (splittedFloats.size()>1)
		{
			newCapsules.End2_offset100.y = strtof(splittedFloats[1].c_str(), 0);
		}
		if (splittedFloats.size()>2)
		{
			newCapsules.End2_offset100.z = strtof(splittedFloats[2].c_str(), 0);
		}
		if (splittedFloats.size()>3)
		{
			newCapsules.End2_radius100 = strtof(splittedFloats[3].c_str(), 0);
		}
	}
}

int GetConfigSettingsValue(std::string line, std::string &variable)
{
	int value=0;
	std::vector<std::string> splittedLine = split(line, '=');
	variable = "";
	if (splittedLine.size() > 1)
	{
		variable = splittedLine[0];
		trim(variable);

		std::string valuestr = splittedLine[1];
		trim(valuestr);
		try 
		{
			value = std::stoi(valuestr);
		}
		catch (...)
		{
			value = 0;
		}
	}

	return value;
}

float GetConfigSettingsFloatValue(std::string line, std::string &variable)
{
	float value = 0;
	std::vector<std::string> splittedLine = split(line, '=');
	variable = "";
	if (splittedLine.size() > 1)
	{
		variable = splittedLine[0];
		trim(variable);

		std::string valuestr = splittedLine[1];
		trim(valuestr);
		value = strtof(valuestr.c_str(), 0);
	}

	return value;
}

std::string GetConfigSettingsStringValue(std::string line, std::string &variable)
{
	std::string valuestr = "";
	std::vector<std::string> splittedLine = split(line, '=');
	variable = "";
	if (splittedLine.size() > 0)
	{
		variable = splittedLine[0];
		trim(variable);
	}

	if (splittedLine.size() > 1)
	{
		valuestr = splittedLine[1];
		trim(valuestr);
	}

	return valuestr;
}

std::string GetConfigSettings2StringValues(std::string line, std::string& variable, std::string& value2)
{
	std::string valuestr = "";
	std::vector<std::string> splittedLine = split(line, '=');
	variable = "";
	if (splittedLine.size() > 0)
	{
		variable = splittedLine[0];
		trim(variable);
	}

	if (splittedLine.size() > 1)
	{
		valuestr = splittedLine[1];
		trim(valuestr);
	}

	if (splittedLine.size() > 2)
	{
		value2 = splittedLine[2];
		trim(value2);
	}
	return valuestr;
}

void printSpheresMessage(std::string message, std::vector<Sphere> spheres)
{
	for (int i = 0; i < spheres.size(); i++)
	{
		message += " Spheres: ";
		message += std::to_string(spheres[i].offset0.x);
		message += ",";
		message += std::to_string(spheres[i].offset0.y);
		message += ",";
		message += std::to_string(spheres[i].offset0.z);
		message += ",";
		message += std::to_string(spheres[i].radius0);
		message += " | ";
		message += std::to_string(spheres[i].offset100.x);
		message += ",";
		message += std::to_string(spheres[i].offset100.y);
		message += ",";
		message += std::to_string(spheres[i].offset100.z);
		message += ",";
		message += std::to_string(spheres[i].radius100);
	}
	LOG(message.c_str());
}

std::vector<std::string> ConfigLineVectorToStringVector(std::vector<ConfigLine> linesList)
{
	std::vector<std::string> outVector;

	for (int i = 0; i < linesList.size(); i++)
	{
		std::string str = linesList[i].NodeName;
		trim(str);
		outVector.emplace_back(str);
	}
	return outVector;
}

bool ConditionCheck(Actor* actor, ConditionItem& condition)
{
	if (condition.type == ConditionType::ActorFormId)
	{
		if (condition.Not == false)
			return (actor->formID == condition.id);
		else
			return (actor->formID != condition.id);
	}
	else if (condition.type == ConditionType::ActorName)
	{
		auto actorRef = DYNAMIC_CAST(actor, Actor, TESObjectREFR);

		if (actorRef != nullptr)
		{
			std::string actorrefname;
			if (actor->formID == 0x14) //If Player
			{
				actorrefname = "Player";
			}
			else
			{
				actorrefname = CALL_MEMBER_FN(actorRef, GetReferenceName)();
			}

			if(condition.Not == false)
				return (actorrefname == condition.str);
			else
				return (actorrefname != condition.str);
		}
	}
	else if (condition.type == ConditionType::ActorWeightGreaterThan)
	{
		auto actorRef = DYNAMIC_CAST(actor, Actor, TESObjectREFR);

		if (actorRef != nullptr)
		{
			auto npcWeight = CALL_MEMBER_FN(actorRef, GetWeight)();

			if(condition.Not == false)
				return ((npcWeight > condition.id));
			else
				return ((npcWeight <= condition.id));
		}
	}
	else if (condition.type == ConditionType::IsRaceFormId)
	{
		if (actor->race != nullptr)
		{
			if(condition.Not == false)
				return (actor->race->formID == condition.id);
			else
				return (actor->race->formID != condition.id);
		}
	}
	else if (condition.type == ConditionType::IsRaceName)
	{
		auto actorRef = DYNAMIC_CAST(actor, Actor, TESObjectREFR);

		if (actor->race != nullptr)
		{
			std::string actorRace = actor->race->fullName.GetName();

			if(condition.Not == false)
				return (actorRace == condition.str);
			else
				return (actorRace != condition.str);
		}
	}
	else if (condition.type == ConditionType::IsFemale)
	{
		bool sexMale = IsActorMale(actor);

		if (condition.Not == false)
			return !sexMale;
		else
			return sexMale;
	}
	else if (condition.type == ConditionType::IsMale)
	{
		bool sexMale = IsActorMale(actor);

		if (condition.Not == false)
			return sexMale;
		else
			return !sexMale;
	}
	else if (condition.type == ConditionType::IsInFaction)
	{
		FactionRankSet rankSet;
		CollectUniqueFactions factionVisitor(&rankSet, SCHAR_MIN, SCHAR_MAX);
		actor->VisitFactions(factionVisitor);

		std::vector<UInt32> factionIdsList;
		for (FactionRankSet::iterator it = rankSet.begin(); it != rankSet.end(); ++it)
		{
			if ((*it) != nullptr)
			{
				factionIdsList.emplace_back((*it)->formID);
			}
		}
		const bool hasFaction = std::find(factionIdsList.begin(), factionIdsList.end(), condition.id) != factionIdsList.end();

		if(condition.Not == false)
			return hasFaction;
		else
			return !hasFaction;
	}
	else if (condition.type == ConditionType::RaceHasKeywordId)
	{
		if (actor->race != nullptr)
		{
			TESForm* keywordForm = LookupFormByID(condition.id);

			if (keywordForm != nullptr)
			{
				BGSKeyword* keyword = DYNAMIC_CAST(keywordForm, TESForm, BGSKeyword);
				if (keyword != nullptr)
				{
					const bool haskeyword = actor->race->keyword.HasKeyword(keyword);

					if (condition.Not == false)
						return haskeyword;
					else
						return !haskeyword;
				}
			}
		}
	}
	else if (condition.type == ConditionType::RaceHasKeywordName)
	{
		if (actor->race != nullptr)
		{
			TESForm* keywordForm = papyrusKeyword::GetKeyword(nullptr, BSFixedString(condition.str.c_str()));

			if (keywordForm != nullptr)
			{
				BGSKeyword* keyword = DYNAMIC_CAST(keywordForm, TESForm, BGSKeyword);
				if (keyword != nullptr)
				{
					const bool haskeyword = actor->race->keyword.HasKeyword(keyword);

					if (condition.Not == false)
						return haskeyword;
					else
						return !haskeyword;
				}
			}
		}
	}
	else if (condition.type == ConditionType::HasKeywordId)
	{
		if (actor->baseForm != nullptr)
		{
			TESNPC* actorBase = DYNAMIC_CAST(actor->baseForm, TESForm, TESNPC);
			if (actorBase != nullptr)
			{
				TESForm* keywordForm = LookupFormByID(condition.id);

				if (keywordForm != nullptr)
				{
					BGSKeyword* keyword = DYNAMIC_CAST(keywordForm, TESForm, BGSKeyword);
					if (keyword != nullptr)
					{
						const bool haskeyword = actorBase->keyword.HasKeyword(keyword);

						if (condition.Not == false)
							return haskeyword;
						else
							return !haskeyword;
					}
				}
			}
		}
	}
	else if (condition.type == ConditionType::HasKeywordName)
	{
		if (actor->baseForm != nullptr)
		{
			TESNPC* actorBase = DYNAMIC_CAST(actor->baseForm, TESForm, TESNPC);
			if (actorBase != nullptr)
			{
				TESForm* keywordForm = papyrusKeyword::GetKeyword(nullptr, BSFixedString(condition.str.c_str()));

				if (keywordForm != nullptr)
				{
					BGSKeyword* keyword = DYNAMIC_CAST(keywordForm, TESForm, BGSKeyword);
					if (keyword != nullptr)
					{
						const bool haskeyword = actorBase->keyword.HasKeyword(keyword);

						if (condition.Not == false)
							return haskeyword;
						else
							return !haskeyword;
					}
				}
			}
		}
	}
	else if (condition.type == ConditionType::IsActorBase)
	{
		if (actor->baseForm != nullptr)
		{
			TESNPC* actorBase = DYNAMIC_CAST(actor->baseForm, TESForm, TESNPC);
			if (actorBase != nullptr)
			{
				if(condition.Not == false)
					return (actorBase->formID == condition.id);
				else
					return (actorBase->formID != condition.id);
			}
		}
	}
	else if (condition.type == ConditionType::IsPlayerTeammate)
	{
		bool isTeammate = (actor->flags1 & Actor::kFlags_IsPlayerTeammate) == Actor::kFlags_IsPlayerTeammate;

		if (condition.Not == false)
			return isTeammate;
		else
			return !isTeammate;
	}
	else if (condition.type == ConditionType::IsPlayer)
	{
		bool isPlayer = actor->formID == 0x14;

		if (condition.Not == false)
			return isPlayer;
		else
			return !isPlayer;
	}
	else if (condition.type == ConditionType::IsUnique)
	{
		if (actor->baseForm != nullptr)
		{
			TESNPC* actorBase = DYNAMIC_CAST(actor->baseForm, TESForm, TESNPC);
			if (actorBase != nullptr)
			{
				bool isUnique = (actorBase->actorData.flags & 0x20/*TESActorBaseData::kFlag_Unique*/) != 0;

				if (condition.Not == false)
					return isUnique;
				else
					return !isUnique;
			}
		}
	}
	else if (condition.type == ConditionType::IsVoiceType)
	{
		if (actor->baseForm != nullptr)
		{
			TESNPC* actorBase = DYNAMIC_CAST(actor->baseForm, TESForm, TESNPC);
			if (actorBase != nullptr)
			{
				bool isVoiceType = (actorBase->actorData.voiceType != nullptr && actorBase->actorData.voiceType->formID == condition.id);

				if (condition.Not == false)
					return isVoiceType;
				else
					return !isVoiceType;
			}
		}
	}
	else if (condition.type == ConditionType::IsCombatStyle)
	{
		if (actor->baseForm != nullptr)
		{
			TESNPC* actorBase = DYNAMIC_CAST(actor->baseForm, TESForm, TESNPC);
			if (actorBase != nullptr)
			{
				bool isCombatStyle = (actorBase->combatStyle != nullptr && actorBase->combatStyle->formID == condition.id);

				if (condition.Not == false)
					return isCombatStyle;
				else
					return !isCombatStyle;
			}
		}
	}
	else if (condition.type == ConditionType::IsClass)
	{
		if (actor->baseForm != nullptr)
		{
			TESNPC* actorBase = DYNAMIC_CAST(actor->baseForm, TESForm, TESNPC);
			if (actorBase != nullptr)
			{
				bool isClass = (actorBase->npcClass != nullptr && actorBase->npcClass->formID == condition.id);

				if (condition.Not == false)
					return isClass;
				else
					return !isClass;
			}
		}
	}

	return false;
}

bool GetSpecificNPCConfigForActor(Actor * actor, SpecificNPCConfig &snc)
{
	if (actor != nullptr)
	{
		for (int i = 0; i < specificNPCConfigList.size(); i++)
		{
			bool correct = true;

			for (int j = 0; j < specificNPCConfigList.at(i).conditions.AndItems.size(); j++)
			{
				if (specificNPCConfigList.at(i).conditions.AndItems.at(j).single)
				{
					if (!ConditionCheck(actor, specificNPCConfigList.at(i).conditions.AndItems.at(j)))
					{
						correct = false;
						break;
					}
				}
				else
				{
					bool innerCorrect = false;
					for (int o = 0; o < specificNPCConfigList.at(i).conditions.AndItems.at(j).OrItems.size(); o++)
					{
						if (specificNPCConfigList.at(i).conditions.AndItems.at(j).OrItems.at(o).single)
						{
							if (ConditionCheck(actor, specificNPCConfigList.at(i).conditions.AndItems.at(j).OrItems.at(o)))
							{
								innerCorrect = true;
								break;
							}
						}
					}
					if (innerCorrect == false)
					{
						correct = false;
						break;
					}
				}
			}

			if (correct)
			{
				snc = specificNPCConfigList.at(i);
				return true;
			}
		}
	}

	return false;
}

bool GetSpecificNPCBounceConfigForActor(Actor* actor, SpecificNPCBounceConfig& snbc)
{
	if (actor != nullptr)
	{
		for (int i = 0; i < specificNPCBounceConfigList.size(); i++)
		{
			bool correct = true;

			for (int j = 0; j < specificNPCBounceConfigList.at(i).conditions.AndItems.size(); j++)
			{
				if (specificNPCBounceConfigList.at(i).conditions.AndItems.at(j).single)
				{
					if (!ConditionCheck(actor, specificNPCBounceConfigList.at(i).conditions.AndItems.at(j)))
					{
						correct = false;
						break;
					}
				}
				else
				{
					bool innerCorrect = false;
					for (int o = 0; o < specificNPCBounceConfigList.at(i).conditions.AndItems.at(j).OrItems.size(); o++)
					{
						if (specificNPCBounceConfigList.at(i).conditions.AndItems.at(j).OrItems.at(o).single)
						{
							if (ConditionCheck(actor, specificNPCBounceConfigList.at(i).conditions.AndItems.at(j).OrItems.at(o)))
							{
								innerCorrect = true;
								break;
							}
						}
					}
					if (innerCorrect == false)
					{
						correct = false;
						break;
					}
				}
			}

			if (correct)
			{
				snbc = specificNPCBounceConfigList.at(i);
				return true;
			}
		}
	}

	return false;
}

//If the config of that part is not set and just set to default, return false
bool IsConfigActuallyAllocated(SpecificNPCBounceConfig snbc, std::string section)
{
	return (snbc.config[section]["stiffness"] >= 0.0001f) || (snbc.config0weight[section]["stiffness"] >= 0.0001f) //Doesn't set physics config?
		|| (snbc.config[section]["stiffness2"] >= 0.0001f) || (snbc.config0weight[section]["stiffness2"] >= 0.0001f)
		|| (snbc.config[section]["damping"] >= 0.0001f) || (snbc.config0weight[section]["damping"] >= 0.0001f)
		|| (snbc.config[section]["collisionXmaxoffset"] >= 100.0001f) || (snbc.config[section]["collisionXmaxoffset"] <= 99.9999f) //Doesn't set collision config?
		|| (snbc.config0weight[section]["collisionXmaxoffset"] >= 100.0001f) || (snbc.config0weight[section]["collisionXmaxoffset"] <= 99.9999f)
		|| (snbc.config[section]["collisionXminoffset"] <= -100.0001f) || (snbc.config[section]["collisionXminoffset"] >= -99.9999f)
		|| (snbc.config0weight[section]["collisionXminoffset"] <= -100.0001f) || (snbc.config0weight[section]["collisionXminoffset"] >= -99.9999f)
		|| (snbc.config[section]["collisionYmaxoffset"] >= 100.0001f) || (snbc.config[section]["collisionYmaxoffset"] <= 99.9999f)
		|| (snbc.config0weight[section]["collisionYmaxoffset"] >= 100.0001f) || (snbc.config0weight[section]["collisionYmaxoffset"] <= 99.9999f)
		|| (snbc.config[section]["collisionYminoffset"] <= -100.0001f) || (snbc.config[section]["collisionYminoffset"] >= -99.9999f)
		|| (snbc.config0weight[section]["collisionYminoffset"] <= -100.0001f) || (snbc.config0weight[section]["collisionYminoffset"] >= -99.9999f)
		|| (snbc.config[section]["collisionZmaxoffset"] >= 100.0001f) || (snbc.config[section]["collisionZmaxoffset"] <= 99.9999f)
		|| (snbc.config0weight[section]["collisionZmaxoffset"] >= 100.0001f) || (snbc.config0weight[section]["collisionZmaxoffset"] <= 99.9999f)
		|| (snbc.config[section]["collisionZminoffset"] <= -100.0001f) || (snbc.config[section]["collisionZminoffset"] >= -99.9999f)
		|| (snbc.config0weight[section]["collisionZminoffset"] <= -100.0001f) || (snbc.config0weight[section]["collisionZminoffset"] >= -99.9999f);
}

bool CheckActorForConditions(Actor* actor, Conditions &conditions)
{
	bool correct = true;
	if (actor != nullptr)
	{		
		for (int j = 0; j < conditions.AndItems.size(); j++)
		{
			if (conditions.AndItems.at(j).single)
			{
				if (!ConditionCheck(actor, conditions.AndItems.at(j)))
				{
					correct = false;
					break;
				}
			}
			else
			{
				bool innerCorrect = false;
				for (int o = 0; o < conditions.AndItems.at(j).OrItems.size(); o++)
				{
					if (conditions.AndItems.at(j).OrItems.at(o).single)
					{
						if (ConditionCheck(actor, conditions.AndItems.at(j).OrItems.at(o)))
						{
							innerCorrect = true;
							break;
						}
					}
				}
				if (innerCorrect == false)
				{
					correct = false;
					break;
				}
			}
		}
	}

	return correct;
}

bool IsActorMale(Actor* actor)
{
	TESNPC * actorNPC = DYNAMIC_CAST(actor->baseForm, TESForm, TESNPC);

	if (actorNPC != nullptr)
	{
		auto npcSex = actorNPC ? CALL_MEMBER_FN(actorNPC, GetSex)() : 1;

		if (npcSex == 0) //Actor is male
			return true;
		else
			return false;
	}
	else
		return false;
}

#ifdef RUNTIME_VR_VERSION_1_4_15
std::vector<Triangle> GetCollisionTriangles(std::string name, UInt8 kType)
{
	std::vector<Triangle> resultList;
	for (int i = 0; i < WeaponCollidersList.size(); i++)
	{
		if (WeaponCollidersList[i].WeaponName == name)
		{
			resultList.emplace_back(WeaponCollidersList[i].CollisionTriangle);
		}
	}
	if (resultList.size() == 0 && kType > 0)
	{
		std::string typeName = GetWeaponTypeName(kType);
		if (typeName != "")
		{
			for (int i = 0; i < WeaponCollidersList.size(); i++)
			{
				if (WeaponCollidersList[i].WeaponName == typeName)
				{
					resultList.emplace_back(WeaponCollidersList[i].CollisionTriangle);
				}
			}
		}
	}
	return resultList;
}

std::string GetWeaponTypeName(UInt8 kType)
{
	if (kType == TESObjectWEAP::GameData::kType_TwoHandSword || kType == TESObjectWEAP::GameData::kType_2HS)
		return "Type_TwoHandSword";
	else if (kType == TESObjectWEAP::GameData::kType_TwoHandAxe || kType == TESObjectWEAP::GameData::kType_2HA)
		return "Type_TwoHandAxe";
	else if (kType == TESObjectWEAP::GameData::kType_OneHandSword || kType == TESObjectWEAP::GameData::kType_1HS)
		return "Type_OneHandSword";
	else if (kType == TESObjectWEAP::GameData::kType_OneHandAxe || kType == TESObjectWEAP::GameData::kType_1HA)
		return "Type_OneHandAxe";
	else if (kType == TESObjectWEAP::GameData::kType_OneHandDagger || kType == TESObjectWEAP::GameData::kType_1HD)
		return "Type_OneHandDagger";
	else if (kType == TESObjectWEAP::GameData::kType_OneHandMace || kType == TESObjectWEAP::GameData::kType_1HM)
		return "Type_OneHandMace";
	else if (kType == TESObjectWEAP::GameData::kType_CrossBow || kType == TESObjectWEAP::GameData::kType_CBow)
		return "Type_CrossBow";
	else if (kType == TESObjectWEAP::GameData::kType_Bow || kType == TESObjectWEAP::GameData::kType_Bow2)
		return "Type_Bow";
	else if (kType == TESObjectWEAP::GameData::kType_Staff || kType == TESObjectWEAP::GameData::kType_Staff2)
		return "Type_Staff";
	else
		return "";
}

void LeftHandedModeChange()
{
	const int value = vlibGetSetting("bLeftHandedMode:VRInput");
	if (value != leftHandedMode)
	{
		leftHandedMode = value;
		LOG_ERR("Left Handed Mode is %s.", leftHandedMode ? "ON" : "OFF");
	}
}
#endif

//Menu Stuff

AllMenuEventHandler menuEvent;

EventResult AllMenuEventHandler::ReceiveEvent(MenuOpenCloseEvent * evn, EventDispatcher<MenuOpenCloseEvent> * dispatcher)
{
	if (evn->opening)
	{
		MenuOpened(evn->menuName.c_str());
	}
	else
	{
		MenuClosed(evn->menuName.c_str());
	}

	return EventResult::kEvent_Continue;
}

void MenuOpened(std::string name)
{
	if (name == "Dialogue Menu")
	{
		dialogueMenuOpen.store(true);
	}
	else if (name == "RaceSex Menu")
	{
		raceSexMenuOpen.store(true);
	}
	else if (name == "Main Menu")
	{
		MainMenuOpen.store(true);
		ActorNodeStoppedPhysicsMap.clear();
	}
}

void MenuClosed(std::string name)
{
	if (name == "Dialogue Menu")
	{
		dialogueMenuOpen.store(false);
	}
	else if (name == "RaceSex Menu")
	{
		raceSexMenuClosed.store(true);
		raceSexMenuOpen.store(false);
	}
#ifdef RUNTIME_VR_VERSION_1_4_15
	else if(name == "Console" || name == "Journal Menu")
	{
		LeftHandedModeChange();
	}
#endif
}





BSFixedString GetVersion(StaticFunctionTag* base)
{
	return BSFixedString("1");
}

BSFixedString GetVersionMinor(StaticFunctionTag* base)
{
	return BSFixedString("5");
}

BSFixedString GetVersionBeta(StaticFunctionTag* base)
{
	return BSFixedString("0");
}

std::string GetActorNodeString(Actor* actor, BSFixedString nodeName)
{
	return num2hex(actor->formID, 8) + ":" + nodeName.c_str();
}

std::string GetFormIdNodeString(UInt32 id, BSFixedString nodeName)
{
	return num2hex(id, 8) + ":" + nodeName.c_str();
}

void StartPhysics(StaticFunctionTag* base, Actor* actor, BSFixedString nodeName)
{
	if (actor != nullptr)
	{
		ActorNodeStoppedPhysicsMap[GetActorNodeString(actor, nodeName)] = false;
	}
}

void StopPhysics(StaticFunctionTag* base, Actor* actor, BSFixedString nodeName)
{
	if (actor != nullptr)
	{
		ActorNodeStoppedPhysicsMap[GetActorNodeString(actor, nodeName)] = true;
	}
}

//Initializes openvr system. Required for haptic triggers.


bool RegisterFuncs(VMClassRegistry* registry)
{
#ifdef RUNTIME_VR_VERSION_1_4_15
	LeftHandedModeChange();
#endif

	registry->RegisterFunction(
		new NativeFunction0<StaticFunctionTag, BSFixedString>("GetVersion", "CBPCPluginScript", GetVersion, registry));

	registry->RegisterFunction(
		new NativeFunction0<StaticFunctionTag, BSFixedString>("GetVersionMinor", "CBPCPluginScript", GetVersionMinor, registry));

	registry->RegisterFunction(
		new NativeFunction0<StaticFunctionTag, BSFixedString>("GetVersionBeta", "CBPCPluginScript", GetVersionBeta, registry));

	registry->RegisterFunction(
		new NativeFunction2 <StaticFunctionTag, void, Actor*, BSFixedString> ("StartPhysics", "CBPCPluginScript", StartPhysics, registry));

	registry->RegisterFunction(
		new NativeFunction2 <StaticFunctionTag, void, Actor*, BSFixedString> ("StopPhysics", "CBPCPluginScript", StopPhysics, registry));

	LOG("CBPC registerFunction\n");
	return true;
}

std::shared_mutex log_lock; //yeah, this is required to avoid conflicts with logging when parallel processing

void Log(const int msgLogLevel, const char * fmt, ...)
{
	if (msgLogLevel > logging)
	{
		return;
	}

	std::lock_guard<std::shared_mutex> log_guard(log_lock);

	va_list args;
	char logBuffer[4096];

	va_start(args, fmt);
	vsprintf_s(logBuffer, sizeof(logBuffer), fmt, args);
	va_end(args);

	_MESSAGE(logBuffer);
}

