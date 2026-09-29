#pragma once

#include "CoreMinimal.h"
#include "GA_GenericMelee.h"
#include "GA_Cleave.generated.h"

class AGenericCharacter;

UCLASS()
class TESTGAME_API UGA_Cleave : public UGA_GenericMelee
{
    GENERATED_BODY()

public:
    UGA_Cleave();
    virtual bool RequiresTarget() const override
    {
        return false;
    }

    virtual float GetMaximumRange() const override
    {
        return 0.0f;
    }

    UPROPERTY(
        EditDefaultsOnly,
        BlueprintReadOnly,
        Category = "Ability|Cleave|Animation"
    )
    TObjectPtr<UAnimMontage> CleaveMontageLR = nullptr;

    UPROPERTY(
        EditDefaultsOnly,
        BlueprintReadOnly,
        Category = "Ability|Cleave|Animation"
    )
    TObjectPtr<UAnimMontage> CleaveMontageRL = nullptr;

protected:
    virtual void ActivateAbility(
        const FGameplayAbilitySpecHandle Handle,
        const FGameplayAbilityActorInfo* ActorInfo,
        const FGameplayAbilityActivationInfo ActivationInfo,
        const FGameplayEventData* TriggerEventData
    ) override;

    virtual void OnMeleeHit(
        AActor* TargetActor
    ) override;

    UPROPERTY(
        EditDefaultsOnly,
        BlueprintReadOnly,
        Category = "Ability|Cleave",
        meta = (ClampMin = "0.0", Units = "cm")
    )
    float CleaveRange = 250.0f;

    UPROPERTY(
        EditDefaultsOnly,
        BlueprintReadOnly,
        Category = "Ability|Cleave",
        meta = (ClampMin = "0.0", ClampMax = "360.0")
    )
    float CleaveArcDegrees = 180.0f;

    UPROPERTY(
        EditDefaultsOnly,
        BlueprintReadOnly,
        Category = "Ability|Cleave"
    )
    TSubclassOf<AGenericCharacter> AffectedCharacterClass;

protected:
    virtual UAnimMontage*
        GetAttackMontageForActivation() override;

private:
    void ApplyCleaveDamage(
        AGenericCharacter* SourceCharacter
    );

    bool bUseLeftToRightNext = true;
};