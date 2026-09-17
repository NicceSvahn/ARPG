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

protected:
    UFUNCTION()
    void OnRep_Health(
        const FGameplayAttributeData& OldHealth
    );

    UFUNCTION()
    void OnRep_MaxHealth(
        const FGameplayAttributeData& OldMaxHealth
    );
};