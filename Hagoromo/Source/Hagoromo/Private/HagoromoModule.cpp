// Hagoromo : Copyright (c) 2025 nozoxa_0131, MIT License

#include "HagoromoModule.h"

#include "Misc/ConfigContext.h"
#include "Misc/ConfigCacheIni.h"

DEFINE_LOG_CATEGORY(LogHagoromoRuntime);

FString HGMGlobal::IniFileName {};
FHGMReal HGMGlobal::TargetFrameRate = 60.0;

DEFINE_STAT(STAT_CollisionCalculateBodyColliderContacts);
DEFINE_STAT(STAT_CollisionCalculateBodyColliderContactsForVerticalEdge);
DEFINE_STAT(STAT_CollisionCalculateBodyColliderContactsForHorizontalEdge);

DEFINE_STAT(STAT_ConstraintVerticalStructuralConstraint);
DEFINE_STAT(STAT_ConstraintHorizontalStructuralConstraint);
DEFINE_STAT(STAT_ConstraintShearConstraint);
DEFINE_STAT(STAT_ConstraintVerticalBendConstraint);
DEFINE_STAT(STAT_ConstraintHorizontalBendConstraint);
DEFINE_STAT(STAT_ConstraintRelativeLimitAngleConstraint);
DEFINE_STAT(STAT_ConstraintAnimPoseMovableRadiusConstraint);
DEFINE_STAT(STAT_ConstraintAnimPoseLimitAngleConstraint);
DEFINE_STAT(STAT_ConstraintAnimPosePlanarConstraint);

DEFINE_STAT(STAT_PhysicsApplyForces);
DEFINE_STAT(STAT_PhysicsVerletIntegrate);

DEFINE_STAT(STAT_SolverInitialize);
DEFINE_STAT(STAT_SolverPreSimulate);
DEFINE_STAT(STAT_SolverSimulate);
DEFINE_STAT(STAT_SolverOutputSimulateResult);

#define LOCTEXT_NAMESPACE "FHagoromoModule"


void FHagoromoModule::StartupModule()
{
	FString HagoromoIni {};
	if (GConfig)
	{
		FConfigContext::ReadIntoGConfig().Load(TEXT("Hagoromo"), HGMGlobal::IniFileName);
#if HGM_USE_FLOAT32
		GConfig->GetFloat(TEXT("/Script/Hagoromo.HagoromoSettings"), TEXT("TargetFrameRate"), HGMGlobal::TargetFrameRate, HGMGlobal::IniFileName);
#else
		GConfig->GetDouble(TEXT("/Script/Hagoromo.HagoromoSettings"), TEXT("TargetFrameRate"), HGMGlobal::TargetFrameRate, HGMGlobal::IniFileName);
#endif
	}
}


void FHagoromoModule::ShutdownModule()
{
}


#undef LOCTEXT_NAMESPACE


IMPLEMENT_MODULE(FHagoromoModule, Hagoromo)