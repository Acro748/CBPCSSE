#pragma once
#include <string>
#include <time.h>
#include <vector>
#include <ppl.h>
#include <vector>


#include "config.h"

#ifdef RUNTIME_VR_VERSION_1_4_15
#include "skse64/GameVR.h"
#endif

static NiPoint3 emptyPoint = NiPoint3(0, 0, 0);

struct CollisionConfigs {
	bool IsElasticCollision = false;

	NiPoint3 maybePos = emptyPoint;
	NiMatrix33 objRot;

	NiMatrix33 origRot;
	NiMatrix33 invRot;
	
	NiPoint3 CollisionMaxOffset = NiPoint3(100.0f, 100.0f, 100.0f);
	NiPoint3 CollisionMinOffset = NiPoint3(-100.0f, -100.0f, -100.0f);
};

void UpdateThingColliderPositions(NiPoint3& Collisiondif, std::vector<Sphere>& thingCollisionSpheres, std::vector<Capsule>& thingCollisionCapsules, CollisionConfigs CollisionConfig);

class Collision
{

public:

	Collision(NiAVObject* node, std::vector<Sphere> &colliderSpheres, std::vector<Capsule>& collidercapsules, float actorWeight);
	~Collision();

	float CollidedWeight = 50;

	float ColliderWeight = 50;

	float actorBaseScale = 1.0f;

	float scaleWeight = 1.0f;

	Actor* colliderActor;

	NiPoint3 lastColliderPosition = emptyPoint;

	bool IsItColliding(NiPoint3 &collisiondif, std::vector<Sphere> &thingCollisionSpheres, std::vector<Sphere> &collisionSpheres, std::vector<Capsule> &thingCollisionCapsules, std::vector<Capsule> &collisionCapsules, CollisionConfigs CollisionConfig, bool maybe);
	
	bool CheckCollision(NiPoint3 &collisionDiff, std::vector<Sphere>& thingCollisionSpheres, std::vector<Capsule>& thingCollisionCapsules, CollisionConfigs CollisionConfig, bool maybe);

	bool CheckPelvisCollision(NiPoint3 &collisionDiff, std::vector<Sphere> &thingCollisionSpheres, std::vector<Capsule>& thingCollisionCapsules, CollisionConfigs CollisionConfig);
	std::vector<Sphere> collisionSpheres;
	std::vector<Capsule> collisionCapsules;
	
	NiAVObject* CollisionObject;
	std::string colliderNodeName;

	float Dot(NiPoint3 A, NiPoint3 B);
	NiPoint3 ClosestPointOnLineSegment(NiPoint3 lineStart, NiPoint3 lineEnd, NiPoint3 point);

	#ifdef RUNTIME_VR_VERSION_1_4_15
	bool IsItCollidingTriangleToAffectedNodes(NiPoint3 &collisiondif, std::vector<Sphere> &thingCollisionSpheres, std::vector<Capsule>& thingCollisionCapsules, std::vector<Triangle> &collisionTriangle, CollisionConfigs CollisionConfig, bool maybe);

	std::vector<Triangle> collisionTriangles;
	
	NiPoint3 MultiplyVector(NiPoint3 A, NiPoint3 B);
	NiPoint3 DotProduct(NiPoint3 A, NiPoint3 B);
	NiPoint3 FindClosestPointOnTriangletoPoint(Triangle T, NiPoint3 P);
	#endif
};

static inline NiPoint3 GetPointFromPercentage(NiPoint3 lowWeight, NiPoint3 highWeight, float weight)
{
	return ((highWeight - lowWeight) * (weight * 0.01f)) + lowWeight;
}

static inline NiPoint3 GetVectorFromCollision(NiPoint3 col, NiPoint3 thing, float Scalar, float currentDistance)
{
	return (thing - col) / currentDistance * Scalar; // normalized vector * scalar
}

static inline float distance(NiPoint3 po1, NiPoint3 po2)
{
	/*LARGE_INTEGER startingTime, endingTime, elapsedMicroseconds;
	LARGE_INTEGER frequency;

	QueryPerformanceFrequency(&frequency);
	QueryPerformanceCounter(&startingTime);
	LOG("distance Start");*/
	float x = po1.x - po2.x;
	float y = po1.y - po2.y;
	float z = po1.z - po2.z;
	float result = std::sqrt(x*x + y*y + z*z);
	/*QueryPerformanceCounter(&endingTime);
	elapsedMicroseconds.QuadPart = endingTime.QuadPart - startingTime.QuadPart;
	elapsedMicroseconds.QuadPart *= 1000000000LL;
	elapsedMicroseconds.QuadPart /= frequency.QuadPart;
	LOG("distance Update Time = %lld ns\n", elapsedMicroseconds.QuadPart);*/
	return result;
}