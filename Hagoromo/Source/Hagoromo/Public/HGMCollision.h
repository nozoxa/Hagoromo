// Hagoromo : Copyright (c) 2025 nozoxa_0131, MIT License

#pragma once

#include "HGMMath.h"

#include "Animation/BoneReference.h"

#include "HGMCollision.generated.h"

class UPhysicsAsset;
struct FHGMPhysicsContext;
struct FHGMPhysicsSettings;
struct FHGMSIMDStructure;
struct FHGMSimulationPlane;
struct FComponentSpacePoseContext;

struct FHGMBoneSpaceSphereCollider
{
	FBoneReference DriverBone {};
	FHGMVector3 Center { FHGMVector3::ZeroVector };
	FHGMReal Radius { 0.0 };
};


struct FHGMBoneSpaceCapsuleCollider
{
	FBoneReference DriverBone {};
	FHGMVector3 StartPoint { FHGMVector3::ZeroVector };
	FHGMVector3 EndPoint { FHGMVector3::ZeroVector };
	FHGMReal Radius { 0.0 };
};


struct FHGMSphereCollider
{
	FHGMVector3 Center { FHGMVector3::ZeroVector };
	FHGMReal Radius { 0.0 };
	bool bEnabled = true;
};


struct FHGMCapsuleCollider
{
	FHGMVector3 StartPoint { FHGMVector3::ZeroVector };
	FHGMVector3 EndPoint { FHGMVector3::ZeroVector };
	FHGMReal Radius { 0.0 };
	bool bEnabled = true;
};


struct FHGMSIMDSphereCollider
{
	FHGMSIMDSphereCollider()
	{
	}

	FHGMSIMDSphereCollider(const FHGMSIMDVector3& sCenter, const FHGMSIMDReal& sRadius)
		: sCenter(sCenter)
		, sRadius(sRadius)
	{
	}

	FHGMSIMDSphereCollider(const FHGMSphereCollider& SphereCollider)
	{
		FHGMSIMDLibrary::Load(this->sCenter, SphereCollider.Center);
		FHGMSIMDLibrary::Load(this->sRadius, SphereCollider.Radius);
	}

	FHGMSIMDVector3 sCenter = FHGMSIMDVector3::ZeroVector;
	FHGMSIMDReal sRadius = HGMSIMDConstants::ZeroReal;
};


struct FHGMSIMDCapsuleCollider
{
	FHGMSIMDCapsuleCollider()
	{
	}

	FHGMSIMDCapsuleCollider(const FHGMCapsuleCollider& CapsuleCollider)
	{
		FHGMSIMDLibrary::Load(this->sStartPoint, CapsuleCollider.StartPoint);
		FHGMSIMDLibrary::Load(this->sEndPoint, CapsuleCollider.EndPoint);
		FHGMSIMDLibrary::Load(this->sRadius, CapsuleCollider.Radius);
	}

	FHGMSIMDCapsuleCollider(const FHGMSIMDVector3& sStartPoint, const FHGMSIMDVector3& sEndPoint, const FHGMSIMDReal& sRadius)
		: sStartPoint(sStartPoint)
		, sEndPoint(sEndPoint)
		, sRadius(sRadius)
	{
	}

	FHGMSIMDVector3 sStartPoint = FHGMSIMDVector3::ZeroVector;
	FHGMSIMDVector3 sEndPoint = FHGMSIMDVector3::ZeroVector;
	FHGMSIMDReal sRadius = HGMSIMDConstants::ZeroReal;
};


USTRUCT()
struct FHGMPlaneCollider
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, Category = "")
	FBoneReference DrivingBone {};

	UPROPERTY(EditDefaultsOnly, Category = "")
	FVector LocationOffset = FVector::ZeroVector;

	UPROPERTY(EditDefaultsOnly, Category = "", meta = (ClampMin = "-360.0", ClampMax = "360.0"))
	FRotator RotationOffset = FRotator::ZeroRotator;
};


struct FHGMSIMDPlaneCollider
{
	FHGMSIMDVector3 sOrigin {};
	FHGMSIMDQuaternion sRotation {};
};


struct FHGMBodyCollider
{
	TArray<FHGMBoneSpaceSphereCollider> BoneSpaceSphereColliders {};
	TArray<FHGMBoneSpaceCapsuleCollider> BoneSpaceCapsuleColliders {};
	TArray<FHGMSphereCollider> SphereColliders {};
	TArray<FHGMCapsuleCollider> CapsuleColliders {};
};


// Data to separate colliding colliders.
struct FHGMSIMDColliderContact
{
	int32 PackedIndex;
	// Added to determine which bones to apply friction to.
	FHGMSIMDReal sHitMask;
	// Stores values that work without HitMask to reduce computational load by SIMD.
	// For example, collision-free register stores ZeroVector, ZeroReal.
	FHGMSIMDVector3 sSeparatingNormal;
	FHGMSIMDReal sSeparatingOffset;
};


// ---------------------------------------------------------------------------------------
// CollisionLibrary
// ---------------------------------------------------------------------------------------
struct FHGMCollisionLibrary
{
	static void InitializeBodyColliderFromPhysicsAsset(const FBoneContainer& RequiredBones, UPhysicsAsset* PhysicsAsset, FHGMBodyCollider& OutBodyCollider);
	static void UpdateBodyCollider(FComponentSpacePoseContext& Output, const FHGMPhysicsContext& PhysicsContext, FHGMBodyCollider& BodyCollider, FHGMBodyCollider& PrevBodyCollider);
	static void CalculateBodyColliderContacts(TConstArrayView<FHGMSIMDReal> BoneSphereColliderRadiuses, TArrayView<FHGMSIMDVector3> Positions, TArrayView<FHGMSIMDVector3> PrevPositions, const FHGMBodyCollider& BodyCollider, const FHGMBodyCollider& PrevBodyCollider, TArray<FHGMSIMDColliderContact>& OutContacts);
	static void CalculateBodyColliderContactsForVerticalEdge(TConstArrayView<FHGMSIMDStructure> VerticalStructures, TArrayView<FHGMSIMDVector3> Positions, const FHGMBodyCollider& BodyCollider, TArray<FHGMSIMDColliderContact>& OutContacts);
	static void CalculateBodyColliderContactsForHorizontalEdge(FHGMSimulationPlane& SimulationPlane, TConstArrayView<FHGMSIMDStructure> HorizontalStructures, TArray<FHGMSIMDVector3>& Positions, const FHGMBodyCollider& BodyCollider, TArray<FHGMSIMDColliderContact>& OutContacts);

	static void InitializePlaneColliders(const FBoneContainer& RequiredBones, TArrayView<FHGMPlaneCollider> PlaneColliders, TArray<FHGMSIMDPlaneCollider>& OutPlaneColliders);
	static void UpdatePlaneColliders(FComponentSpacePoseContext& Output, TConstArrayView<FHGMPlaneCollider> PlaneColliders, TArrayView<FHGMSIMDPlaneCollider> OutUpdatedPlaneColliders);
	static void CalculatePlaneColliderContacts(TConstArrayView<FHGMSIMDPlaneCollider> PlaneColliders, TConstArrayView<FHGMSIMDReal> BoneSphereColliderRadiuses, TArray<FHGMSIMDVector3>& Positions, TArray<FHGMSIMDColliderContact>& OutContacts);
};
