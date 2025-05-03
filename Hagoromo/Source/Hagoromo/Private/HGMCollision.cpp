// Hagoromo : Copyright (c) 2025 nozoxa_0131, MIT License

#include "HGMCollision.h"
#include "HagoromoModule.h"
#include "HGMDebug.h"
#include "HGMSolvers.h"

#include "PhysicsEngine/PhysicsAsset.h"
#include "Animation/AnimNodeBase.h"

DECLARE_CYCLE_STAT(TEXT("Collision CalculateBodyColliderContacts"), STAT_CollisionCalculateBodyColliderContacts, STATGROUP_Hagoromo);
DECLARE_CYCLE_STAT(TEXT("Collision CalculateBodyColliderContactsForVerticalEdge"), STAT_CollisionCalculateBodyColliderContactsForVerticalEdge, STATGROUP_Hagoromo);
DECLARE_CYCLE_STAT(TEXT("Collision CalculateBodyColliderContactsForHorizontalEdge"), STAT_CollisionCalculateBodyColliderContactsForHorizontalEdge, STATGROUP_Hagoromo);

namespace
{
	/**
	* Calculates the nearest point on a line segment.
	* Return value is squared distance between segment and sphere.
	*/
	FHGMSIMDReal ClosestPointPointSegment(const FHGMSIMDVector3& sP, const FHGMSIMDVector3& sSegmentStart, const FHGMSIMDVector3& sSegmentEnd, FHGMSIMDReal& sOutT, FHGMSIMDVector3& sOutProjectedPoint)
	{
		// Converted "ゲームプログラミングのためのリアルタイム衝突判定 > P128" to SIMD.
		const FHGMSIMDVector3 sAB = sSegmentEnd - sSegmentStart;
		FHGMSIMDReal sSafeLengthSquared = FHGMMathLibrary::DotProduct(sAB, sAB);
		sSafeLengthSquared = FHGMSIMDLibrary::Select(sSafeLengthSquared <= HGMSIMDConstants::ZeroReal, HGMSIMDConstants::OneReal, sSafeLengthSquared);
		sOutT = FHGMMathLibrary::DotProduct(sP - sSegmentStart, sAB) / sSafeLengthSquared;
		sOutT = FHGMMathLibrary::Clamp(sOutT, HGMSIMDConstants::ZeroReal, HGMSIMDConstants::OneReal);
		sOutProjectedPoint = sSegmentStart + (sOutT * sAB);

		return FHGMMathLibrary::DotProduct(sOutProjectedPoint - sP, sOutProjectedPoint - sP);
	}


	/**
	 * Calculates line segment and line segment nearest neighbor.
	 * Return value is squared distance between segment and segment.
	 */
	FHGMSIMDReal ClosestPointSegmentSegment(const FHGMSIMDVector3& sP1, const FHGMSIMDVector3& sQ1, const FHGMSIMDVector3& sP2, const FHGMSIMDVector3& sQ2,
											FHGMSIMDReal& sOutS, FHGMSIMDReal& sOutT, FHGMSIMDVector3& sOutC1, FHGMSIMDVector3& sOutC2)
	{
		// Converted "ゲームプログラミングのためのリアルタイム衝突判定 > P150" to SIMD.
		const FHGMSIMDVector3 sD1 = sQ1 - sP1;
		const FHGMSIMDVector3 sD2 = sQ2 - sP2;
		const FHGMSIMDVector3 sR = sP1 - sP2;
		const FHGMSIMDReal sA = FHGMMathLibrary::DotProduct(sD1, sD1);
		const FHGMSIMDReal sE = FHGMMathLibrary::DotProduct(sD2, sD2);
		const FHGMSIMDReal sF = FHGMMathLibrary::DotProduct(sD2, sR);

		const FHGMSIMDReal sSafeA = FHGMSIMDLibrary::Select(sA <= HGMSIMDConstants::SmallReal, HGMSIMDConstants::OneReal, sA);
		const FHGMSIMDReal sSafeE = FHGMSIMDLibrary::Select(sE <= HGMSIMDConstants::SmallReal, HGMSIMDConstants::OneReal, sE);
		const FHGMSIMDReal sSafeF = FHGMSIMDLibrary::Select(sF <= HGMSIMDConstants::SmallReal, HGMSIMDConstants::OneReal, sF);

		const FHGMSIMDReal sADegenerateMask = sA <= HGMSIMDConstants::SmallReal;
		const FHGMSIMDReal sEDegenerateMask = sE <= HGMSIMDConstants::SmallReal;
		const FHGMSIMDReal sBothDegenerateMask = sADegenerateMask & sEDegenerateMask;

		// Both are degenerating :
		FHGMSIMDReal sBothDegenerateS, sBothDegenerateT {};
		sBothDegenerateS = sBothDegenerateT = HGMSIMDConstants::ZeroReal;

		// Segment1 is degenerate :
		FHGMSIMDReal sADegenerateS, sADegenerateT {};
		sADegenerateS = HGMSIMDConstants::ZeroReal;
		sADegenerateT = FHGMMathLibrary::Clamp(sF / sSafeE, HGMSIMDConstants::ZeroReal, HGMSIMDConstants::OneReal);

		// Segment2 is degenerate :
		FHGMSIMDReal sEDegenerateS, sEDegenerateT {};
		const FHGMSIMDReal sC = FHGMMathLibrary::DotProduct(sD1, sR);
		sEDegenerateS = FHGMMathLibrary::Clamp(-sC / sSafeA, HGMSIMDConstants::ZeroReal, HGMSIMDConstants::OneReal);
		sEDegenerateT = HGMSIMDConstants::ZeroReal;

		// Not degenerating :
		FHGMSIMDReal sNotDegenerateS, sNotDegenerateT {};
		const FHGMSIMDReal sB = FHGMMathLibrary::DotProduct(sD1, sD2);
		const FHGMSIMDReal sDenom = sA * sE - sB * sB;
		const FHGMSIMDReal sSafeDenom = FHGMSIMDLibrary::Select(sDenom <= HGMSIMDConstants::SmallReal, HGMSIMDConstants::OneReal, sDenom);

		sNotDegenerateS = FHGMSIMDLibrary::Select(sDenom != HGMSIMDConstants::ZeroReal, FHGMMathLibrary::Clamp((sB * sF - sC * sE) / sSafeDenom, HGMSIMDConstants::ZeroReal, HGMSIMDConstants::OneReal), HGMSIMDConstants::ZeroReal);
		FHGMSIMDReal sTempNotDegenerateT = (sB * sNotDegenerateS + sF) / sE;
		sNotDegenerateT = FHGMSIMDLibrary::Select(sTempNotDegenerateT < HGMSIMDConstants::ZeroReal, HGMSIMDConstants::ZeroReal, sTempNotDegenerateT);
		sNotDegenerateS = FHGMSIMDLibrary::Select(sTempNotDegenerateT < HGMSIMDConstants::ZeroReal, FHGMMathLibrary::Clamp(-sC / sSafeA, HGMSIMDConstants::ZeroReal, HGMSIMDConstants::OneReal), sNotDegenerateS);

		sNotDegenerateT = FHGMSIMDLibrary::Select(sTempNotDegenerateT > HGMSIMDConstants::OneReal, HGMSIMDConstants::OneReal, sTempNotDegenerateT);
		sNotDegenerateS = FHGMSIMDLibrary::Select(sTempNotDegenerateT > HGMSIMDConstants::OneReal, FHGMMathLibrary::Clamp((sB - sC) / sSafeA, HGMSIMDConstants::ZeroReal, HGMSIMDConstants::OneReal), sNotDegenerateS);

		sOutS = sBothDegenerateS;
		sOutS = FHGMSIMDLibrary::Select(~sBothDegenerateMask & sADegenerateMask, sADegenerateS, sOutS);
		sOutS = FHGMSIMDLibrary::Select(~sBothDegenerateMask & sEDegenerateMask, sEDegenerateS, sOutS);
		sOutS = FHGMSIMDLibrary::Select(~sBothDegenerateMask, sNotDegenerateS, sOutS);

		sOutT= sBothDegenerateT;
		sOutT= FHGMSIMDLibrary::Select(~sBothDegenerateMask & sADegenerateMask, sADegenerateT, sOutT);
		sOutT= FHGMSIMDLibrary::Select(~sBothDegenerateMask & sEDegenerateMask, sEDegenerateT, sOutT);
		sOutT= FHGMSIMDLibrary::Select(~sBothDegenerateMask, sNotDegenerateT, sOutT);

		sOutC1 = FHGMSIMDLibrary::Select(sBothDegenerateMask, sP1, sP1 + sD1 * sOutS);
		sOutC2 = FHGMSIMDLibrary::Select(sBothDegenerateMask, sP2, sP2 + sD2 * sOutT);

		return FHGMMathLibrary::DotProduct(sOutC1 - sOutC2, sOutC1 - sOutC2);
	}


	/**
	 * Collision detection between line segment and sphere.
	 */
	FHGMSIMDReal IntersectSegmentSphere(const FHGMSIMDVector3& sSegmentStart, const FHGMSIMDVector3& sSegmentEnd, const FHGMSIMDSphereCollider& sSphere, FHGMSIMDReal& sOutT, FHGMSIMDVector3& sOutIntersectedPoint)
	{
		// Converted "ゲームプログラミングのためのリアルタイム衝突判定 > P178" to SIMD.
		const FHGMSIMDVector3 sSegment = sSegmentEnd - sSegmentStart;
		const FHGMSIMDReal sSegmentLength = FHGMMathLibrary::Length(sSegment);
		const FHGMSIMDVector3 sD = FHGMMathLibrary::MakeSafeNormal(sSegment);
		const FHGMSIMDVector3 sM = sSegmentStart - sSphere.sCenter;
		const FHGMSIMDReal sB = FHGMMathLibrary::DotProduct(sM, sD);
		const FHGMSIMDReal sC = FHGMMathLibrary::DotProduct(sM, sM) - (sSphere.sRadius * sSphere.sRadius);

		FHGMSIMDReal sNoHitMask = (sC > HGMSIMDConstants::ZeroReal) & (sB > HGMSIMDConstants::ZeroReal);

		const FHGMSIMDReal sDiscr = sB * sB - sC;
		sNoHitMask |= sDiscr < HGMSIMDConstants::ZeroReal;

		sOutT = -sB - FHGMMathLibrary::Sqrt(sDiscr);
		sOutT = FHGMMathLibrary::Max(sOutT, HGMSIMDConstants::ZeroReal);
		sNoHitMask |= sOutT > sSegmentLength;
		sOutT = FHGMSIMDLibrary::Select(sNoHitMask, HGMSIMDConstants::ZeroReal, sOutT);

		sOutIntersectedPoint = FHGMSIMDLibrary::Select(sNoHitMask, sSegmentStart, sSegmentStart + sOutT * sSegment);

		return ~sNoHitMask;
	}


	/**
	* Collision detection between line segment and capsule.
	*/
	FHGMSIMDReal IntersectSegmentCapsule(const FHGMSIMDVector3& sSegmentStart, const FHGMSIMDVector3& sSegmentEnd, const FHGMSIMDCapsuleCollider& sCapsule, FHGMSIMDVector3& sOutIntersectedPoint)
	{
		// Referring "ゲームプログラミングのためのリアルタイム衝突判定 > P197" .
		// Calculate cylinder that encapsulates capsule and make sure that line segments are not in a completely collision-free position :
		const FHGMSIMDSphereCollider sStartCapSphere(sCapsule.sStartPoint, sCapsule.sRadius);
		const FHGMSIMDSphereCollider sEndCapSphere(sCapsule.sEndPoint, sCapsule.sRadius);
		const FHGMSIMDVector3 sCapsuleDirection = FHGMMathLibrary::MakeSafeNormal(sCapsule.sEndPoint - sCapsule.sStartPoint);
		const FHGMSIMDVector3 sSA = sSegmentStart;
		const FHGMSIMDVector3 sSB = sSegmentEnd;
		const FHGMSIMDVector3 sN = sSB - sSA;
		const FHGMSIMDReal sR = sCapsule.sRadius;
		FHGMSIMDVector3 sP = sCapsule.sStartPoint - (sCapsule.sRadius * sCapsuleDirection);
		FHGMSIMDVector3 sQ = sCapsule.sEndPoint + (sCapsule.sRadius * sCapsuleDirection);
		FHGMSIMDVector3 sD = sQ - sP;
		FHGMSIMDVector3 sM = sSA - sP;
		FHGMSIMDReal sMD = FHGMMathLibrary::DotProduct(sM, sD);
		FHGMSIMDReal sND = FHGMMathLibrary::DotProduct(sN, sD);
		FHGMSIMDReal sDD = FHGMMathLibrary::DotProduct(sD, sD);

		FHGMSIMDReal sHitMask = ((sMD >= HGMSIMDConstants::ZeroReal) | ((sMD + sND) >= HGMSIMDConstants::ZeroReal)) & ((sMD <= sDD) | ((sMD + sND) <= sDD));

		// Calculate back to original size due to possible collision.
		sP = sCapsule.sStartPoint;
		sQ = sCapsule.sEndPoint;
		sD = sQ - sP;
		sM = sSA - sP;
		sMD = FHGMMathLibrary::DotProduct(sM, sD);
		sND = FHGMMathLibrary::DotProduct(sN, sD);
		sDD = FHGMMathLibrary::DotProduct(sD, sD);
		const FHGMSIMDReal sNN = FHGMMathLibrary::DotProduct(sN, sN);
		const FHGMSIMDReal sMN = FHGMMathLibrary::DotProduct(sM, sN);
		const FHGMSIMDReal sA = sDD * sNN * - sND * sND;
		const FHGMSIMDReal sK = FHGMMathLibrary::DotProduct(sM, sM) - sR * sR;
		const FHGMSIMDReal sC = sDD * sK - sMD * sMD;

		// When colliding parallel to the capsule along its axis :
		const FHGMSIMDReal sParallelMask = (sA < HGMSIMDConstants::SmallReal);

		FHGMSIMDReal sParallelStartCapT {};
		const FHGMSIMDReal sPossibleParallelStartCapHitMask = sHitMask & sParallelMask & (sC <= HGMSIMDConstants::ZeroReal) & (sMD < HGMSIMDConstants::ZeroReal);
		FHGMSIMDVector3 sParallelStartCapIntersectedPoint {};
		const FHGMSIMDReal sParallelStartCapHitMask = IntersectSegmentSphere(sSegmentStart, sSegmentEnd, sStartCapSphere, sParallelStartCapT, sParallelStartCapIntersectedPoint) & sPossibleParallelStartCapHitMask;

		FHGMSIMDReal sParallelEndCapT {};
		const FHGMSIMDReal sPossibleParallelEndCapHitMask = sHitMask & sParallelMask & (sC <= HGMSIMDConstants::ZeroReal) & (sMD >= HGMSIMDConstants::ZeroReal) & (sMD > sDD);
		FHGMSIMDVector3 sParallelEndCapIntersectedPoint {};
		const FHGMSIMDReal sParallelEndCapHitMask = IntersectSegmentSphere(sSegmentStart, sSegmentEnd, sEndCapSphere, sParallelEndCapT, sParallelEndCapIntersectedPoint) & sPossibleParallelEndCapHitMask;

		FHGMSIMDReal sParallelBuriedCapsuleT(HGMSIMDConstants::ZeroReal);
		const FHGMSIMDReal sParallelBuriedCapsuleHitMask = sHitMask & sParallelMask & (sC <= HGMSIMDConstants::ZeroReal) & (sMD >= HGMSIMDConstants::ZeroReal) & (sMD <= sDD);
		FHGMSIMDVector3 sParallelBuriedCapsuleIntersectedPoint(sSA);

		// Not parallel :
		const FHGMSIMDReal sB = sDD * sMN - sND * sMD;
		const FHGMSIMDReal sDiscr = sB * sB - sA * sC;
		FHGMSIMDReal sT = (-sB - FHGMMathLibrary::Sqrt(sDiscr)) / (sA + HGMSIMDConstants::SmallReal);
		sHitMask &= ((sT >= HGMSIMDConstants::ZeroReal) & sT <= HGMSIMDConstants::OneReal) & (sDiscr > HGMSIMDConstants::ZeroReal) & ~sParallelMask;

		const FHGMSIMDReal sLeftSideMask = ((sMD + sT * sND) < HGMSIMDConstants::ZeroReal);
		const FHGMSIMDReal sPossibleStartCapHitMask = sHitMask & sLeftSideMask;
		FHGMSIMDReal sStartCapT {};
		FHGMSIMDVector3 sStartCapIntersectedPoint {};
		const FHGMSIMDReal sStartCapHitMask = IntersectSegmentSphere(sSegmentStart, sSegmentEnd, sStartCapSphere, sStartCapT, sStartCapIntersectedPoint) & sPossibleStartCapHitMask;

		const FHGMSIMDReal sRightSideMask = ((sMD + sT * sND) > sDD);
		const FHGMSIMDReal sPossibleEndCapHitMask = sHitMask & ~sLeftSideMask & sRightSideMask;
		FHGMSIMDReal sEndCapT {};
		FHGMSIMDVector3 sEndCapIntersectedPoint {};
		const FHGMSIMDReal sEndCapHitMask = IntersectSegmentSphere(sSegmentStart, sSegmentEnd, sEndCapSphere, sEndCapT, sEndCapIntersectedPoint) & sPossibleEndCapHitMask;

		// Intersects with cylindrical part of capsule except for the above :
		const FHGMSIMDReal sCylinderHitMask = sHitMask & ~sLeftSideMask & ~sRightSideMask;

		// Output results :
		sOutIntersectedPoint = sSA;
		sOutIntersectedPoint = FHGMSIMDLibrary::Select(sParallelStartCapHitMask, sParallelStartCapIntersectedPoint, sOutIntersectedPoint);
		sOutIntersectedPoint = FHGMSIMDLibrary::Select(sParallelEndCapHitMask, sParallelEndCapIntersectedPoint, sOutIntersectedPoint);
		sOutIntersectedPoint = FHGMSIMDLibrary::Select(sParallelBuriedCapsuleHitMask, sParallelBuriedCapsuleIntersectedPoint, sOutIntersectedPoint);

		sOutIntersectedPoint = FHGMSIMDLibrary::Select(sStartCapHitMask, sStartCapIntersectedPoint, sOutIntersectedPoint);
		sOutIntersectedPoint = FHGMSIMDLibrary::Select(sEndCapHitMask, sEndCapIntersectedPoint, sOutIntersectedPoint);

		sOutIntersectedPoint = FHGMSIMDLibrary::Select(sCylinderHitMask, sSA + sT * sN, sOutIntersectedPoint);

		return sParallelStartCapHitMask | sParallelEndCapHitMask | sParallelBuriedCapsuleHitMask | sStartCapHitMask | sEndCapHitMask | sCylinderHitMask;
	}


	/**
	* Collision detection between static sphere and capsule.
	* OutColliderContact contains parameters to separate sSphere from sCapsule.
	*/
	FHGMSIMDReal IntersectStaticSphereCapsule(const FHGMSIMDSphereCollider& sSphere, const FHGMSIMDCapsuleCollider& sCapsule, FHGMSIMDColliderContact& sOutColliderContact)
	{
		// Converted "ゲームプログラミングのためのリアルタイム衝突判定 > P115" to SIMD.
		FHGMSIMDReal sT {};
		FHGMSIMDVector3 sProjectedPoint {};
		const FHGMSIMDReal sDistanceSquared = ClosestPointPointSegment(sSphere.sCenter, sCapsule.sStartPoint, sCapsule.sEndPoint, sT, sProjectedPoint);
		const FHGMSIMDReal sSumRadius = sSphere.sRadius + sCapsule.sRadius;
		const FHGMSIMDReal sSumRadiusSquared = sSumRadius * sSumRadius;
		const FHGMSIMDReal sHitMask = sDistanceSquared < sSumRadiusSquared;

		sOutColliderContact.sSeparatingNormal = FHGMSIMDLibrary::Select(sHitMask, FHGMMathLibrary::MakeSafeNormal(sSphere.sCenter - sProjectedPoint), FHGMSIMDVector3::ZeroVector);

		const FHGMSIMDVector3 sSeparatedPosition = sProjectedPoint + (sSumRadius * sOutColliderContact.sSeparatingNormal);
		sOutColliderContact.sSeparatingOffset = FHGMMathLibrary::DotProduct(sOutColliderContact.sSeparatingNormal, sSeparatedPosition);

		sOutColliderContact.sHitMask = sHitMask;

		return sHitMask;
	}


	/**
	* Collision detection between dynamic sphere and capsule.
	* OutColliderContact contains parameters to separate sSphere from sCapsule.
	*/
	FHGMSIMDReal IntersectDynamicSphereCapsule(const FHGMSIMDSphereCollider& sSphere, const FHGMSIMDVector3& sSphereVelocity, const FHGMSIMDCapsuleCollider& sCapsule, const FHGMSIMDVector3& sCapsuleVelocity, FHGMSIMDColliderContact& sOutColliderContact)
	{
		const FHGMSIMDCapsuleCollider sStaticCapsule(sCapsule.sStartPoint, sCapsule.sEndPoint, sCapsule.sRadius + sSphere.sRadius);
		const FHGMSIMDVector3 sRelativeSphereVelocity = sSphereVelocity - sCapsuleVelocity;

		FHGMSIMDVector3 sIntersectedPoint {};
		const FHGMSIMDReal sHitMask = IntersectSegmentCapsule(sSphere.sCenter, sSphere.sCenter + sRelativeSphereVelocity, sStaticCapsule, sIntersectedPoint);

		FHGMSIMDReal sT {};
		FHGMSIMDVector3 sCapsuleCenter {};
		ClosestPointPointSegment(sIntersectedPoint, sCapsule.sStartPoint, sCapsule.sEndPoint, sT, sCapsuleCenter);

		sOutColliderContact.sSeparatingNormal = FHGMSIMDLibrary::Select(sHitMask, FHGMMathLibrary::MakeSafeNormal(sSphere.sCenter - sCapsuleCenter), FHGMSIMDVector3::ZeroVector);
		sOutColliderContact.sSeparatingOffset = FHGMMathLibrary::DotProduct(sIntersectedPoint, sOutColliderContact.sSeparatingNormal);

		sOutColliderContact.sHitMask = sHitMask;

		return sHitMask;
	}


	/**
	* Collision detection between sphere and capsule.
	* OutColliderContact contains parameters to separate sSphere from sCapsule.
	*/
	bool IntersectSphereCapsule(const FHGMSIMDSphereCollider& sSphere, const FHGMSIMDSphereCollider& sPrevSphere, const FHGMSIMDCapsuleCollider& sCapsule, const FHGMSIMDCapsuleCollider& sPrevCapsule, FHGMSIMDColliderContact& sOutColliderContact)
	{
		FHGMSIMDReal sHitMask = ~HGMSIMDConstants::AllBitMask;
		sOutColliderContact.sSeparatingNormal = FHGMSIMDVector3::ZeroVector;
		sOutColliderContact.sSeparatingOffset = HGMSIMDConstants::ZeroReal;

		FHGMSIMDColliderContact sStaticContact {};
		const FHGMSIMDReal sStaticHitMask = IntersectStaticSphereCapsule(sSphere, sCapsule, sStaticContact);
		sHitMask |= sStaticHitMask;

		const FHGMSIMDVector3 sSphereVelocity = sSphere.sCenter - sPrevSphere.sCenter;
		const FHGMSIMDVector3 sCapsuleCenter(FHGMMathLibrary::Lerp(sCapsule.sStartPoint, sCapsule.sEndPoint, HGMSIMDConstants::OneHalfReal));
		const FHGMSIMDVector3 sPrevCapsuleCenter(FHGMMathLibrary::Lerp(sPrevCapsule.sStartPoint, sPrevCapsule.sEndPoint, HGMSIMDConstants::OneHalfReal));
		const FHGMSIMDVector3 sCapsuleVelocity(sCapsuleCenter - sPrevCapsuleCenter);
		FHGMSIMDColliderContact sDynamicContact {};
		const FHGMSIMDReal sDynamicHitMask = IntersectDynamicSphereCapsule(sPrevSphere, sSphereVelocity, sPrevCapsule, sCapsuleVelocity, sDynamicContact);
		sHitMask |= sDynamicHitMask;

		sOutColliderContact.sSeparatingNormal = FHGMSIMDLibrary::Select(sDynamicHitMask, sDynamicContact.sSeparatingNormal, sOutColliderContact.sSeparatingNormal);
		sOutColliderContact.sSeparatingOffset = FHGMSIMDLibrary::Select(sDynamicHitMask, sDynamicContact.sSeparatingOffset, sOutColliderContact.sSeparatingOffset);

		sOutColliderContact.sSeparatingNormal = FHGMSIMDLibrary::Select(sStaticHitMask, sStaticContact.sSeparatingNormal, sOutColliderContact.sSeparatingNormal);
		sOutColliderContact.sSeparatingOffset = FHGMSIMDLibrary::Select(sStaticHitMask, sStaticContact.sSeparatingOffset, sOutColliderContact.sSeparatingOffset);

		sOutColliderContact.sHitMask = sHitMask;

		return FHGMSIMDLibrary::IsAnyMaskSet(sHitMask);
	}


	/**
	 * Collision detection between stopped spheres.
	 * OutColliderContact contains parameters to separate sSphereA from sSphereB.
	 */
	FHGMSIMDReal IntersectStaticSphereSphere(const FHGMSIMDSphereCollider& sSphereA, const FHGMSIMDSphereCollider& sSphereB, FHGMSIMDColliderContact& sOutColliderContact)
	{
		const FHGMSIMDVector3 sDistanceVector = sSphereA.sCenter - sSphereB.sCenter;
		const FHGMSIMDReal sDistanceSquared = FHGMMathLibrary::LengthSquared(sDistanceVector);
		const FHGMSIMDReal sRadiusSummed = sSphereA.sRadius + sSphereB.sRadius;
		const FHGMSIMDReal sRadiusSummedSquared = sRadiusSummed * sRadiusSummed;

		const FHGMSIMDReal sHitMask = sDistanceSquared < sRadiusSummedSquared;

		const FHGMSIMDVector3 sSeparatingNormal = FHGMMathLibrary::MakeSafeNormal(sDistanceVector);
		const FHGMSIMDVector3 sSeparatedPosition = sSphereB.sCenter + sSeparatingNormal * sRadiusSummed;

		sOutColliderContact.sSeparatingNormal = FHGMSIMDLibrary::Select(sHitMask, sSeparatingNormal, FHGMSIMDVector3::ZeroVector);
		sOutColliderContact.sSeparatingOffset = FHGMMathLibrary::DotProduct(sSeparatedPosition, sOutColliderContact.sSeparatingNormal);
		sOutColliderContact.sHitMask = sHitMask;

		return sHitMask;
	}


	/**
	 * Collision detection between moving spheres.
	 * OutColliderContact contains parameters to separate sSphereA from sSphereB.
	 */
	FHGMSIMDReal IntersectDynamicSphereSphere(const FHGMSIMDSphereCollider& sSphereA, const FHGMSIMDVector3& sVelocityA, const FHGMSIMDSphereCollider& sSphereB, const FHGMSIMDVector3& sVelocityB, FHGMSIMDColliderContact& sOutColliderContact)
	{
		// Converted "ゲームプログラミングのためのリアルタイム衝突判定 > P226" to SIMD.
		const FHGMSIMDSphereCollider sStaticSphereB(sSphereB.sCenter, sSphereB.sRadius + sSphereA.sRadius);
		const FHGMSIMDVector3 sRelativeVelocityA = sVelocityA - sVelocityB;
		const FHGMSIMDVector3 sSphereAEndPosition = sSphereA.sCenter + sRelativeVelocityA;
		FHGMSIMDReal sT {};
		FHGMSIMDVector3 sIntersectedPoint {};

		const FHGMSIMDReal sHitMask = IntersectSegmentSphere(sSphereA.sCenter, sSphereAEndPosition, sStaticSphereB, sT, sIntersectedPoint);
		const FHGMSIMDVector3 sSeparatingNormal = FHGMMathLibrary::MakeSafeNormal(sSphereA.sCenter - sSphereB.sCenter);

		sOutColliderContact.sSeparatingNormal = FHGMSIMDLibrary::Select(sHitMask, sSeparatingNormal, FHGMSIMDVector3::ZeroVector);
		sOutColliderContact.sSeparatingOffset = FHGMMathLibrary::DotProduct(sIntersectedPoint, sOutColliderContact.sSeparatingNormal);
		sOutColliderContact.sHitMask = sHitMask;

		return sHitMask;
	}


	/**
	 * Collision detection between spheres.
	 * OutColliderContact contains parameters to separate sSphereA from sSphereB.
	 */
	bool IntersectSphereSphere(const FHGMSIMDSphereCollider& sSphereA, const FHGMSIMDSphereCollider& sPrevSphereA, const FHGMSIMDSphereCollider& sSphereB, const FHGMSIMDSphereCollider& sPrevSphereB, FHGMSIMDColliderContact& sOutColliderContact)
	{
		FHGMSIMDReal sHitMask = ~HGMSIMDConstants::AllBitMask;
		sOutColliderContact.sSeparatingNormal = FHGMSIMDVector3::ZeroVector;
		sOutColliderContact.sSeparatingOffset = HGMSIMDConstants::ZeroReal;

		FHGMSIMDColliderContact sStaticContact {};
		const FHGMSIMDReal sStaticHitMask = IntersectStaticSphereSphere(sSphereA, sSphereB, sStaticContact);
		sHitMask |= sStaticHitMask;

		const FHGMSIMDVector3 sVelocityA = sSphereA.sCenter - sPrevSphereA.sCenter;
		const FHGMSIMDVector3 sVelocityB = sSphereB.sCenter - sPrevSphereB.sCenter;
		FHGMSIMDColliderContact sDynamicContact {};
		const FHGMSIMDReal sDynamicHitMask = IntersectDynamicSphereSphere(sSphereA, sVelocityA, sSphereB, sVelocityB, sDynamicContact);
		sHitMask |= sDynamicHitMask;

		sOutColliderContact.sSeparatingNormal = FHGMSIMDLibrary::Select(sDynamicHitMask, sDynamicContact.sSeparatingNormal, sOutColliderContact.sSeparatingNormal);
		sOutColliderContact.sSeparatingOffset = FHGMSIMDLibrary::Select(sDynamicHitMask, sDynamicContact.sSeparatingOffset, sOutColliderContact.sSeparatingOffset);

		sOutColliderContact.sSeparatingNormal = FHGMSIMDLibrary::Select(sStaticHitMask, sStaticContact.sSeparatingNormal, sOutColliderContact.sSeparatingNormal);
		sOutColliderContact.sSeparatingOffset = FHGMSIMDLibrary::Select(sStaticHitMask, sStaticContact.sSeparatingOffset, sOutColliderContact.sSeparatingOffset);

		sOutColliderContact.sHitMask = sHitMask;

		return FHGMSIMDLibrary::IsAnyMaskSet(sHitMask);
	}


	struct FHGMSIMDEdgeColliderContact
	{
		FHGMSIMDVector3 sSeparatingNormal {};
		FHGMSIMDReal sEdgeStartSeparatingOffset {};
		FHGMSIMDReal sEdgeEndSeparatingOffset {};
		FHGMSIMDReal sHitMask {};
	};


	bool IntersectEdgeCapsule(const FHGMSIMDVector3& sEdgeStart, const FHGMSIMDVector3& sEdgeEnd, const FHGMSIMDCapsuleCollider& sCapsule, FHGMSIMDEdgeColliderContact& sOutEdgeColliderContact)
	{
		FHGMSIMDReal sT0, sT1 {};
		FHGMSIMDVector3 sProjectedPoint0, sProjectedPoint1 {};
		const FHGMSIMDReal sDistanceSquared = ClosestPointSegmentSegment(sEdgeStart, sEdgeEnd, sCapsule.sStartPoint, sCapsule.sEndPoint, sT0, sT1, sProjectedPoint0, sProjectedPoint1);
		const FHGMSIMDReal sRadiusSquared = sCapsule.sRadius * sCapsule.sRadius;
		const FHGMSIMDReal sHitMask = sDistanceSquared < sRadiusSquared;

		sOutEdgeColliderContact.sSeparatingNormal = FHGMSIMDLibrary::Select(sHitMask, FHGMMathLibrary::MakeSafeNormal(sProjectedPoint0 - sProjectedPoint1), FHGMSIMDVector3::ZeroVector);
		const FHGMSIMDVector3 sSeparatedPoint = sCapsule.sRadius * sOutEdgeColliderContact.sSeparatingNormal + sProjectedPoint1;
		const FHGMSIMDReal sSeparatingOffset = FHGMMathLibrary::DotProduct(sSeparatedPoint, sOutEdgeColliderContact.sSeparatingNormal);

		sOutEdgeColliderContact.sEdgeStartSeparatingOffset = sSeparatingOffset;
		sOutEdgeColliderContact.sEdgeEndSeparatingOffset = sSeparatingOffset;
		sOutEdgeColliderContact.sHitMask;

		return FHGMSIMDLibrary::IsAnyMaskSet(sHitMask);
	}


	bool IntersectEdgeSphere(const FHGMSIMDVector3& sEdgeStart, const FHGMSIMDVector3& sEdgeEnd, const FHGMSIMDSphereCollider& sSphere, FHGMSIMDEdgeColliderContact& sOutEdgeColliderContact)
	{
		FHGMSIMDReal sT {};
		FHGMSIMDVector3 sProjectedPoint {};
		const FHGMSIMDReal sDistanceSquared = ClosestPointPointSegment(sSphere.sCenter, sEdgeStart, sEdgeEnd, sT, sProjectedPoint);
		const FHGMSIMDReal sRadiusSquared = sSphere.sRadius * sSphere.sRadius;
		const FHGMSIMDReal sHitMask = sDistanceSquared < sRadiusSquared;

		sOutEdgeColliderContact.sSeparatingNormal = FHGMSIMDLibrary::Select(sHitMask, FHGMMathLibrary::MakeSafeNormal(sProjectedPoint - sSphere.sCenter), FHGMSIMDVector3::ZeroVector);
		const FHGMSIMDVector3 sSeparatedPoint = sSphere.sRadius * sOutEdgeColliderContact.sSeparatingNormal + sSphere.sCenter;
		sOutEdgeColliderContact.sEdgeStartSeparatingOffset = FHGMMathLibrary::DotProduct(sSeparatedPoint, sOutEdgeColliderContact.sSeparatingNormal);
		sOutEdgeColliderContact.sEdgeEndSeparatingOffset = FHGMMathLibrary::DotProduct(sSeparatedPoint, sOutEdgeColliderContact.sSeparatingNormal);
		sOutEdgeColliderContact.sHitMask = sHitMask;

		return FHGMSIMDLibrary::IsAnyMaskSet(sHitMask);
	}


	bool IntersectPlaneSphere(const FHGMSIMDPlaneCollider& sPlaneCollider, const FHGMSIMDSphereCollider& sSphereCollider, FHGMSIMDColliderContact& sOutColliderContact)
	{
		const FHGMSIMDVector3& sPlaneNormal = FHGMMathLibrary::GetUpVector(sPlaneCollider.sRotation);
		const FHGMSIMDReal sPlaneProjection = FHGMMathLibrary::DotProduct(sPlaneNormal, sPlaneCollider.sOrigin) + sSphereCollider.sRadius;
		const FHGMSIMDReal sSphereColliderProjection = FHGMMathLibrary::DotProduct(sPlaneNormal, sSphereCollider.sCenter);
		const FHGMSIMDReal sOffset = FHGMMathLibrary::Max(HGMSIMDConstants::ZeroReal, sPlaneProjection - sSphereColliderProjection);

		sOutColliderContact.sHitMask = sOffset > HGMSIMDConstants::ZeroReal;
		sOutColliderContact.sSeparatingNormal = sPlaneNormal;
		sOutColliderContact.sSeparatingOffset = sPlaneProjection + sOffset;

		return FHGMSIMDLibrary::IsAnyMaskSet(sOutColliderContact.sHitMask);
	}
} // End of namespace


  // ---------------------------------------------------------------------------------------
  // CollisionLibrary
  // ---------------------------------------------------------------------------------------
void FHGMCollisionLibrary::InitializeBodyColliderFromPhysicsAsset(const FBoneContainer& RequiredBones, UPhysicsAsset* PhysicsAsset, FHGMBodyCollider& OutBodyCollider)
{
	if (!PhysicsAsset)
	{
		return;
	}

	// Gather and initialize body collider from the physical asset.
	OutBodyCollider.BoneSpaceSphereColliders.Reset();
	OutBodyCollider.BoneSpaceCapsuleColliders.Reset();
	for (const TObjectPtr<USkeletalBodySetup> SkeletalBodySetup : PhysicsAsset->SkeletalBodySetups)
	{
		FBoneReference DriverBone = SkeletalBodySetup->BoneName;
		DriverBone.Initialize(RequiredBones);
		if (!DriverBone.IsValidToEvaluate(RequiredBones))
		{
			HGM_LOG(Log, TEXT("BoneName: [%s] was invalid. Switching LODs may have changed configuration of the skeleton."), *SkeletalBodySetup->BoneName.ToString());
			continue;
		}

		// Sphere:
		const FKAggregateGeom& AggGeom = SkeletalBodySetup->AggGeom;
		for (const auto& SphereElem : AggGeom.SphereElems)
		{
			OutBodyCollider.BoneSpaceSphereColliders.Emplace(DriverBone, SphereElem.Center, SphereElem.Radius);
		}

		// Capsule:
		for (const auto& CapsuleElem : AggGeom.SphylElems)
		{
			FHGMVector3 HalfCapsuleDirection { 0.0, 0.0, CapsuleElem.Length * 0.5 };
			HalfCapsuleDirection = CapsuleElem.Rotation.RotateVector(HalfCapsuleDirection);
			const FHGMVector3 StartPoint = CapsuleElem.Center - HalfCapsuleDirection;
			const FHGMVector3 EndPoint = CapsuleElem.Center + HalfCapsuleDirection;
			OutBodyCollider.BoneSpaceCapsuleColliders.Emplace(DriverBone, StartPoint, EndPoint, CapsuleElem.Radius);
		}
	}

	OutBodyCollider.SphereColliders.Init(FHGMSphereCollider(), OutBodyCollider.BoneSpaceSphereColliders.Num());
	OutBodyCollider.CapsuleColliders.Init(FHGMCapsuleCollider(), OutBodyCollider.BoneSpaceCapsuleColliders.Num());

	HGM_CLOG(OutBodyCollider.BoneSpaceSphereColliders.Num() + OutBodyCollider.BoneSpaceCapsuleColliders.Num() <= 0, Log, TEXT("No collider is included in body collider."));
}


void FHGMCollisionLibrary::UpdateBodyCollider(FComponentSpacePoseContext& Output, const FHGMPhysicsContext& PhysicsContext, FHGMBodyCollider& BodyCollider, FHGMBodyCollider& PrevBodyCollider)
{
	PrevBodyCollider = BodyCollider;

	for (int32 ColliderIndex = 0; ColliderIndex < BodyCollider.BoneSpaceSphereColliders.Num(); ++ColliderIndex)
	{
		FHGMSphereCollider& UpdatingSphereCollider = BodyCollider.SphereColliders[ColliderIndex];
		const FHGMBoneSpaceSphereCollider& BoneSpaceSphereCollider = BodyCollider.BoneSpaceSphereColliders[ColliderIndex];
		if (!BoneSpaceSphereCollider.DriverBone.IsValidToEvaluate())
		{
			UpdatingSphereCollider.bEnabled = false;
			continue;
		}
		UpdatingSphereCollider.bEnabled = true;

		const FCompactPoseBoneIndex BoneIndex(BoneSpaceSphereCollider.DriverBone.BoneIndex);
		const FHGMTransform& ComponentSpaceBoneTransform = Output.Pose.GetComponentSpaceTransform(BoneIndex);

		UpdatingSphereCollider.Center = ComponentSpaceBoneTransform.TransformPosition(BoneSpaceSphereCollider.Center);
		UpdatingSphereCollider.Radius = BoneSpaceSphereCollider.Radius;
	}

	for (int32 ColliderIndex = 0; ColliderIndex < BodyCollider.BoneSpaceCapsuleColliders.Num(); ++ColliderIndex)
	{
		FHGMCapsuleCollider& UpdatingCapsuleCollider = BodyCollider.CapsuleColliders[ColliderIndex];
		const FHGMBoneSpaceCapsuleCollider& BoneSpaceCapsuleCollider = BodyCollider.BoneSpaceCapsuleColliders[ColliderIndex];
		if (!BoneSpaceCapsuleCollider.DriverBone.IsValidToEvaluate())
		{
			UpdatingCapsuleCollider.bEnabled = false;
			continue;
		}
		UpdatingCapsuleCollider.bEnabled = true;

		const FCompactPoseBoneIndex BoneIndex(BoneSpaceCapsuleCollider.DriverBone.BoneIndex);
		const FHGMTransform& ComponentSpaceBoneTransform = Output.Pose.GetComponentSpaceTransform(BoneIndex);

		UpdatingCapsuleCollider.StartPoint = ComponentSpaceBoneTransform.TransformPosition(BoneSpaceCapsuleCollider.StartPoint);
		UpdatingCapsuleCollider.EndPoint = ComponentSpaceBoneTransform.TransformPosition(BoneSpaceCapsuleCollider.EndPoint);
		UpdatingCapsuleCollider.Radius = BoneSpaceCapsuleCollider.Radius;
	}

	if (PhysicsContext.bIsFirstUpdate)
	{
		PrevBodyCollider = BodyCollider;
	}
}


void FHGMCollisionLibrary::CalculateBodyColliderContacts(TConstArrayView<FHGMSIMDReal> BoneSphereColliderRadiuses, TArrayView<FHGMSIMDVector3> Positions, TArrayView<FHGMSIMDVector3> PrevPositions, const FHGMBodyCollider& BodyCollider, const FHGMBodyCollider& PrevBodyCollider, TArray<FHGMSIMDColliderContact>& OutContacts)
{
	SCOPE_CYCLE_COUNTER(STAT_CollisionCalculateBodyColliderContacts);

	for (int32 PackedIndex = 0; PackedIndex < Positions.Num(); ++PackedIndex)
	{
		const FHGMSIMDVector3& sPosition = Positions[PackedIndex];
		const FHGMSIMDVector3& sPrevPosition = PrevPositions[PackedIndex];
		const FHGMSIMDReal& sBoneSphereColliderRadius = BoneSphereColliderRadiuses[PackedIndex];
		const FHGMSIMDSphereCollider sBoneSphereCollider(sPosition, sBoneSphereColliderRadius);
		const FHGMSIMDSphereCollider sPrevBoneSphereCollider(sPrevPosition, sBoneSphereColliderRadius);

		// BoneSphere vs Sphere :
		for (int32 ColliderIndex = 0; ColliderIndex < BodyCollider.SphereColliders.Num(); ++ColliderIndex)
		{
			const FHGMSphereCollider& SphereCollider = BodyCollider.SphereColliders[ColliderIndex];
			if (!SphereCollider.bEnabled)
			{
				continue;
			}

			const FHGMSphereCollider& PrevSphereCollider = PrevBodyCollider.SphereColliders[ColliderIndex];
			FHGMSIMDSphereCollider sSphereCollider(SphereCollider);
			FHGMSIMDSphereCollider sPrevSphereCollider(PrevSphereCollider);
			FHGMSIMDColliderContact sContact {};
			if (IntersectSphereSphere(sBoneSphereCollider, sPrevBoneSphereCollider, sSphereCollider, sPrevSphereCollider, sContact))
			{
				sContact.PackedIndex = PackedIndex;
				OutContacts.Emplace(sContact);
			}
		}

		// BoneSphere vs Capsule :
		for (int32 ColliderIndex = 0; ColliderIndex < BodyCollider.CapsuleColliders.Num(); ++ColliderIndex)
		{
			const FHGMCapsuleCollider& CapsuleCollider = BodyCollider.CapsuleColliders[ColliderIndex];
			if (!CapsuleCollider.bEnabled)
			{
				continue;
			}

			const FHGMCapsuleCollider& PrevCapsuleCollider = PrevBodyCollider.CapsuleColliders[ColliderIndex];
			FHGMSIMDCapsuleCollider sCapsuleCollider(CapsuleCollider);
			FHGMSIMDCapsuleCollider sPrevCapsuleCollider(PrevCapsuleCollider);
			FHGMSIMDColliderContact sContact {};
			if (IntersectSphereCapsule(sBoneSphereCollider, sPrevBoneSphereCollider, sCapsuleCollider, sPrevCapsuleCollider, sContact))
			{
				sContact.PackedIndex = PackedIndex;
				OutContacts.Emplace(sContact);
			}
		}
	}
}


void FHGMCollisionLibrary::CalculateBodyColliderContactsForVerticalEdge(TConstArrayView<FHGMSIMDStructure> VerticalStructures, TArrayView<FHGMSIMDVector3> Positions, const FHGMBodyCollider& BodyCollider, TArray<FHGMSIMDColliderContact>& OutContacts)
{
	SCOPE_CYCLE_COUNTER(STAT_CollisionCalculateBodyColliderContactsForVerticalEdge);

	for (int32 PackedIndex = 0; PackedIndex < Positions.Num(); ++PackedIndex)
	{
		for (const FHGMSIMDStructure& Structure : VerticalStructures)
		{
			const FHGMSIMDVector3& sSegmentStart = Positions[Structure.FirstBonePackedIndex];
			const FHGMSIMDVector3& sSegmentEnd = Positions[Structure.SecondBonePackedIndex];

			// BoneEdge vs Sphere :
			for (int32 ColliderIndex = 0; ColliderIndex < BodyCollider.SphereColliders.Num(); ++ColliderIndex)
			{
				const FHGMSphereCollider& SphereCollider = BodyCollider.SphereColliders[ColliderIndex];
				if (!SphereCollider.bEnabled)
				{
					continue;
				}

				FHGMSIMDSphereCollider sSphereCollider(SphereCollider);
				FHGMSIMDEdgeColliderContact sEdgeContact {};

				if (IntersectEdgeSphere(sSegmentStart, sSegmentEnd, sSphereCollider, sEdgeContact))
				{
					OutContacts.Emplace(Structure.FirstBonePackedIndex, sEdgeContact.sHitMask, sEdgeContact.sSeparatingNormal, sEdgeContact.sEdgeStartSeparatingOffset);
					OutContacts.Emplace(Structure.SecondBonePackedIndex, sEdgeContact.sHitMask, sEdgeContact.sSeparatingNormal, sEdgeContact.sEdgeEndSeparatingOffset);
				}
			}

			// BoneEdge vs Capsule :
			for (int32 ColliderIndex = 0; ColliderIndex < BodyCollider.CapsuleColliders.Num(); ++ColliderIndex)
			{
				const FHGMCapsuleCollider& CapsuleCollider = BodyCollider.CapsuleColliders[ColliderIndex];
				if (!CapsuleCollider.bEnabled)
				{
					continue;
				}

				FHGMCapsuleCollider sCapsuleCollider(CapsuleCollider);
				FHGMSIMDEdgeColliderContact sEdgeContact {};
				if (IntersectEdgeCapsule(sSegmentStart, sSegmentEnd, sCapsuleCollider, sEdgeContact))
				{
					OutContacts.Emplace(Structure.FirstBonePackedIndex, sEdgeContact.sHitMask, sEdgeContact.sSeparatingNormal, sEdgeContact.sEdgeStartSeparatingOffset);
					OutContacts.Emplace(Structure.SecondBonePackedIndex, sEdgeContact.sHitMask, sEdgeContact.sSeparatingNormal, sEdgeContact.sEdgeEndSeparatingOffset);
				}
			}
		}
	}
}


void FHGMCollisionLibrary::CalculateBodyColliderContactsForHorizontalEdge(FHGMSimulationPlane& SimulationPlane, TConstArrayView<FHGMSIMDStructure> HorizontalStructures, TArray<FHGMSIMDVector3>& Positions, const FHGMBodyCollider& BodyCollider, TArray<FHGMSIMDColliderContact>& OutContacts)
{
	SCOPE_CYCLE_COUNTER(STAT_CollisionCalculateBodyColliderContactsForHorizontalEdge);

	if (HorizontalStructures.IsEmpty())
	{
		return;
	}

	FScopedTranspose ScopedTranspose(&SimulationPlane);
	ScopedTranspose.Add(&Positions);
	ScopedTranspose.Execute();

	for (const FHGMSIMDStructure& Structure : HorizontalStructures)
	{
		const FHGMSIMDVector3& sSegmentStart = Positions[Structure.FirstBonePackedIndex];
		const FHGMSIMDVector3& sSegmentEnd = Positions[Structure.SecondBonePackedIndex];

		// BoneEdge vs Sphere :
		for (int32 ColliderIndex = 0; ColliderIndex < BodyCollider.SphereColliders.Num(); ++ColliderIndex)
		{
			const FHGMSphereCollider& SphereCollider = BodyCollider.SphereColliders[ColliderIndex];
			if (!SphereCollider.bEnabled)
			{
				continue;
			}

			FHGMSIMDSphereCollider sSphereCollider(SphereCollider);
			FHGMSIMDEdgeColliderContact sEdgeContact {};

			if (IntersectEdgeSphere(sSegmentStart, sSegmentEnd, sSphereCollider, sEdgeContact))
			{
				OutContacts.Emplace(Structure.FirstBonePackedIndex, sEdgeContact.sHitMask, sEdgeContact.sSeparatingNormal, sEdgeContact.sEdgeStartSeparatingOffset);
				OutContacts.Emplace(Structure.SecondBonePackedIndex, sEdgeContact.sHitMask, sEdgeContact.sSeparatingNormal, sEdgeContact.sEdgeEndSeparatingOffset);
			}
		}

		// BoneEdge vs Capsule :
		for (int32 ColliderIndex = 0; ColliderIndex < BodyCollider.CapsuleColliders.Num(); ++ColliderIndex)
		{
			const FHGMCapsuleCollider& CapsuleCollider = BodyCollider.CapsuleColliders[ColliderIndex];
			if (!CapsuleCollider.bEnabled)
			{
				continue;
			}

			FHGMCapsuleCollider sCapsuleCollider(CapsuleCollider);
			FHGMSIMDEdgeColliderContact sEdgeContact {};
			if (IntersectEdgeCapsule(sSegmentStart, sSegmentEnd, sCapsuleCollider, sEdgeContact))
			{
				OutContacts.Emplace(Structure.FirstBonePackedIndex, sEdgeContact.sHitMask, sEdgeContact.sSeparatingNormal, sEdgeContact.sEdgeStartSeparatingOffset);
				OutContacts.Emplace(Structure.SecondBonePackedIndex, sEdgeContact.sHitMask, sEdgeContact.sSeparatingNormal, sEdgeContact.sEdgeEndSeparatingOffset);
			}
		}
	}
}


void FHGMCollisionLibrary::InitializePlaneColliders(const FBoneContainer& RequiredBones, TArrayView<FHGMPlaneCollider> PlaneColliders, TArray<FHGMSIMDPlaneCollider>& OutPlaneColliders)
{
	OutPlaneColliders.Reset(PlaneColliders.Num());
	OutPlaneColliders.SetNum(PlaneColliders.Num());
	for (int32 ColliderIndex = 0; ColliderIndex < PlaneColliders.Num(); ++ColliderIndex)
	{
		FHGMPlaneCollider& PlaneCollider = PlaneColliders[ColliderIndex];
		PlaneCollider.DrivingBone.Initialize(RequiredBones);
	}
}


void FHGMCollisionLibrary::UpdatePlaneColliders(FComponentSpacePoseContext& Output, TConstArrayView<FHGMPlaneCollider> PlaneColliders, TArrayView<FHGMSIMDPlaneCollider> OutUpdatedPlaneColliders)
{
	const FBoneContainer& BoneContainer = Output.Pose.GetPose().GetBoneContainer();
	for (int32 ColliderIndex = 0; ColliderIndex < PlaneColliders.Num(); ++ColliderIndex)
	{
		const FHGMPlaneCollider& PlaneCollider = PlaneColliders[ColliderIndex];
		if (PlaneCollider.DrivingBone.IsValidToEvaluate())
		{
			FHGMTransform PlaneTansform = FHGMTransform::Identity;
			PlaneTansform.SetLocation(PlaneCollider.LocationOffset);
			PlaneTansform.SetRotation(PlaneCollider.RotationOffset.Quaternion());

			const FCompactPoseBoneIndex DrivingBoneIndex = PlaneCollider.DrivingBone.GetCompactPoseIndex(BoneContainer);
			const FHGMTransform& BoneTransform = Output.Pose.GetComponentSpaceTransform(DrivingBoneIndex);

			// Converts to component space.
			PlaneTansform *= BoneTransform;

			FHGMSIMDLibrary::Load(OutUpdatedPlaneColliders[ColliderIndex].sOrigin, PlaneTansform.GetTranslation());
			FHGMSIMDLibrary::Load(OutUpdatedPlaneColliders[ColliderIndex].sRotation, PlaneTansform.GetRotation());
		}
		// It will be floor collider with pivot originating from skeletal mesh pivot.
		else
		{
			FHGMSIMDLibrary::Load(OutUpdatedPlaneColliders[ColliderIndex].sOrigin, PlaneCollider.LocationOffset);
			FHGMSIMDLibrary::Load(OutUpdatedPlaneColliders[ColliderIndex].sRotation, PlaneCollider.RotationOffset.Quaternion());
		}
	}
}


void FHGMCollisionLibrary::CalculatePlaneColliderContacts(TConstArrayView<FHGMSIMDPlaneCollider> PlaneColliders, TConstArrayView<FHGMSIMDReal> BoneSphereColliderRadiuses, TArray<FHGMSIMDVector3>& Positions, TArray<FHGMSIMDColliderContact>& OutContacts)
{
	if (PlaneColliders.Num() <= 0)
	{
		return;
	}

	for (int32 PackedIndex = 0; PackedIndex < Positions.Num(); ++PackedIndex)
	{
		const FHGMSIMDVector3& sPosition = Positions[PackedIndex];
		const FHGMSIMDReal& sBoneSphereColliderRadius = BoneSphereColliderRadiuses[PackedIndex];
		const FHGMSIMDSphereCollider sBoneSphereCollider(sPosition, sBoneSphereColliderRadius);

		for (const FHGMSIMDPlaneCollider& sPlaneCollider : PlaneColliders)
		{
			FHGMSIMDColliderContact sContact {};
			if (IntersectPlaneSphere(sPlaneCollider, sBoneSphereCollider, sContact))
			{
				sContact.PackedIndex = PackedIndex;
				OutContacts.Emplace(sContact);
			}
		}
	}
}
