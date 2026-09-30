#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "AnimNotify_SpawnDirectionalNiagara.generated.h"

class UNiagaraSystem;

/**
 * Generic animation notify for short-lived directional Niagara effects.
 *
 * All calculations and spawning are handled in C++.
 * The montage only configures which asset to use and its presentation values.
 */
UCLASS(
    meta = (DisplayName = "Spawn Directional Niagara")
)
class TESTGAME_API UAnimNotify_SpawnDirectionalNiagara
    : public UAnimNotify
{
    GENERATED_BODY()

public:
    virtual void Notify(
        USkeletalMeshComponent* MeshComp,
        UAnimSequenceBase* Animation,
        const FAnimNotifyEventReference& EventReference
    ) override;

protected:
    UPROPERTY(
        EditAnywhere,
        BlueprintReadOnly,
        Category = "VFX"
    )
    TObjectPtr<UNiagaraSystem> NiagaraSystem = nullptr;

    /**
     * Actor-relative offset in centimeters.
     * X = forward, Y = right, Z = up.
     */
    UPROPERTY(
        EditAnywhere,
        BlueprintReadOnly,
        Category = "VFX|Transform",
        meta = (Units = "cm")
    )
    FVector LocalOffset = FVector::ZeroVector;

    /**
     * Rotation applied on top of the source actor rotation.
     * Useful for non-billboard effects or asset-specific correction.
     */
    UPROPERTY(
        EditAnywhere,
        BlueprintReadOnly,
        Category = "VFX|Transform"
    )
    FRotator LocalRotationOffset = FRotator::ZeroRotator;

    UPROPERTY(
        EditAnywhere,
        BlueprintReadOnly,
        Category = "VFX|Transform"
    )
    FVector Scale = FVector::OneVector;

    /**
     * Niagara user float that drives Particles.SpriteRotation.
     * Leave as None to skip screen-space sprite rotation entirely.
     */
    UPROPERTY(
        EditAnywhere,
        BlueprintReadOnly,
        Category = "VFX|Directional Sprite"
    )
    FName SpriteRotationParameterName =
        TEXT("User.SpriteRotation");

    /**
     * Constant correction for the visual asset's own zero-degree orientation.
     * Example: use 90 or -90 if the arc artwork is authored sideways.
     */
    UPROPERTY(
        EditAnywhere,
        BlueprintReadOnly,
        Category = "VFX|Directional Sprite",
        meta = (Units = "deg")
    )
    float SpriteRotationOffsetDegrees = 0.0f;

    /**
     * Normally 1. Set to -1 if this particular sprite rotates in the opposite
     * visual direction from the screen-space angle.
     */
    UPROPERTY(
        EditAnywhere,
        BlueprintReadOnly,
        Category = "VFX|Directional Sprite"
    )
    float SpriteRotationMultiplier = 1.0f;

    UPROPERTY(
        EditAnywhere,
        BlueprintReadOnly,
        Category = "VFX|Lifetime"
    )
    bool bAutoDestroy = true;

    UPROPERTY(
        EditAnywhere,
        BlueprintReadOnly,
        Category = "VFX|Performance"
    )
    bool bPreCullCheck = true;
};
