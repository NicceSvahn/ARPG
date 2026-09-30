#pragma once

#include "CoreMinimal.h"

class AActor;
class APlayerController;
class UNiagaraComponent;
class UNiagaraSystem;
class UObject;

namespace AnimationVFXUtils
{
    /**
     * Cosmetic VFX should not be spawned on a dedicated server.
     */
    TESTGAME_API bool CanSpawnCosmeticVFX(
        const UObject* WorldContextObject
    );

    /**
     * Returns the first local player controller in the current world.
     * Returns nullptr on dedicated servers or when no local player exists.
     */
    TESTGAME_API APlayerController* GetLocalPlayerController(
        const UObject* WorldContextObject
    );

    /**
     * Converts an actor-relative offset into a world-space location.
     *
     * LocalOffset.X = forward/backward
     * LocalOffset.Y = right/left
     * LocalOffset.Z = up/down
     */
    TESTGAME_API FVector CalculateActorRelativeLocation(
        const AActor* SourceActor,
        const FVector& LocalOffset
    );

    /**
     * Builds a world-space transform from an actor-relative offset and
     * a local rotation offset.
     */
    TESTGAME_API FTransform CalculateActorRelativeTransform(
        const AActor* SourceActor,
        const FVector& LocalOffset,
        const FRotator& LocalRotationOffset = FRotator::ZeroRotator,
        const FVector& Scale = FVector::OneVector
    );

    /**
     * Projects a world-space direction onto the local player's screen and
     * returns its screen-space angle in degrees.
     *
     * This is intended for camera-facing Niagara sprites whose
     * Particles.SpriteRotation should visually follow a world-space direction.
     */
    TESTGAME_API bool TryCalculateScreenSpaceAngle(
        const UObject* WorldContextObject,
        const FVector& WorldOrigin,
        const FVector& WorldDirection,
        float& OutAngleDegrees,
        float SampleDistance = 100.0f
    );

    /**
     * Spawns a Niagara system at a world-space transform.
     */
    TESTGAME_API UNiagaraComponent* SpawnNiagaraAtTransform(
        const UObject* WorldContextObject,
        UNiagaraSystem* NiagaraSystem,
        const FTransform& SpawnTransform,
        bool bAutoDestroy = true,
        bool bAutoActivate = true,
        bool bPreCullCheck = true
    );

    /**
     * Spawns a Niagara system relative to SourceActor and optionally writes
     * the actor's screen-space facing angle into a Niagara user float before
     * activating the system.
     *
     * The system is intentionally spawned inactive when a rotation parameter
     * is supplied. The parameter is set first, then the system is activated,
     * so burst particles see the correct value on their first frame.
     */
    TESTGAME_API UNiagaraComponent* SpawnDirectionalNiagara(
        const UObject* WorldContextObject,
        AActor* SourceActor,
        UNiagaraSystem* NiagaraSystem,
        const FVector& LocalOffset,
        const FRotator& LocalRotationOffset,
        const FVector& Scale,
        FName SpriteRotationParameterName,
        float SpriteRotationOffsetDegrees = 0.0f,
        float SpriteRotationMultiplier = 1.0f,
        bool bAutoDestroy = true,
        bool bPreCullCheck = true
    );
}
