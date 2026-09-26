#pragma once

#include "CoreMinimal.h"
#include "GenericAttributeSet.h"
#include "Net/UnrealNetwork.h"
#include "HealthAttributeSet.generated.h"

UCLASS()
class TESTGAME_API UHealthAttributeSet : public UGenericAttributeSet
{
    GENERATED_BODY()

public:
    virtual bool PreGameplayEffectExecute(
        FGameplayEffectModCallbackData& Data
    ) override;

    virtual void PostGameplayEffectExecute(
        const FGameplayEffectModCallbackData& Data
    ) override;

    virtual void GetLifetimeReplicatedProps(
        TArray<FLifetimeProperty>& OutLifetimeProps
    ) const override;

    UPROPERTY(
        BlueprintReadOnly,
        ReplicatedUsing = OnRep_Health,
        Category = "Attributes"
    )
    FGameplayAttributeData Health;

    PLAY_ATTRIBUTE_ACCESSORS(
        UHealthAttributeSet,
        Health
    );

    UPROPERTY(
        BlueprintReadOnly,
        ReplicatedUsing = OnRep_MaxHealth,
        Category = "Attributes"
    )
    FGameplayAttributeData MaxHealth;

    PLAY_ATTRIBUTE_ACCESSORS(
        UHealthAttributeSet,
        MaxHealth
    );

    UPROPERTY(
        BlueprintReadOnly,
        ReplicatedUsing = OnRep_LifeRegeneration,
        Category = "Attributes"
    )
    FGameplayAttributeData LifeRegeneration;

    PLAY_ATTRIBUTE_ACCESSORS(
        UHealthAttributeSet,
        LifeRegeneration
    );


    // Meta attribute. This is intentionally not replicated.
    // Damage executions write a positive value here; this set consumes it
    // and subtracts it from Health.
    UPROPERTY(BlueprintReadOnly, Category = "Attributes|Meta")
    FGameplayAttributeData IncomingDamage;

    PLAY_ATTRIBUTE_ACCESSORS(
        UHealthAttributeSet,
        IncomingDamage
    );

protected:
    UFUNCTION()
    void OnRep_Health(
        const FGameplayAttributeData& OldHealth
    );

    UFUNCTION()
    void OnRep_MaxHealth(
        const FGameplayAttributeData& OldMaxHealth
    );

    UFUNCTION()
    void OnRep_LifeRegeneration(
        const FGameplayAttributeData& OldLifeRegeneration
    );
};