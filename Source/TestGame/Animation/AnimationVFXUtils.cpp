#include "AnimationVFXUtils.h"

#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "GameFramework/PlayerController.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"


namespace
{
    /**
     * Returns the actor's forward direction using yaw only.
     *
     * This deliberately ignores pitch and roll because directional
     * ARPG effects should follow the character across the ground plane.
     */
    FVector GetPlanarForward(const AActor* SourceActor)
    {
        if (!SourceActor)
        {
            return FVector::ForwardVector;
        }

        const float Yaw =
            SourceActor->GetActorRotation().Yaw;

        return FRotator(
            0.0f,
            Yaw,
            0.0f
        ).Vector();
    }

    /**
     * Returns the actor's right direction using yaw only.
     */
    FVector GetPlanarRight(const AActor* SourceActor)
    {
        if (!SourceActor)
        {
            return FVector::RightVector;
        }

        const float Yaw =
            SourceActor->GetActorRotation().Yaw;

        return FRotator(
            0.0f,
            Yaw + 90.0f,
            0.0f
        ).Vector();
    }

    /**
     * Returns an actor rotation using yaw only.
     */
    FRotator GetPlanarRotation(const AActor* SourceActor)
    {
        if (!SourceActor)
        {
            return FRotator::ZeroRotator;
        }

        return FRotator(
            0.0f,
            SourceActor->GetActorRotation().Yaw,
            0.0f
        );
    }
}


bool AnimationVFXUtils::CanSpawnCosmeticVFX(
    const UObject* WorldContextObject)
{
    if (!WorldContextObject)
    {
        return false;
    }

    const UWorld* World =
        WorldContextObject->GetWorld();

    if (!World)
    {
        return false;
    }

    return World->GetNetMode() != NM_DedicatedServer;
}


APlayerController*
AnimationVFXUtils::GetLocalPlayerController(
    const UObject* WorldContextObject)
{
    if (!WorldContextObject)
    {
        return nullptr;
    }

    UWorld* World =
        WorldContextObject->GetWorld();

    if (!World)
    {
        return nullptr;
    }

    for (
        FConstPlayerControllerIterator It =
        World->GetPlayerControllerIterator();
        It;
        ++It)
    {
        APlayerController* PlayerController =
            It->Get();

        if (
            PlayerController &&
            PlayerController->IsLocalController()
            )
        {
            return PlayerController;
        }
    }

    return nullptr;
}


FVector AnimationVFXUtils::CalculateActorRelativeLocation(
    const AActor* SourceActor,
    const FVector& LocalOffset)
{
    if (!SourceActor)
    {
        return FVector::ZeroVector;
    }

    const FVector Forward =
        GetPlanarForward(SourceActor);

    const FVector Right =
        GetPlanarRight(SourceActor);

    return
        SourceActor->GetActorLocation()
        + Forward * LocalOffset.X
        + Right * LocalOffset.Y
        + FVector::UpVector * LocalOffset.Z;
}


FTransform AnimationVFXUtils::CalculateActorRelativeTransform(
    const AActor* SourceActor,
    const FVector& LocalOffset,
    const FRotator& LocalRotationOffset,
    const FVector& Scale)
{
    if (!SourceActor)
    {
        return FTransform::Identity;
    }

    const FVector WorldLocation =
        CalculateActorRelativeLocation(
            SourceActor,
            LocalOffset
        );

    const FQuat WorldRotation =
        GetPlanarRotation(SourceActor).Quaternion()
        * LocalRotationOffset.Quaternion();

    return FTransform(
        WorldRotation,
        WorldLocation,
        Scale
    );
}


bool AnimationVFXUtils::TryCalculateScreenSpaceAngle(
    const UObject* WorldContextObject,
    const FVector& WorldOrigin,
    const FVector& WorldDirection,
    float& OutAngleDegrees,
    float SampleDistance)
{
    OutAngleDegrees = 0.0f;

    APlayerController* PlayerController =
        GetLocalPlayerController(
            WorldContextObject
        );

    if (!PlayerController)
    {
        return false;
    }

    const FVector SafeDirection =
        WorldDirection.GetSafeNormal();

    if (SafeDirection.IsNearlyZero())
    {
        return false;
    }

    const float SafeSampleDistance =
        FMath::Max(
            FMath::Abs(SampleDistance),
            1.0f
        );

    const FVector WorldEnd =
        WorldOrigin
        + SafeDirection * SafeSampleDistance;

    FVector2D ScreenOrigin;
    FVector2D ScreenEnd;

    const bool bProjectedOrigin =
        PlayerController->ProjectWorldLocationToScreen(
            WorldOrigin,
            ScreenOrigin,
            false
        );

    const bool bProjectedEnd =
        PlayerController->ProjectWorldLocationToScreen(
            WorldEnd,
            ScreenEnd,
            false
        );

    if (
        !bProjectedOrigin ||
        !bProjectedEnd
        )
    {
        return false;
    }

    const FVector2D ScreenDirection =
        ScreenEnd - ScreenOrigin;

    if (ScreenDirection.IsNearlyZero())
    {
        return false;
    }

    OutAngleDegrees =
        FMath::RadiansToDegrees(
            FMath::Atan2(
                ScreenDirection.Y,
                ScreenDirection.X
            )
        );

    return true;
}


UNiagaraComponent*
AnimationVFXUtils::SpawnNiagaraAtTransform(
    const UObject* WorldContextObject,
    UNiagaraSystem* NiagaraSystem,
    const FTransform& SpawnTransform,
    bool bAutoDestroy,
    bool bAutoActivate,
    bool bPreCullCheck)
{
    if (
        !CanSpawnCosmeticVFX(WorldContextObject) ||
        !NiagaraSystem
        )
    {
        return nullptr;
    }

    return UNiagaraFunctionLibrary::SpawnSystemAtLocation(
        WorldContextObject,
        NiagaraSystem,
        SpawnTransform.GetLocation(),
        SpawnTransform.GetRotation().Rotator(),
        SpawnTransform.GetScale3D(),
        bAutoDestroy,
        bAutoActivate,
        ENCPoolMethod::None,
        bPreCullCheck
    );
}


UNiagaraComponent*
AnimationVFXUtils::SpawnDirectionalNiagara(
    const UObject* WorldContextObject,
    AActor* SourceActor,
    UNiagaraSystem* NiagaraSystem,
    const FVector& LocalOffset,
    const FRotator& LocalRotationOffset,
    const FVector& Scale,
    FName SpriteRotationParameterName,
    float SpriteRotationOffsetDegrees,
    float SpriteRotationMultiplier,
    bool bAutoDestroy,
    bool bPreCullCheck)
{
    if (
        !SourceActor ||
        !NiagaraSystem ||
        !CanSpawnCosmeticVFX(WorldContextObject)
        )
    {
        return nullptr;
    }

    /*
     * POSITION:
     *
     * X = fixed distance in front of Manny
     * Y = fixed sideways offset
     * Z = fixed vertical offset
     *
     * All based solely on Manny's yaw.
     */
    const FTransform SpawnTransform =
        CalculateActorRelativeTransform(
            SourceActor,
            LocalOffset,
            LocalRotationOffset,
            Scale
        );

    const bool bHasRotationParameter =
        !SpriteRotationParameterName.IsNone();

    float ScreenSpaceAngle = 0.0f;

    /*
     * ROTATION:
     *
     * Calculate the on-screen direction from MANNY'S position
     * and MANNY'S planar forward direction.
     *
     * Do not use the Niagara spawn position here.
     */
    const FVector PlanarForward =
        GetPlanarForward(SourceActor);

    const bool bHasScreenSpaceAngle =
        bHasRotationParameter &&
        TryCalculateScreenSpaceAngle(
            WorldContextObject,
            SourceActor->GetActorLocation(),
            PlanarForward,
            ScreenSpaceAngle
        );

    /*
     * If we use a sprite rotation parameter, spawn inactive.
     *
     * That lets us set User.CleaveRotation before the burst
     * particle is created.
     */
    UNiagaraComponent* NiagaraComponent =
        SpawnNiagaraAtTransform(
            WorldContextObject,
            NiagaraSystem,
            SpawnTransform,
            bAutoDestroy,
            !bHasRotationParameter,
            bPreCullCheck
        );

    if (!NiagaraComponent)
    {
        return nullptr;
    }

    if (bHasRotationParameter)
    {
        const float FinalRotationDegrees =
            bHasScreenSpaceAngle
            ? FMath::UnwindDegrees(
                ScreenSpaceAngle
                * SpriteRotationMultiplier
                + SpriteRotationOffsetDegrees
            )
            : SpriteRotationOffsetDegrees;

        NiagaraComponent->SetVariableFloat(
            SpriteRotationParameterName,
            FinalRotationDegrees
        );

        NiagaraComponent->Activate(true);
    }

    return NiagaraComponent;
}