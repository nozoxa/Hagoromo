// Hagoromo : Copyright (c) 2025 nozoxa_0131, MIT License

#pragma once

#include "HGMMath.h"
#include "HGMSolvers.h"
#include "AnimNode_Hagoromo.h"

struct FHGMSIMDColliderContact;


struct FHGMPhysicsLibrary
{
	static void ApplyForces(FComponentSpacePoseContext& Output, const FHGMPhysicsContext& PhysicsContext, TArrayView<FHGMSIMDVector3> Positions, TArrayView<FHGMSIMDVector3> PrevPositions,
						TConstArrayView<FHGMSIMDReal> WorldVelocityDampings, TConstArrayView<FHGMSIMDReal> WorldAngularVelocityDampings, TConstArrayView<FHGMSIMDReal> SimulationVelocityDampings, TConstArrayView<FHGMSIMDReal> SimulationAngularVelocityDampings, TConstArrayView<FHGMSIMDReal> MasterDampings,
						TConstArrayView<FHGMSIMDReal> Frictions, TConstArrayView<FHGMSIMDReal> FixedBlends, TConstArrayView<FHGMSIMDReal> DummyBoneMasks);

	static void VerletIntegrate(const FHGMPhysicsContext& PhysicsContext, TArray<FHGMSIMDVector3>& Positions, TArray<FHGMSIMDVector3>& PrevPositions, TConstArrayView<FHGMSIMDReal> Frictions, TConstArrayView<FHGMSIMDReal> MasterDampings, TConstArrayView<FHGMSIMDReal> FixedBlends, TConstArrayView<FHGMSIMDReal> DummyBoneMasks);

	static void ResetFriction(TArray<FHGMSIMDReal>& Frictions);
	static void CalculateFriction(TConstArrayView<FHGMSIMDReal> Frictions, TConstArrayView<FHGMSIMDColliderContact> ColliderContacts, TArray<FHGMSIMDReal>& sOutFrictions);
};
