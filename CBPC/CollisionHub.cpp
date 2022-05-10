#include "CollisionHub.h"
#include "hash.h"

PartitionMap partitions;


//debug variable
long callCount = 0;
#ifdef RUNTIME_VR_VERSION_1_4_15

void CreatePlayerColliders(concurrency::concurrent_unordered_map<std::string, Collision> &actorCollidersList)
{
	PlayerCharacter	* player = *g_thePlayer;

	BSFixedString leftWandNode("LeftWandNode");
	BSFixedString rightWandNode("RightWandNode");

	LOG_INFO("Creating player colliders");
	if (player && player->loadedState)
	{
		auto actorRef = DYNAMIC_CAST(player, PlayerCharacter, TESObjectREFR);

		auto playerWeight = CALL_MEMBER_FN(actorRef, GetWeight)();		

		//printMessageInt("PlayerNodesList count:", PlayerNodesList.size());
		for each (ConfigLine line in PlayerNodesList)
		{
			//printSpheresMessage(line.NodeName, line.CollisionSpheres);
			BSFixedString fs = ReturnUsableString(line.NodeName);

			NiAVObject* node = nullptr;
			auto & nodeList = (*g_thePlayer)->nodeList;
			if (fs.data == leftWandNode.data)
			{
				node = nodeList[PlayerCharacter::kNode_LeftWandNode];
			}
			else if(fs.data == rightWandNode.data)
			{
				node = nodeList[PlayerCharacter::kNode_RightWandNode];
			}
			std::string leftObjectName = "";
			std::string rightObjectName = "";

			if (node)
			{
				Collision nodeCollision = Collision::Collision(node, line.CollisionSpheres, line.CollisionCapsules, playerWeight);
				nodeCollision.colliderActor = player;
				nodeCollision.colliderNodeName = fs.data;

				if (player->actorState.IsWeaponDrawn() && !dialogueMenuOpen)
				{
					if (fs.data == leftWandNode.data)
					{
						TESForm* leftEquippedObject = player->GetEquippedObject(true);
						if (leftEquippedObject)
						{
							if (leftEquippedObject->IsWeapon())
							{
								TESObjectWEAP* leftWeapon = DYNAMIC_CAST(leftEquippedObject, TESForm, TESObjectWEAP);
								if (leftWeapon)
								{
									leftObjectName = leftWeapon->fullName.GetName();

									if (!(leftWeapon->gameData.type == TESObjectWEAP::GameData::kType_TwoHandSword 
										|| leftWeapon->gameData.type == TESObjectWEAP::GameData::kType_TwoHandAxe 
										|| leftWeapon->gameData.type == TESObjectWEAP::GameData::kType_2HA 
										|| leftWeapon->gameData.type == TESObjectWEAP::GameData::kType_2HS))
									{
										nodeCollision.collisionTriangles = GetCollisionTriangles(leftObjectName, leftWeapon->gameData.type);
									}
								}
							}
						}
					}
					else if (fs.data == rightWandNode.data)
					{
						TESForm* rightEquippedObject = player->GetEquippedObject(false);

						if (rightEquippedObject)
						{
							if (rightEquippedObject->IsWeapon())
							{
								TESObjectWEAP* rightWeapon = DYNAMIC_CAST(rightEquippedObject, TESForm, TESObjectWEAP);
								if (rightWeapon)
								{
									rightObjectName = rightWeapon->fullName.GetName();

									if (rightWeapon->gameData.type != TESObjectWEAP::GameData::kType_Bow && rightWeapon->gameData.type != TESObjectWEAP::GameData::kType_Bow2)
									{
										nodeCollision.collisionTriangles = GetCollisionTriangles(rightObjectName, rightWeapon->gameData.type);
									}
								}
							}
						}
					}
				}
				actorCollidersList.insert(std::make_pair(line.NodeName, nodeCollision));
				LOG_INFO("Added %s weapon collider", fs.data);
			}
		}
	}
}

void UpdatePlayerColliders(concurrency::concurrent_unordered_map<std::string, Collision> &actorCollidersList)
{
	PlayerCharacter	* player = *g_thePlayer;

	if (player && player->loadedState)
	{
		LOG_INFO("Updating player colliders");
		auto actorRef = DYNAMIC_CAST(player, PlayerCharacter, TESObjectREFR);

		auto playerWeight = CALL_MEMBER_FN(actorRef, GetWeight)();

		/*LOG("actorCollidersList has %d items", actorCollidersList.size());
		for each(auto& item in actorCollidersList)
		{			
			LOG("item %s - actor: %x - %s, spheres:%d - triangles:%d", item.first, item.second.colliderActor->formID, item.second.colliderNodeName.c_str(), item.second.collisionSpheres.size(), item.second.collisionTriangles.size());
		}*/
		//printMessageInt("PlayerNodesList count:", PlayerNodesList.size());
		for each (ConfigLine line in PlayerNodesList)
		{
			LOG_INFO("Checking actorCollidersList for %s", line.NodeName.c_str());
			auto actorCollider = actorCollidersList.find(line.NodeName);
			if (actorCollider != actorCollidersList.end())
			{
				NiAVObject* node = actorCollider->second.CollisionObject;
				std::string leftObjectName = "";
				std::string rightObjectName = "";

				LOG_INFO("actorCollider %s of %x", actorCollider->first, actorCollider->second.colliderActor->formID);

				if (node)
				{
					LOG_INFO("Node: %s", line.NodeName.c_str());
					if (player->actorState.IsWeaponDrawn() && !dialogueMenuOpen)
					{
						if (actorCollider->second.colliderNodeName == "LeftWandNode")
						{
							TESForm* leftEquippedObject = player->GetEquippedObject(true);
							if (leftEquippedObject)
							{
								if (leftEquippedObject->IsWeapon())
								{
									TESObjectWEAP* leftWeapon = DYNAMIC_CAST(leftEquippedObject, TESForm, TESObjectWEAP);
									if (leftWeapon)
									{
										leftObjectName = leftWeapon->fullName.GetName();

										if (!(leftWeapon->gameData.type == TESObjectWEAP::GameData::kType_TwoHandSword
											|| leftWeapon->gameData.type == TESObjectWEAP::GameData::kType_TwoHandAxe
											|| leftWeapon->gameData.type == TESObjectWEAP::GameData::kType_2HA
											|| leftWeapon->gameData.type == TESObjectWEAP::GameData::kType_2HS))
										{
											actorCollider->second.collisionTriangles = GetCollisionTriangles(leftObjectName, leftWeapon->type());
											LOG_INFO("Updated left weapon collider.");
											continue;
										}
									}
								}
							}
						}
						else if (actorCollider->second.colliderNodeName == "RightWandNode")
						{
							TESForm* rightEquippedObject = player->GetEquippedObject(false);

							if (rightEquippedObject)
							{
								if (rightEquippedObject->IsWeapon())
								{
									TESObjectWEAP* rightWeapon = DYNAMIC_CAST(rightEquippedObject, TESForm, TESObjectWEAP);
									if (rightWeapon)
									{
										rightObjectName = rightWeapon->fullName.GetName();

										if (rightWeapon->gameData.type != TESObjectWEAP::GameData::kType_Bow && rightWeapon->gameData.type != TESObjectWEAP::GameData::kType_Bow2)
										{
											actorCollider->second.collisionTriangles = GetCollisionTriangles(rightObjectName, rightWeapon->type());
											LOG_INFO("Updated right weapon collider.");
											continue;
										}
									}
								}
							}

						}						
					}
				}
				else
				{
					LOG_INFO("Node %s is null", line.NodeName.c_str());
				}
				actorCollider->second.collisionTriangles.clear();
				LOG_INFO("Deleted weapon collider.");
			}
			else
			{
				LOG_INFO("Cannot find %s", line.NodeName.c_str());
			}
		}
	}
}
#endif

bool CreateActorColliders(Actor * actor, concurrency::concurrent_unordered_map<std::string, Collision> &actorCollidersList)
{
	bool GroundCollisionEnabled = false;
	NiNode* mostInterestingRoot;
	
	#ifdef RUNTIME_VR_VERSION_1_4_15
	if (actor == (*g_thePlayer)) //To check if we can support VR IK
	{			
		//NiNode* rootNodeTP = (*g_thePlayer)->GetNiRootNode(0);

		NiNode* rootNodeFP = (*g_thePlayer)->GetNiRootNode(2);

		if (rootNodeFP != nullptr)
			mostInterestingRoot = rootNodeFP;
		else
			return false;
	}
	else
	{
	#endif
		if (actor && actor->loadedState && actor->loadedState->node)
		{
			mostInterestingRoot = actor->loadedState->node;
		}
		else
			return false;
#ifdef RUNTIME_VR_VERSION_1_4_15
	}
#endif

	auto actorRef = DYNAMIC_CAST(actor, Actor, TESObjectREFR);
	float npcWeight = 50.0f;
	float actorBaseScale = 1.0f;
	if (actorRef)
	{
		npcWeight = CALL_MEMBER_FN(actorRef, GetWeight)();

		actorBaseScale = CALL_MEMBER_FN(actorRef, GetBaseScale)();
	}

	concurrency::concurrent_vector<ConfigLine>* ColliderNodesListPtr;

	SpecificNPCConfig snc;

	if (actor)
	{
		if (GetSpecificNPCConfigForActor(actor, snc))
		{
			ColliderNodesListPtr = &(snc.ColliderNodesList);
		}
		else
		{
			ColliderNodesListPtr = &ColliderNodesList;
		}
	}
	else
	{
		ColliderNodesListPtr = &ColliderNodesList;
	}

	std::shared_mutex CH_read_lock;

	concurrency::parallel_for (size_t(0), ColliderNodesListPtr->size(), [&](size_t j)
	{
		if (ColliderNodesListPtr->at(j).NodeName.compare(GroundReferenceBone.data) == 0) //detecting NPC Root [Root] node for ground collision
		{
			GroundCollisionEnabled = true;
		}
		else
		{
			BSFixedString fs = ReturnUsableString(ColliderNodesListPtr->at(j).NodeName);
			CH_read_lock.lock();
			NiAVObject* node = mostInterestingRoot->GetObjectByName(&fs.data);
			CH_read_lock.unlock();
			if (node)
			{
				Collision newCol = Collision::Collision(node, ColliderNodesListPtr->at(j).CollisionSpheres, ColliderNodesListPtr->at(j).CollisionCapsules, npcWeight);
				newCol.colliderActor = actor;
				newCol.colliderNodeName = fs.data;
				newCol.scaleWeight = ColliderNodesListPtr->at(j).scaleWeight;
				newCol.actorBaseScale = actorBaseScale;

				actorCollidersList.insert(std::make_pair(ColliderNodesListPtr->at(j).NodeName, newCol));
			}
		}
	});
	return GroundCollisionEnabled;
}

//Unfortunately this doesn't work.
bool CheckPelvisArmor(Actor* actor)
{
	return papyrusActor::GetWornForm(actor, 49) != NULL && papyrusActor::GetWornForm(actor, 52) != NULL && papyrusActor::GetWornForm(actor, 53) != NULL && papyrusActor::GetWornForm(actor, 54) != NULL && papyrusActor::GetWornForm(actor, 56) != NULL && papyrusActor::GetWornForm(actor, 58) != NULL;
}

void UpdateColliderPositions(concurrency::concurrent_unordered_map<std::string, Collision> &colliderList, concurrency::concurrent_unordered_map<std::string, NiPoint3> NodeCollisionSyncList)
{
	concurrency::parallel_for_each(colliderList.begin(), colliderList.end(), [&](auto& collider)
	{
		NiPoint3 VirtualOffset = emptyPoint;

		if (NodeCollisionSyncList.find(collider.second.colliderNodeName) != NodeCollisionSyncList.end())
			VirtualOffset = NodeCollisionSyncList[collider.second.colliderNodeName];

		float colliderNodescale = 1.0f - ((1.0f - (collider.second.CollisionObject->m_worldTransform.scale / collider.second.actorBaseScale)) * collider.second.scaleWeight);

		for (int j = 0; j < collider.second.collisionSpheres.size(); j++)
		{
			collider.second.collisionSpheres[j].offset100 = collider.second.collisionSpheres[j].offset0 * collider.second.actorBaseScale * colliderNodescale;
			collider.second.collisionSpheres[j].worldPos = collider.second.CollisionObject->m_worldTransform.pos + (collider.second.CollisionObject->m_worldTransform.rot * collider.second.collisionSpheres[j].offset100) + VirtualOffset;
			collider.second.collisionSpheres[j].radius100 = collider.second.collisionSpheres[j].radius0 * collider.second.actorBaseScale * colliderNodescale;
			collider.second.collisionSpheres[j].radius100pwr2 = collider.second.collisionSpheres[j].radius100 * collider.second.collisionSpheres[j].radius100;
		}

		for (int k = 0; k < collider.second.collisionCapsules.size(); k++)
		{
			collider.second.collisionCapsules[k].End1_offset100 = collider.second.collisionCapsules[k].End1_offset0 * collider.second.actorBaseScale * colliderNodescale;
			collider.second.collisionCapsules[k].End1_worldPos = collider.second.CollisionObject->m_worldTransform.pos + (collider.second.CollisionObject->m_worldTransform.rot * collider.second.collisionCapsules[k].End1_offset100) + VirtualOffset;
			collider.second.collisionCapsules[k].End1_radius100 = collider.second.collisionCapsules[k].End1_radius0 * collider.second.actorBaseScale * colliderNodescale;
			collider.second.collisionCapsules[k].End1_radius100pwr2 = collider.second.collisionCapsules[k].End1_radius100 * collider.second.collisionCapsules[k].End1_radius100;
			collider.second.collisionCapsules[k].End2_offset100 = collider.second.collisionCapsules[k].End2_offset0 * collider.second.actorBaseScale * colliderNodescale;
			collider.second.collisionCapsules[k].End2_worldPos = collider.second.CollisionObject->m_worldTransform.pos + (collider.second.CollisionObject->m_worldTransform.rot * collider.second.collisionCapsules[k].End2_offset100) + VirtualOffset;
			collider.second.collisionCapsules[k].End2_radius100 = collider.second.collisionCapsules[k].End2_radius0 * collider.second.actorBaseScale * colliderNodescale;
			collider.second.collisionCapsules[k].End2_radius100pwr2 = collider.second.collisionCapsules[k].End2_radius100 * collider.second.collisionCapsules[k].End2_radius100;
		}

#ifdef RUNTIME_VR_VERSION_1_4_15
		for (int j = 0; j < collider.second.collisionTriangles.size(); j++)
		{
			collider.second.collisionTriangles[j].a = collider.second.CollisionObject->m_worldTransform.pos + collider.second.CollisionObject->m_worldTransform.rot * collider.second.collisionTriangles[j].orga * colliderNodescale;
			collider.second.collisionTriangles[j].b = collider.second.CollisionObject->m_worldTransform.pos + collider.second.CollisionObject->m_worldTransform.rot * collider.second.collisionTriangles[j].orgb * colliderNodescale;
			collider.second.collisionTriangles[j].c = collider.second.CollisionObject->m_worldTransform.pos + collider.second.CollisionObject->m_worldTransform.rot * collider.second.collisionTriangles[j].orgc * colliderNodescale;
		}
#endif
	});
}

std::vector<int> GetHashIdsFromPos(NiPoint3 pos, float radiusplus)
{
	//float radiusplus = radius + 1.0f; //1 is enough now.

	std::vector<int> hashIdList;
	
	int hashId = CreateHashId(pos.x, pos.y, pos.z, gridsize, actorDistance);
	//LOG_INFO("hashId=%d", hashId);
	if (hashId >= 0)
		hashIdList.emplace_back(hashId);

	bool xPlus = false;
	bool xMinus = false;
	bool yPlus = false;
	bool yMinus = false;
	bool zPlus = false;
	bool zMinus = false;

	hashId = CreateHashId(pos.x + radiusplus, pos.y, pos.z, gridsize, actorDistance);
	//LOG_INFO("hashId=%d", hashId);
	if (hashId >= 0)
	{
		if (!(std::find(hashIdList.begin(), hashIdList.end(), hashId) != hashIdList.end()))
		{
			xPlus = true;
			hashIdList.emplace_back(hashId);
		}
	}

	hashId = CreateHashId(pos.x - radiusplus, pos.y, pos.z, gridsize, actorDistance);
	//LOG_INFO("hashId=%d", hashId);
	if (hashId >= 0)
	{
		if (!(std::find(hashIdList.begin(), hashIdList.end(), hashId) != hashIdList.end()))
		{
			xMinus = true;
			hashIdList.emplace_back(hashId);
		}
	}

	hashId = CreateHashId(pos.x, pos.y + radiusplus, pos.z, gridsize, actorDistance);
	//LOG_INFO("hashId=%d", hashId);
	if (hashId >= 0)
	{
		if (!(std::find(hashIdList.begin(), hashIdList.end(), hashId) != hashIdList.end()))
		{
			yPlus = true;
			hashIdList.emplace_back(hashId);
		}
	}

	hashId = CreateHashId(pos.x, pos.y - radiusplus, pos.z, gridsize, actorDistance);
	//LOG_INFO("hashId=%d", hashId);
	if (hashId >= 0)
	{
		if (!(std::find(hashIdList.begin(), hashIdList.end(), hashId) != hashIdList.end()))
		{
			yMinus = true;
			hashIdList.emplace_back(hashId);
		}
	}

	hashId = CreateHashId(pos.x, pos.y, pos.z + radiusplus, gridsize, actorDistance);
	//LOG_INFO("hashId=%d", hashId);
	if (hashId >= 0)
	{
		if (!(std::find(hashIdList.begin(), hashIdList.end(), hashId) != hashIdList.end()))
		{
			zPlus = true;
			hashIdList.emplace_back(hashId);
		}
	}

	hashId = CreateHashId(pos.x, pos.y, pos.z - radiusplus, gridsize, actorDistance);
	//LOG_INFO("hashId=%d", hashId);
	if (hashId >= 0)
	{
		if (!(std::find(hashIdList.begin(), hashIdList.end(), hashId) != hashIdList.end()))
		{
			zMinus = true;
			hashIdList.emplace_back(hashId);
		}
	}

	if (xPlus && yPlus)
	{
		hashId = CreateHashId(pos.x + radiusplus, pos.y + radiusplus, pos.z, gridsize,  actorDistance);
		//LOG_INFO("hashId=%d", hashId);
		if (hashId >= 0)
		{
			if (!(std::find(hashIdList.begin(), hashIdList.end(), hashId) != hashIdList.end()))
			{
				hashIdList.emplace_back(hashId);
			}
		}

		if (xPlus && yPlus && zPlus)
		{
			hashId = CreateHashId(pos.x + radiusplus, pos.y + radiusplus, pos.z + radiusplus, gridsize, actorDistance);
			//LOG_INFO("hashId=%d", hashId);
			if (hashId >= 0)
			{
				if (!(std::find(hashIdList.begin(), hashIdList.end(), hashId) != hashIdList.end()))
				{
					hashIdList.emplace_back(hashId);
				}
			}
		}
		else if (xPlus && yPlus && zMinus)
		{
			hashId = CreateHashId(pos.x + radiusplus, pos.y + radiusplus, pos.z - radiusplus, gridsize, actorDistance);
			//LOG_INFO("hashId=%d", hashId);
			if (hashId >= 0)
			{
				if (!(std::find(hashIdList.begin(), hashIdList.end(), hashId) != hashIdList.end()))
				{
					hashIdList.emplace_back(hashId);
				}
			}
		}
	}
	else if (xMinus && yPlus)
	{
		hashId = CreateHashId(pos.x - radiusplus, pos.y + radiusplus, pos.z, gridsize, actorDistance);
		//LOG_INFO("hashId=%d", hashId);
		if (hashId >= 0)
		{
			if (!(std::find(hashIdList.begin(), hashIdList.end(), hashId) != hashIdList.end()))
			{
				hashIdList.emplace_back(hashId);
			}
		}

		if (xMinus && yPlus && zMinus)
		{
			hashId = CreateHashId(pos.x - radiusplus, pos.y + radiusplus, pos.z - radiusplus, gridsize, actorDistance);
			//LOG_INFO("hashId=%d", hashId);
			if (hashId >= 0)
			{
				if (!(std::find(hashIdList.begin(), hashIdList.end(), hashId) != hashIdList.end()))
				{
					hashIdList.emplace_back(hashId);
				}
			}
		}
		else if (xMinus && yPlus && zPlus)
		{
			hashId = CreateHashId(pos.x - radiusplus, pos.y + radiusplus, pos.z + radiusplus, gridsize, actorDistance);
			//LOG_INFO("hashId=%d", hashId);
			if (hashId >= 0)
			{
				if (!(std::find(hashIdList.begin(), hashIdList.end(), hashId) != hashIdList.end()))
				{
					hashIdList.emplace_back(hashId);
				}
			}
		}
	}
	else if (xPlus && yMinus)
	{
		hashId = CreateHashId(pos.x + radiusplus, pos.y - radiusplus, pos.z, gridsize,  actorDistance);
		//LOG_INFO("hashId=%d", hashId);
		if (hashId >= 0)
		{
			if (!(std::find(hashIdList.begin(), hashIdList.end(), hashId) != hashIdList.end()))
			{
				hashIdList.emplace_back(hashId);
			}
		}

		if (xPlus && yMinus && zMinus)
		{
			hashId = CreateHashId(pos.x + radiusplus, pos.y - radiusplus, pos.z - radiusplus, gridsize, actorDistance);
			//LOG_INFO("hashId=%d", hashId);
			if (hashId >= 0)
			{
				if (!(std::find(hashIdList.begin(), hashIdList.end(), hashId) != hashIdList.end()))
				{
					hashIdList.emplace_back(hashId);
				}
			}
		}
		else if (xPlus && yMinus && zPlus)
		{
			hashId = CreateHashId(pos.x + radiusplus, pos.y - radiusplus, pos.z + radiusplus, gridsize, actorDistance);
			//LOG_INFO("hashId=%d", hashId);
			if (hashId >= 0)
			{
				if (!(std::find(hashIdList.begin(), hashIdList.end(), hashId) != hashIdList.end()))
				{
					hashIdList.emplace_back(hashId);
				}
			}
		}
	}
	else if (xMinus && yMinus)
	{
		hashId = CreateHashId(pos.x - radiusplus, pos.y - radiusplus, pos.z, gridsize, actorDistance);
		//LOG_INFO("hashId=%d", hashId);
		if (hashId >= 0)
		{
			if (!(std::find(hashIdList.begin(), hashIdList.end(), hashId) != hashIdList.end()))
			{
				hashIdList.emplace_back(hashId);
			}
		}

		if (xMinus && yMinus && zMinus)
		{
			hashId = CreateHashId(pos.x - radiusplus, pos.y - radiusplus, pos.z - radiusplus, gridsize, actorDistance);
			//LOG_INFO("hashId=%d", hashId);
			if (hashId >= 0)
			{
				if (!(std::find(hashIdList.begin(), hashIdList.end(), hashId) != hashIdList.end()))
				{
					hashIdList.emplace_back(hashId);
				}
			}
		}
		else if (xMinus && yMinus && zPlus)
		{
			hashId = CreateHashId(pos.x - radiusplus, pos.y - radiusplus, pos.z + radiusplus, gridsize, actorDistance);
			//LOG_INFO("hashId=%d", hashId);
			if (hashId >= 0)
			{
				if (!(std::find(hashIdList.begin(), hashIdList.end(), hashId) != hashIdList.end()))
				{
					hashIdList.emplace_back(hashId);
				}
			}
		}
	}

	if (xPlus && zPlus)
	{
		hashId = CreateHashId(pos.x + radiusplus, pos.y, pos.z + radiusplus, gridsize, actorDistance);
		//LOG_INFO("hashId=%d", hashId);
		if (hashId >= 0)
		{
			if (!(std::find(hashIdList.begin(), hashIdList.end(), hashId) != hashIdList.end()))
			{
				hashIdList.emplace_back(hashId);
			}
		}
	}
	else if (xMinus && zPlus)
	{
		hashId = CreateHashId(pos.x - radiusplus, pos.y, pos.z + radiusplus, gridsize, actorDistance);
		//LOG_INFO("hashId=%d", hashId);
		if (hashId >= 0)
		{
			if (!(std::find(hashIdList.begin(), hashIdList.end(), hashId) != hashIdList.end()))
			{
				hashIdList.emplace_back(hashId);
			}
		}
	}
	else if (xPlus && zMinus)
	{
		hashId = CreateHashId(pos.x + radiusplus, pos.y, pos.z - radiusplus, gridsize, actorDistance);
		//LOG_INFO("hashId=%d", hashId);
		if (hashId >= 0)
		{
			if (!(std::find(hashIdList.begin(), hashIdList.end(), hashId) != hashIdList.end()))
			{
				hashIdList.emplace_back(hashId);
			}
		}
	}
	else if (xMinus && zMinus)
	{
		hashId = CreateHashId(pos.x - radiusplus, pos.y, pos.z - radiusplus, gridsize, actorDistance);
		//LOG_INFO("hashId=%d", hashId);
		if (hashId >= 0)
		{
			if (!(std::find(hashIdList.begin(), hashIdList.end(), hashId) != hashIdList.end()))
			{
				hashIdList.emplace_back(hashId);
			}
		}
	}

	if (yPlus && zPlus)
	{
		hashId = CreateHashId(pos.x, pos.y + radiusplus, pos.z + radiusplus, gridsize, actorDistance);
		//LOG_INFO("hashId=%d", hashId);
		if (hashId >= 0)
		{
			if (!(std::find(hashIdList.begin(), hashIdList.end(), hashId) != hashIdList.end()))
			{
				hashIdList.emplace_back(hashId);
			}
		}
	}
	else if (yMinus && zPlus)
	{
		hashId = CreateHashId(pos.x, pos.y - radiusplus, pos.z + radiusplus, gridsize, actorDistance);
		//LOG_INFO("hashId=%d", hashId);
		if (hashId >= 0)
		{
			if (!(std::find(hashIdList.begin(), hashIdList.end(), hashId) != hashIdList.end()))
			{
				hashIdList.emplace_back(hashId);
			}
		}
	}
	else if (yPlus && zMinus)
	{
		hashId = CreateHashId(pos.x, pos.y + radiusplus, pos.z - radiusplus, gridsize, actorDistance);
		//LOG_INFO("hashId=%d", hashId);
		if (hashId >= 0)
		{
			if (!(std::find(hashIdList.begin(), hashIdList.end(), hashId) != hashIdList.end()))
			{
				hashIdList.emplace_back(hashId);
			}
		}
	}
	else if (yMinus && zMinus)
	{
		hashId = CreateHashId(pos.x, pos.y - radiusplus, pos.z - radiusplus, gridsize, actorDistance);
		//LOG_INFO("hashId=%d", hashId);
		if (hashId >= 0)
		{
			if (!(std::find(hashIdList.begin(), hashIdList.end(), hashId) != hashIdList.end()))
			{
				hashIdList.emplace_back(hashId);
			}
		}
	}

	return hashIdList;
}

int GetHashIdFromPos(NiPoint3 pos)
{	
	int hashId = CreateHashId(pos.x, pos.y, pos.z, gridsize, actorDistance);
	if (hashId >= 0)
		return hashId;
	else
		return -1;

	/*hashId = unsigned(floor((pos.x+radius) / gridsize)*a + floor(pos.y / gridsize)*b + floor(pos.z / gridsize)*c) % size;
	if (hashId < size && hashId >= 0)
		hashIdList.emplace_back(hashId);

	hashId = unsigned(floor((pos.x - radius) / gridsize)*a + floor(pos.y / gridsize)*b + floor(pos.z / gridsize)*c) % size;
	if (hashId < size && hashId >= 0)
		hashIdList.emplace_back(hashId);*/


}