#include "Collision.h"

Collision::Collision(NiAVObject* node, std::vector<Sphere> &colliderspheres, std::vector<Capsule> &collidercapsules, float actorWeight)
{
	CollisionObject = node;

	ColliderWeight = actorWeight;
	
	collisionSpheres = colliderspheres;
	for (int j = 0; j < collisionSpheres.size(); j++)
	{
		collisionSpheres[j].offset100 = GetPointFromPercentage(colliderspheres[j].offset0, colliderspheres[j].offset100, ColliderWeight) * node->m_worldTransform.scale;

		collisionSpheres[j].radius100 = GetPercentageValue(colliderspheres[j].radius0, colliderspheres[j].radius100, ColliderWeight) * node->m_worldTransform.scale;
		
		collisionSpheres[j].radius100pwr2 = collisionSpheres[j].radius100 * collisionSpheres[j].radius100;

		collisionSpheres[j].NodeName = colliderspheres[j].NodeName;
	}

	collisionCapsules = collidercapsules;
	for (int k = 0; k < collisionCapsules.size(); k++)
	{
		collisionCapsules[k].End1_offset100 = GetPointFromPercentage(collidercapsules[k].End1_offset0, collidercapsules[k].End1_offset100, ColliderWeight) * node->m_worldTransform.scale;
		
		collisionCapsules[k].End1_radius100 = GetPercentageValue(collidercapsules[k].End1_radius0, collidercapsules[k].End1_radius100, ColliderWeight) * node->m_worldTransform.scale;
		
		collisionCapsules[k].End1_radius100pwr2 = collisionCapsules[k].End1_radius100 * collisionCapsules[k].End1_radius100;
	
		collisionCapsules[k].End2_offset100 = GetPointFromPercentage(collidercapsules[k].End2_offset0, collidercapsules[k].End2_offset100, ColliderWeight) * node->m_worldTransform.scale;

		collisionCapsules[k].End2_radius100 = GetPercentageValue(collidercapsules[k].End2_radius0, collidercapsules[k].End2_radius100, ColliderWeight) * node->m_worldTransform.scale;
		
		collisionCapsules[k].End2_radius100pwr2 = collisionCapsules[k].End2_radius100 * collisionCapsules[k].End2_radius100;

		collisionCapsules[k].NodeName = collidercapsules[k].NodeName;
	}
}
#ifdef RUNTIME_VR_VERSION_1_4_15

void HandHapticFeedbackEffect(bool left)
{
	if (*g_openVR)
	{
		for (int i = 0; i < hapticStrength; i++)
		{
			//ivrSystem->TriggerHapticPulse(controller, 0, hapticFrequency);
			LOG_INFO("Triggering haptic pulse for %g on %s...", (float)hapticFrequency, left ? "left hand":"right hand");
			(*g_openVR)->TriggerHapticPulse(left ? BSVRInterface::kControllerHand_Left : BSVRInterface::kControllerHand_Right, (float)hapticFrequency / 3999.0f);
		}
	}
	else
	{
		LOG("g_openVR is false...");
	}
}
#endif
bool Collision::IsItColliding(NiPoint3 &collisiondif, std::vector<Sphere> &thingCollisionSpheres, std::vector<Sphere> &collisionSpheres, std::vector<Capsule> &thingCollisionCapsules, std::vector<Capsule> &collisionCapsules, bool maybe)
{	
	/*LARGE_INTEGER startingTime, endingTime, elapsedMicroseconds;
	LARGE_INTEGER frequency;

	QueryPerformanceFrequency(&frequency);
	QueryPerformanceCounter(&startingTime);
	LOG("Collision.IsItColliding() Start");*/
	bool isItColliding = false;
	NiPoint3 lastcollisionVector = emptyPoint;
	NiPoint3 collisionVector = emptyPoint;

	bool IsHandHapticFeedbackL = false;
	bool IsHandHapticFeedbackR = false;

	for(int j = 0; j < thingCollisionSpheres.size(); j++) 
	{
		NiPoint3 thingSpherePosition = thingCollisionSpheres[j].worldPos;

		for (int i = 0; i < collisionSpheres.size(); i++) //sphere X sphere
		{			
			float limitDistance = thingCollisionSpheres[j].radius100 + collisionSpheres[i].radius100;

			NiPoint3 colSpherePosition = collisionSpheres[i].worldPos;
			
			float currentDistancePwr2 = distanceNoSqrt(thingSpherePosition, colSpherePosition);

			if (currentDistancePwr2 < limitDistance * limitDistance)
			{
				isItColliding = true;
				if (maybe)
					return true;

				float currentDistance = std::sqrt(currentDistancePwr2);
				double Scalar = limitDistance - currentDistance; //Get vector scalar

				collisionVector = collisionVector + GetVectorFromCollision(colSpherePosition, thingSpherePosition, Scalar, currentDistance); //Get collision vector

				if (!CompareNiPoints(lastcollisionVector, collisionVector))
				{
					for (int l = 0; l < thingCollisionSpheres.size(); l++)
					{
						thingCollisionSpheres[l].worldPos = thingCollisionSpheres[l].worldPos - lastcollisionVector + collisionVector;
					}

					for (int m = 0; m < thingCollisionCapsules.size(); m++)
					{
						thingCollisionCapsules[m].End1_worldPos = thingCollisionCapsules[m].End1_worldPos - lastcollisionVector + collisionVector;
						thingCollisionCapsules[m].End2_worldPos = thingCollisionCapsules[m].End2_worldPos - lastcollisionVector + collisionVector;
					}
				}
				lastcollisionVector = collisionVector;

				#ifdef RUNTIME_VR_VERSION_1_4_15
				if (collisionSpheres[i].NodeName == "LeftWandNode")
				{
					IsHandHapticFeedbackL = true;
				}
				else if (collisionSpheres[i].NodeName == "RightWandNode")
				{
					IsHandHapticFeedbackR = true;
				}
				#endif
			}			
		}
		
		for (int k = 0; k < collisionCapsules.size(); k++) //sphere X capsule
		{
			NiPoint3 bestPoint = ClosestPointOnLineSegment(collisionCapsules[k].End1_worldPos, collisionCapsules[k].End2_worldPos, thingCollisionSpheres[j].worldPos);

			float twoPointDistancePwr2 = distanceNoSqrt(collisionCapsules[k].End1_worldPos, collisionCapsules[k].End2_worldPos);

			float bestPointDistancePwr2 = distanceNoSqrt(collisionCapsules[k].End1_worldPos, bestPoint);

			float PointWeight = bestPointDistancePwr2 / twoPointDistancePwr2 * 100;

			float limitDistance = GetPercentageValue(collisionCapsules[k].End1_radius100, collisionCapsules[k].End2_radius100, PointWeight) + thingCollisionSpheres[j].radius100;

			float currentDistancePwr2 = distanceNoSqrt(thingCollisionSpheres[j].worldPos, bestPoint);

			if (currentDistancePwr2 < limitDistance * limitDistance)
			{
				isItColliding = true;
				if (maybe)
					return true;

				float currentDistance = std::sqrt(currentDistancePwr2);
				double Scalar = limitDistance - currentDistance; //Get vector scalar

				collisionVector = collisionVector + GetVectorFromCollision(bestPoint, thingCollisionSpheres[j].worldPos, Scalar, currentDistance); //Get collision vector
			
				if (!CompareNiPoints(lastcollisionVector, collisionVector))
				{
					for (int l = 0; l < thingCollisionSpheres.size(); l++)
					{
						thingCollisionSpheres[l].worldPos = thingCollisionSpheres[l].worldPos - lastcollisionVector + collisionVector;
					}

					for (int m = 0; m < thingCollisionCapsules.size(); m++)
					{
						thingCollisionCapsules[m].End1_worldPos = thingCollisionCapsules[m].End1_worldPos - lastcollisionVector + collisionVector;
						thingCollisionCapsules[m].End2_worldPos = thingCollisionCapsules[m].End2_worldPos - lastcollisionVector + collisionVector;
					}
				}
				lastcollisionVector = collisionVector;

				#ifdef RUNTIME_VR_VERSION_1_4_15
				if (collisionCapsules[k].NodeName == "LeftWandNode")
				{
					IsHandHapticFeedbackL = true;
				}
				else if (collisionCapsules[k].NodeName == "RightWandNode")
				{
					IsHandHapticFeedbackR = true;
				}
				#endif
			}
		}
	}
	
	for (int n = 0; n < thingCollisionCapsules.size(); n++)
	{
		for (int l = 0; l < collisionSpheres.size(); l++) //capsule X sphere
		{
			NiPoint3 bestPoint = ClosestPointOnLineSegment(thingCollisionCapsules[n].End1_worldPos, thingCollisionCapsules[n].End2_worldPos, collisionSpheres[l].worldPos);

			float twoPointDistancePwr2 = distanceNoSqrt(thingCollisionCapsules[n].End1_worldPos, thingCollisionCapsules[n].End2_worldPos);

			float bestPointDistancePwr2 = distanceNoSqrt(thingCollisionCapsules[n].End1_worldPos, bestPoint);

			float PointWeight = bestPointDistancePwr2 / twoPointDistancePwr2 * 100;

			float limitDistance = GetPercentageValue(thingCollisionCapsules[n].End1_radius100, thingCollisionCapsules[n].End2_radius100, PointWeight) + collisionSpheres[l].radius100;

			float currentDistancePwr2 = distanceNoSqrt(bestPoint, collisionSpheres[l].worldPos);

			if (currentDistancePwr2 < limitDistance * limitDistance)
			{
				isItColliding = true;
				if (maybe)
					return true;

				float currentDistance = std::sqrt(currentDistancePwr2);
				double Scalar = limitDistance - currentDistance; //Get vector scalar

				collisionVector = collisionVector + GetVectorFromCollision(collisionSpheres[l].worldPos, bestPoint, Scalar, currentDistance); //Get collision vector
			
				if (!CompareNiPoints(lastcollisionVector, collisionVector))
				{
					for (int l = 0; l < thingCollisionSpheres.size(); l++)
					{
						thingCollisionSpheres[l].worldPos = thingCollisionSpheres[l].worldPos - lastcollisionVector + collisionVector;
					}

					for (int m = 0; m < thingCollisionCapsules.size(); m++)
					{
						thingCollisionCapsules[m].End1_worldPos = thingCollisionCapsules[m].End1_worldPos - lastcollisionVector + collisionVector;
						thingCollisionCapsules[m].End2_worldPos = thingCollisionCapsules[m].End2_worldPos - lastcollisionVector + collisionVector;
					}
				}
				lastcollisionVector = collisionVector;

				#ifdef RUNTIME_VR_VERSION_1_4_15
				if (collisionSpheres[l].NodeName == "LeftWandNode")
				{
					IsHandHapticFeedbackL = true;
				}
				else if (collisionSpheres[l].NodeName == "RightWandNode")
				{
					IsHandHapticFeedbackR = true;
				}
				#endif
			}
		}

		for (int m = 0; m < collisionCapsules.size(); m++) //capsule X capsule
		{
			NiPoint3 v0 = collisionCapsules[m].End1_worldPos - thingCollisionCapsules[n].End1_worldPos;
			NiPoint3 v1 = collisionCapsules[m].End2_worldPos - thingCollisionCapsules[n].End1_worldPos;
			NiPoint3 v2 = collisionCapsules[m].End1_worldPos - thingCollisionCapsules[n].End2_worldPos;
			NiPoint3 v3 = collisionCapsules[m].End2_worldPos - thingCollisionCapsules[n].End2_worldPos;

			float d0 = Dot(v0, v0);
			float d1 = Dot(v1, v1);
			float d2 = Dot(v2, v2);
			float d3 = Dot(v3, v3);
			
			NiPoint3 bestPointA;
			if (d2 < d0 || d2 < d1 || d3 < d0 || d3 < d1)
			{
				bestPointA = thingCollisionCapsules[n].End2_worldPos;
			}
			else
			{
				bestPointA = thingCollisionCapsules[n].End1_worldPos;
			}

			NiPoint3 bestPointB = ClosestPointOnLineSegment(collisionCapsules[m].End1_worldPos, collisionCapsules[m].End2_worldPos, bestPointA);
			bestPointA = ClosestPointOnLineSegment(thingCollisionCapsules[n].End1_worldPos, thingCollisionCapsules[n].End2_worldPos, bestPointB);

			float twoPointADistancePwr2 = distanceNoSqrt(thingCollisionCapsules[n].End1_worldPos, thingCollisionCapsules[n].End2_worldPos);
			float twoPointBDistancePwr2 = distanceNoSqrt(collisionCapsules[m].End1_worldPos, collisionCapsules[m].End2_worldPos);

			float bestPointADistancePwr2 = distanceNoSqrt(thingCollisionCapsules[n].End1_worldPos, bestPointA);
			float bestPointBDistancePwr2 = distanceNoSqrt(collisionCapsules[m].End1_worldPos, bestPointB);

			float PointAWeight = bestPointADistancePwr2 / twoPointADistancePwr2 * 100;
			float PointBWeight = bestPointBDistancePwr2 / twoPointBDistancePwr2 * 100;

			float limitDistance = GetPercentageValue(thingCollisionCapsules[n].End1_radius100, thingCollisionCapsules[n].End2_radius100, PointAWeight)
				+ GetPercentageValue(collisionCapsules[m].End1_radius100, collisionCapsules[m].End2_radius100, PointBWeight);

			float currentDistancePwr2 = distanceNoSqrt(bestPointA, bestPointB);

			if (currentDistancePwr2 < limitDistance * limitDistance)
			{
				isItColliding = true;
				if (maybe)
					return true;

				float currentDistance = std::sqrt(currentDistancePwr2);
				double Scalar = limitDistance - currentDistance; //Get vector scalar

				collisionVector = collisionVector + GetVectorFromCollision(bestPointB, bestPointA, Scalar, currentDistance); //Get collision vector

				if (!CompareNiPoints(lastcollisionVector, collisionVector))
				{
					for (int l = 0; l < thingCollisionSpheres.size(); l++)
					{
						thingCollisionSpheres[l].worldPos = thingCollisionSpheres[l].worldPos - lastcollisionVector + collisionVector;
					}

					for (int m = 0; m < thingCollisionCapsules.size(); m++)
					{
						thingCollisionCapsules[m].End1_worldPos = thingCollisionCapsules[m].End1_worldPos - lastcollisionVector + collisionVector;
						thingCollisionCapsules[m].End2_worldPos = thingCollisionCapsules[m].End2_worldPos - lastcollisionVector + collisionVector;
					}
				}
				lastcollisionVector = collisionVector;

				#ifdef RUNTIME_VR_VERSION_1_4_15
				if (collisionCapsules[m].NodeName == "LeftWandNode")
				{
					IsHandHapticFeedbackL = true;
				}
				else if (collisionCapsules[m].NodeName == "RightWandNode")
				{
					IsHandHapticFeedbackR = true;
				}
				#endif
			}
		}
	}

	collisiondif = collisiondif + collisionVector;
	
#ifdef RUNTIME_VR_VERSION_1_4_15
	if (IsHandHapticFeedbackL)
	{
		HandHapticFeedbackEffect(true);
	}
	if (IsHandHapticFeedbackR)
	{
		HandHapticFeedbackEffect(false);
	}
#endif

	/*QueryPerformanceCounter(&endingTime);
	elapsedMicroseconds.QuadPart = endingTime.QuadPart - startingTime.QuadPart;
	elapsedMicroseconds.QuadPart *= 1000000000LL;
	elapsedMicroseconds.QuadPart /= frequency.QuadPart;
	LOG("Collision.IsItColliding() Update Time = %lld ns\n", elapsedMicroseconds.QuadPart);*/
	return isItColliding;
}

#ifdef RUNTIME_VR_VERSION_1_4_15
bool Collision::IsItCollidingTriangleToAffectedNodes(NiPoint3 &collisiondif, std::vector<Sphere> &thingCollisionSpheres, std::vector<Capsule>& thingCollisionCapsules, std::vector<Triangle> &collisionTriangles, bool maybe)
{
	/*LARGE_INTEGER startingTime, endingTime, elapsedMicroseconds;
	LARGE_INTEGER frequency;

	QueryPerformanceFrequency(&frequency);
	QueryPerformanceCounter(&startingTime);
	LOG("Collision.IsItCollidingTriangleToSphere() Start");*/
	bool isItColliding = false;

	NiPoint3 lastcollisionVector = emptyPoint;
	NiPoint3 collisionVector = emptyPoint;

	for (int k = 0; k < thingCollisionSpheres.size(); k++)
	{
		NiPoint3 thingSpherePosition = thingCollisionSpheres[k].worldPos;

		for (int i = 0; i < collisionTriangles.size(); i++)
		{
			//Phase One
			//Check lines to center of sphere distance

			Triangle t = collisionTriangles[i];
			/*t.a = colliderObjectTransform.pos + colliderObjectTransform.rot*t.a;
			t.b = colliderObjectTransform.pos + colliderObjectTransform.rot*t.b;
			t.c = colliderObjectTransform.pos + colliderObjectTransform.rot*t.c;*/

			NiPoint3 closestPoint = FindClosestPointOnTriangletoPoint(t, thingSpherePosition);

			float distToCenterPwr2 = distanceNoSqrt(closestPoint, thingSpherePosition);
			if (distToCenterPwr2 < thingCollisionSpheres[k].radius100pwr2)
			{
				// we have a collision
				isItColliding = true;
				if (maybe)
					return true;

				float distToCenter = sqrt(distToCenterPwr2);

				double Scalar = thingCollisionSpheres[k].radius100 - distToCenter; //Get vector scalar
				collisionVector = collisionVector + GetVectorFromCollision(closestPoint, thingSpherePosition, Scalar, distToCenter); //Get collision vector

				//old method
				//double difPercentage = ((thingCollisionSpheres[k].radius100 - distToCenter) / distToCenter) * 100;
				//collisionVector = collisionVector + GetPointFromPercentage(closestPoint, thingSpherePosition, difPercentage+100) - thingSpherePosition;

				if (!CompareNiPoints(lastcollisionVector, collisionVector))
				{
					for (int l = 0; l < thingCollisionSpheres.size(); l++)
					{
						thingCollisionSpheres[l].worldPos = thingCollisionSpheres[l].worldPos - lastcollisionVector + collisionVector;
					}

					for (int m = 0; m < thingCollisionCapsules.size(); m++)
					{
						thingCollisionCapsules[m].End1_worldPos = thingCollisionCapsules[m].End1_worldPos - lastcollisionVector + collisionVector;
						thingCollisionCapsules[m].End2_worldPos = thingCollisionCapsules[m].End2_worldPos - lastcollisionVector + collisionVector;
					}
				}
				lastcollisionVector = collisionVector;
			}
		}
	}

	for (int k = 0; k < thingCollisionCapsules.size(); k++)
	{
		for (int i = 0; i < collisionTriangles.size(); i++)
		{
			//Phase One
			//Check lines to end point of capsule distance

			Triangle t = collisionTriangles[i];
			/*t.a = colliderObjectTransform.pos + colliderObjectTransform.rot*t.a;
			t.b = colliderObjectTransform.pos + colliderObjectTransform.rot*t.b;
			t.c = colliderObjectTransform.pos + colliderObjectTransform.rot*t.c;*/

			NiPoint3 closestPointA = FindClosestPointOnTriangletoPoint(t, thingCollisionCapsules[k].End1_worldPos);
			NiPoint3 closestPointB = FindClosestPointOnTriangletoPoint(t, thingCollisionCapsules[k].End2_worldPos);

			NiPoint3 v0 = closestPointA - thingCollisionCapsules[k].End1_worldPos;
			NiPoint3 v1 = closestPointB - thingCollisionCapsules[k].End1_worldPos;
			NiPoint3 v2 = closestPointA - thingCollisionCapsules[k].End2_worldPos;
			NiPoint3 v3 = closestPointB - thingCollisionCapsules[k].End2_worldPos;

			float d0 = Dot(v0, v0);
			float d1 = Dot(v1, v1);
			float d2 = Dot(v2, v2);
			float d3 = Dot(v3, v3);

			NiPoint3 bestPointA;
			if (d2 < d0 || d2 < d1 || d3 < d0 || d3 < d1)
			{
				bestPointA = thingCollisionCapsules[k].End2_worldPos;
			}
			else
			{
				bestPointA = thingCollisionCapsules[k].End1_worldPos;
			}

			NiPoint3 bestPointB = ClosestPointOnLineSegment(closestPointA, closestPointB, bestPointA);
			bestPointA = ClosestPointOnLineSegment(thingCollisionCapsules[k].End1_worldPos, thingCollisionCapsules[k].End2_worldPos, bestPointB);

			float twoPointDistancePwr2 = distanceNoSqrt(thingCollisionCapsules[k].End1_worldPos, thingCollisionCapsules[k].End2_worldPos);

			float bestPointDistancePwr2 = distanceNoSqrt(thingCollisionCapsules[k].End1_worldPos, bestPointA);

			float PointWeight = bestPointDistancePwr2 / twoPointDistancePwr2 * 100;

			float limitDistance = GetPercentageValue(thingCollisionCapsules[k].End1_radius100, thingCollisionCapsules[k].End2_radius100, PointWeight);

			float distToCenterPwr2 = distanceNoSqrt(bestPointA, bestPointB);

			if (distToCenterPwr2 < limitDistance * limitDistance)
			{
				// we have a collision
				isItColliding = true;
				if (maybe)
					return true;

				float distToCenter = sqrt(distToCenterPwr2);

				double Scalar = limitDistance - distToCenter; //Get vector scalar
				collisionVector = collisionVector + GetVectorFromCollision(bestPointB, bestPointA, Scalar, distToCenter); //Get collision vector

				//old method
				//double difPercentage = ((thingCollisionSpheres[k].radius100 - distToCenter) / distToCenter) * 100;
				//collisionVector = collisionVector + GetPointFromPercentage(bestPointB, bestPointA, difPercentage + 100) - bestPointA;

				if (!CompareNiPoints(lastcollisionVector, collisionVector))
				{
					for (int l = 0; l < thingCollisionSpheres.size(); l++)
					{
						thingCollisionSpheres[l].worldPos = thingCollisionSpheres[l].worldPos - lastcollisionVector + collisionVector;
					}

					for (int m = 0; m < thingCollisionCapsules.size(); m++)
					{
						thingCollisionCapsules[m].End1_worldPos = thingCollisionCapsules[m].End1_worldPos - lastcollisionVector + collisionVector;
						thingCollisionCapsules[m].End2_worldPos = thingCollisionCapsules[m].End2_worldPos - lastcollisionVector + collisionVector;
					}
				}
				lastcollisionVector = collisionVector;
			}
		}
	}

	collisiondif = collisiondif + collisionVector;

	/*QueryPerformanceCounter(&endingTime);
	elapsedMicroseconds.QuadPart = endingTime.QuadPart - startingTime.QuadPart;
	elapsedMicroseconds.QuadPart *= 1000000000LL;
	elapsedMicroseconds.QuadPart /= frequency.QuadPart;
	LOG("Collision.IsItCollidingTriangleToSphere() Update Time = %lld ns\n", elapsedMicroseconds.QuadPart);*/
	return isItColliding;
}
#endif

NiPoint3 Collision::CheckPelvisCollision(std::vector<Sphere> &thingCollisionSpheres, std::vector<Capsule>& thingCollisionCapsules)
{
	/*LARGE_INTEGER startingTime, endingTime, elapsedMicroseconds;
	LARGE_INTEGER frequency;

	QueryPerformanceFrequency(&frequency);
	QueryPerformanceCounter(&startingTime);
	LOG("Collision.CheckPelvisCollision() Start");*/
	NiPoint3 collisionDiff = emptyPoint;

	if (CollisionObject != nullptr)
	{
		IsItColliding(collisionDiff, thingCollisionSpheres, collisionSpheres, thingCollisionCapsules, collisionCapsules, false);

		#ifdef RUNTIME_VR_VERSION_1_4_15
		if (collisionTriangles.size() > 0)
		{
			IsItCollidingTriangleToAffectedNodes(collisionDiff, thingCollisionSpheres, thingCollisionCapsules, collisionTriangles, false);
		}
		#endif
	}

	/*QueryPerformanceCounter(&endingTime);
	elapsedMicroseconds.QuadPart = endingTime.QuadPart - startingTime.QuadPart;
	elapsedMicroseconds.QuadPart *= 1000000000LL;
	elapsedMicroseconds.QuadPart /= frequency.QuadPart;
	LOG("Collision.CheckPelvisCollision() Update Time = %lld ns\n", elapsedMicroseconds.QuadPart);*/
	return collisionDiff;
}

NiPoint3 Collision::CheckCollision(bool &isItColliding, std::vector<Sphere> &thingCollisionSpheres, std::vector<Capsule>& thingCollisionCapsules, bool maybe)
{
	/*LARGE_INTEGER startingTime, endingTime, elapsedMicroseconds;
	LARGE_INTEGER frequency;

	QueryPerformanceFrequency(&frequency);
	QueryPerformanceCounter(&startingTime);
	LOG("Collision.CheckCollision() Start");*/
	NiPoint3 collisionDiff = emptyPoint;
	bool isColliding = false;
	if (CollisionObject != nullptr)
	{
		isColliding = IsItColliding(collisionDiff, thingCollisionSpheres, collisionSpheres, thingCollisionCapsules, collisionCapsules, maybe);
		if (isColliding)
		{
			isItColliding = true;
			if (maybe)
				return emptyPoint;
		}

		#ifdef RUNTIME_VR_VERSION_1_4_15
		if (collisionTriangles.size() > 0)
		{
			isColliding = IsItCollidingTriangleToAffectedNodes(collisionDiff, thingCollisionSpheres, thingCollisionCapsules, collisionTriangles, maybe);
			if (isColliding)
			{
				isItColliding = true;
				if (maybe)
					return emptyPoint;
			}
		}
		#endif
		/*
		if (isItColliding)
		{
			//float timeMultiplier = timeTick / (float)deltaT;

			//collisionDiff *= timeMultiplier;

			//if (lastColliderPosition.x != 0 || lastColliderPosition.y != 0 || lastColliderPosition.z != 0)
			//{
			//	float distanceInOneCall = distance(lastColliderPosition, CollisionObject->m_worldTransform.pos);
			//	//if (distanceInOneCall > 1)
			//		collisionDiff *= distanceInOneCall;
			//}

//			collisionDiff.x = clamp(collisionDiff.x, -maxOffset * testColOffsetMult, maxOffset * testColOffsetMult);
//			collisionDiff.y = clamp(collisionDiff.y, -maxOffset * testColOffsetMult, maxOffset * testColOffsetMult);
//			collisionDiff.z = clamp(collisionDiff.z, -maxOffset * testColOffsetMult, maxOffset * testColOffsetMult);
		}*/
		//lastColliderPosition = CollisionObject->m_worldTransform.pos;
	}

	/*QueryPerformanceCounter(&endingTime);
	elapsedMicroseconds.QuadPart = endingTime.QuadPart - startingTime.QuadPart;
	elapsedMicroseconds.QuadPart *= 1000000000LL;
	elapsedMicroseconds.QuadPart /= frequency.QuadPart;
	LOG("Collision.CheckCollision() Update Time = %lld ns\n", elapsedMicroseconds.QuadPart);*/
	return collisionDiff;
}

// Dot product of 2 vectors 
float Collision::Dot(NiPoint3 A, NiPoint3 B)
{
	float x1, y1, z1;
	x1 = A.x * B.x;
	y1 = A.y * B.y;
	z1 = A.z * B.z;
	return (x1 + y1 + z1);
}

NiPoint3 Collision::ClosestPointOnLineSegment(NiPoint3 lineStart, NiPoint3 lineEnd, NiPoint3 point)
{
	auto v = lineEnd - lineStart;
	auto t = Dot(point - lineStart, v) / Dot(v, v);
	t = max(t, 0.0f);
	t = min(t, 1.0f);
	return lineStart + v * t;
}

#ifdef RUNTIME_VR_VERSION_1_4_15
//A * B
NiPoint3 Collision::MultiplyVector(NiPoint3 A, NiPoint3 B)
{
	NiPoint3 result = emptyPoint;
	result.x = A.y * B.z - A.z * B.y;
	result.y = A.z * B.x - A.x * B.z;
	result.z = A.x * B.y - A.y * B.x;
	return result;
}

NiPoint3 Collision::DotProduct(NiPoint3 A, NiPoint3 B)
{
	NiPoint3 result = emptyPoint;
	result.x = A.x * B.x;
	result.y = A.y * B.y;
	result.z = A.z * B.z;
	return result;
}

NiPoint3 Collision::FindClosestPointOnTriangletoPoint(Triangle T, NiPoint3 P)
{
	/*LARGE_INTEGER startingTime, endingTime, elapsedMicroseconds;
	LARGE_INTEGER frequency;

	QueryPerformanceFrequency(&frequency);
	QueryPerformanceCounter(&startingTime);
	LOG("Collision.FindClosestPointOnTriangletoPoint() Start");*/
	NiPoint3 edge0 = T.b - T.a;
	NiPoint3 edge1 = T.c - T.a;
	NiPoint3 v0 = T.a - P;

	float a = Dot(edge0,edge0);
	float b = Dot(edge0,edge1);
	float c = Dot(edge1,edge1);
	float d = Dot(edge0,v0);
	float e = Dot(edge1,v0);

	float det = a*c - b*b;
	float s = b*e - c*d;
	float t = b*d - a*e;

	if (s + t < det)
	{
		if (s < 0.f)
		{
			if (t < 0.f)
			{
				if (d < 0.f)
				{
					s = clamp(-d / a, 0.f, 1.f);
					t = 0.f;
				}
				else
				{
					s = 0.f;
					t = clamp(-e / c, 0.f, 1.f);
				}
			}
			else
			{
				s = 0.f;
				t = clamp(-e / c, 0.f, 1.f);
			}
		}
		else if (t < 0.f)
		{
			s = clamp(-d / a, 0.f, 1.f);
			t = 0.f;
		}
		else
		{
			float invDet = 1.f / det;
			s *= invDet;
			t *= invDet;
		}
	}
	else
	{
		if (s < 0.f)
		{
			float tmp0 = b + d;
			float tmp1 = c + e;
			if (tmp1 > tmp0)
			{
				float numer = tmp1 - tmp0;
				float denom = a - 2 * b + c;
				s = clamp(numer / denom, 0.f, 1.f);
				t = 1 - s;
			}
			else
			{
				t = clamp(-e / c, 0.f, 1.f);
				s = 0.f;
			}
		}
		else if (t < 0.f)
		{
			if (a + d > b + e)
			{
				float numer = c + e - b - d;
				float denom = a - 2 * b + c;
				s = clamp(numer / denom, 0.f, 1.f);
				t = 1 - s;
			}
			else
			{
				s = clamp(-e / c, 0.f, 1.f);
				t = 0.f;
			}
		}
		else
		{
			float numer = c + e - b - d;
			float denom = a - 2 * b + c;
			s = clamp(numer / denom, 0.f, 1.f);
			t = 1.f - s;
		}
	}

	/*QueryPerformanceCounter(&endingTime);
	elapsedMicroseconds.QuadPart = endingTime.QuadPart - startingTime.QuadPart;
	elapsedMicroseconds.QuadPart *= 1000000000LL;
	elapsedMicroseconds.QuadPart /= frequency.QuadPart;
	LOG("Collision.FindClosestPointOnTriangletoPoint() Update Time = %lld ns\n", elapsedMicroseconds.QuadPart);*/
	return T.a + edge0*s + edge1*t;
}
#endif

Collision::~Collision() {
}