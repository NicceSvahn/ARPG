#pragma once

#include "CoreMinimal.h"
#include "TestGame/AbilitySystem/Abilities/GA_GenericAbility.h"
#include "GA_FrostNova.generated.h"

class AGenericCharacter;
class UAnimMontage;
class UAbilitySystemComponent;
class UGameplayEffect;
class UNiagaraSystem;

UCLASS()
class TESTGAME_API UGA_FrostNova : public UGA_GenericAbility
{
	GENERATED_BODY()

public:
	UGA_FrostNova();

    virtual void ActivateAbility(
        const FGameplayAbilitySpecHandle Handle,
        const FGameplayAbilityActorInfo* ActorInfo,
        const FGameplayAbilityActivationInfo ActivationInfo,
        const FGameplayEventData* TriggerEventData
    ) override;

protected:
    UPROPERTY(
        EditDefaultsOnly,
        BlueprintReadOnly,
        Category = "Frost Nova",
        meta = (ClampMin = "0.0", Units = "cm")
    )
    float NovaRadius = 400.0f;

    UPROPERTY(
        EditDefaultsOnly,
        BlueprintReadOnly,
        Category = "Frost Nova",
        meta = (ClampMin = "0.0")
    )
    float NovaDamage = 15.0f;

    UPROPERTY(
        EditDefaultsOnly,
        BlueprintReadOnly,
        Category = "Frost Nova",
        meta = (ClampMin = "0.0", Units = "s")
    )
    float FreezeDuration = 3.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Frost Nova")
    TSubclassOf<AGenericCharacter> AffectedCharacterClass;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Frost Nova")
    TSubclassOf<UGameplayEffect> DamageEffect;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Frost Nova")
    TSubclassOf<UGameplayEffect> FreezeEffect;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Frost Nova")
    TObjectPtr<UAnimMontage> FrostNovaMontage = nullptr;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Frost Nova|Visuals")
    TObjectPtr<UNiagaraSystem>FrostNovaEffect = nullptr;

private:
    bool ApplyEffectToTarget(
        UAbilitySystemComponent* SourceASC,
        UAbilitySystemComponent* TargetASC,
        TSubclassOf<UGameplayEffect> EffectClass,
        float DurationOverride,
        bool bSetDamageMagnitude
    );

};
