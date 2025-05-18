// Hagoromo : Copyright (c) 2025 nozoxa_0131, MIT License

#pragma once

#include "HGMMath.h"

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
#include "Stats/Stats.h"

DECLARE_LOG_CATEGORY_EXTERN(LogHagoromoRuntime, Log, Log);


// ---------------------------------------------------------------------------------------
// Globals
// ---------------------------------------------------------------------------------------
namespace HGMGlobal
{
	extern FString IniFileName;
	extern FHGMReal TargetFrameRate;
}


// ---------------------------------------------------------------------------------------
// Macros
// ---------------------------------------------------------------------------------------
#define HGM_LOG(Verbosity, Format, ...) \
UE_LOG(LogHagoromoRuntime, Verbosity, TEXT("[%hs L%d] %s"), (__FUNCTION__), (__LINE__), *FString::Printf(Format, ##__VA_ARGS__)); \

#define HGM_CLOG(Condition, Verbosity, Format, ...) \
UE_CLOG(Condition, LogHagoromoRuntime, Verbosity, TEXT("[%hs L%d] %s"), (__FUNCTION__), (__LINE__), *FString::Printf(Format, ##__VA_ARGS__)); \

DECLARE_STATS_GROUP(TEXT("Hagoromo"), STATGROUP_Hagoromo, STATCAT_Advanced);

DECLARE_CYCLE_STAT_EXTERN(TEXT("Collision CalculateBodyColliderContacts"), STAT_CollisionCalculateBodyColliderContacts, STATGROUP_Hagoromo, HAGOROMO_API);
DECLARE_CYCLE_STAT_EXTERN(TEXT("Collision CalculateBodyColliderContactsForVerticalEdge"), STAT_CollisionCalculateBodyColliderContactsForVerticalEdge, STATGROUP_Hagoromo, HAGOROMO_API);
DECLARE_CYCLE_STAT_EXTERN(TEXT("Collision CalculateBodyColliderContactsForHorizontalEdge"), STAT_CollisionCalculateBodyColliderContactsForHorizontalEdge, STATGROUP_Hagoromo, HAGOROMO_API);

DECLARE_CYCLE_STAT_EXTERN(TEXT("Constraint VerticalStructuralConstraint"), STAT_ConstraintVerticalStructuralConstraint, STATGROUP_Hagoromo, HAGOROMO_API);
DECLARE_CYCLE_STAT_EXTERN(TEXT("Constraint HorizontalStructuralConstraint"), STAT_ConstraintHorizontalStructuralConstraint, STATGROUP_Hagoromo, HAGOROMO_API);
DECLARE_CYCLE_STAT_EXTERN(TEXT("Constraint ShearConstraint"), STAT_ConstraintShearConstraint, STATGROUP_Hagoromo, HAGOROMO_API);
DECLARE_CYCLE_STAT_EXTERN(TEXT("Constraint VerticalBendConstraint"), STAT_ConstraintVerticalBendConstraint, STATGROUP_Hagoromo, HAGOROMO_API);
DECLARE_CYCLE_STAT_EXTERN(TEXT("Constraint HorizontalBendConstraint"), STAT_ConstraintHorizontalBendConstraint, STATGROUP_Hagoromo, HAGOROMO_API);
DECLARE_CYCLE_STAT_EXTERN(TEXT("Constraint RelativeLimitAngleConstraint"), STAT_ConstraintRelativeLimitAngleConstraint, STATGROUP_Hagoromo, HAGOROMO_API);
DECLARE_CYCLE_STAT_EXTERN(TEXT("Constraint AnimPoseMovableRadiusConstraint"), STAT_ConstraintAnimPoseMovableRadiusConstraint, STATGROUP_Hagoromo, HAGOROMO_API);
DECLARE_CYCLE_STAT_EXTERN(TEXT("Constraint AnimPoseLimitAngleConstraint"), STAT_ConstraintAnimPoseLimitAngleConstraint, STATGROUP_Hagoromo, HAGOROMO_API);
DECLARE_CYCLE_STAT_EXTERN(TEXT("Constraint AnimPosePlanarConstraint"), STAT_ConstraintAnimPosePlanarConstraint, STATGROUP_Hagoromo, HAGOROMO_API);

DECLARE_CYCLE_STAT_EXTERN(TEXT("Physics ApplyForces"), STAT_PhysicsApplyForces, STATGROUP_Hagoromo, HAGOROMO_API);
DECLARE_CYCLE_STAT_EXTERN(TEXT("Physics VerletIntegrate"), STAT_PhysicsVerletIntegrate, STATGROUP_Hagoromo, HAGOROMO_API);

DECLARE_CYCLE_STAT_EXTERN(TEXT("Solver Initialize"), STAT_SolverInitialize, STATGROUP_Hagoromo, HAGOROMO_API);
DECLARE_CYCLE_STAT_EXTERN(TEXT("Solver PreSimulate"), STAT_SolverPreSimulate, STATGROUP_Hagoromo, HAGOROMO_API);
DECLARE_CYCLE_STAT_EXTERN(TEXT("Solver Simulate"), STAT_SolverSimulate, STATGROUP_Hagoromo, HAGOROMO_API);
DECLARE_CYCLE_STAT_EXTERN(TEXT("Solver OutputSimulateResult"), STAT_SolverOutputSimulateResult, STATGROUP_Hagoromo, HAGOROMO_API);

// ---------------------------------------------------------------------------------------
// Module
// ---------------------------------------------------------------------------------------
class FHagoromoModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
