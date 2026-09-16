#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "Abilities/GameplayAbilityTargetTypes.h"
#include "GameFramework/Pawn.h"
#include "GA_GenericAbility.generated.h"

class AGenericCharacter;
class UTexture2D;
class UGameplayEffect;
class UTestGameAbilitySystemComponent;

UCLASS(Abstract)
class TESTGAME_API UGA_GenericAbility : public UGameplayAbility
{
    GENERATED_BODY()

public:
    UGA_GenericAbility();

    UFUNCTION(BlueprintPure, Category = "Ability|UI")
    FText GetAbilityName() const { return AbilityName; }

    UFUNCTION(BlueprintPure, Category = "Ability|UI")
    UTexture2D* GetAbilityIcon() const { return AbilityIcon; }

    UFUNCTION(BlueprintPure, Category = "Ability|UI")
    FText GetAbilityDescription() const { return AbilityDescription; }

    FGameplayTag GetCooldownTag() const
    {
        return CooldownTag;
    }

    virtual void ActivateAbility(
        const FGameplayAbilitySpecHandle Handle,
        const FGameplayAbilityActorInfo* ActorInfo,
        const FGameplayAbilityActivationInfo ActivationInfo,
        const FGameplayEventData* TriggerEventData
    ) override;

protected:
    AGenericCharacter* GetGenericCharacter() const;

    UTestGameAbilitySystemComponent* GetTestGameASC() const;

    void SendTargetDataToServer(
        const FGameplayAbilitySpecHandle Handle,
        const FGameplayAbilityActivationInfo ActivationInfo
    );

    void WaitForTargetData();

    void HandleTargetDataReceived(
        const FGameplayAbilityTargetDataHandle& Data,
        FGameplayTag ActivationTag
    );

    virtual void OnTargetDataReady(
        const FGameplayAbilityTargetDataHandle& Data
    );

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability|UI")
    FText AbilityName;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability|UI")
    TObjectPtr<UTexture2D> AbilityIcon = nullptr;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability|UI")
    FText AbilityDescription;

    UPROPERTY(
        EditDefaultsOnly,
        BlueprintReadOnly,
        Category = "Ability|Cooldown",
        meta = (ClampMin = "0.0")
    )
    float CooldownDuration = 0.0f;

    UPROPERTY(
        EditDefaultsOnly,
        BlueprintReadOnly,
        Category = "Ability|Cooldown",
        meta = (Categories = "Cooldown.Ability")
    )
    FGameplayTag CooldownTag;

    UPROPERTY(
        EditDefaultsOnly,
        BlueprintReadOnly,
        Category = "Ability|Cooldown"
    )
    TSubclassOf<UGameplayEffect> CooldownEffectTemplate;

    mutable FGameplayTagContainer CooldownTagContainer;

    virtual const FGameplayTagContainer* GetCooldownTags() const override;

    virtual void ApplyCooldown(
        const FGameplayAbilitySpecHandle Handle,
        const FGameplayAbilityActorInfo* ActorInfo,
        const FGameplayAbilityActivationInfo ActivationInfo
    ) const override;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability|Cost", meta = (ClampMin = "0.0"))
    float ResourceCost = 0.0f;

    virtual bool CheckCost(
        const FGameplayAbilitySpecHandle Handle,
        const FGameplayAbilityActorInfo* ActorInfo,
        OUT FGameplayTagContainer* OptionalRelevantTags = nullptr
    ) const override;

    UPROPERTY(
        EditDefaultsOnly,
        BlueprintReadOnly,
        Category = "Ability|Cost"
    )
    TSubclassOf<UGameplayEffect> ResourceCostEffect;

    virtual void ApplyCost(
        const FGameplayAbilitySpecHandle Handle,
        const FGameplayAbilityActorInfo* ActorInfo,
        const FGameplayAbilityActivationInfo ActivationInfo
    ) const override;

    bool StartTargetedAbility(
        const FGameplayAbilitySpecHandle Handle,
        const FGameplayAbilityActorInfo* ActorInfo,
        const FGameplayAbilityActivationInfo ActivationInfo
    );

    UPROPERTY(
        EditDefaultsOnly,
        BlueprintReadOnly,
        Category = "Ability|Targeting"
    )
    bool bRequiresTargetData = false;

};