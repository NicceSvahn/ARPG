#pragma once

#include "CoreMinimal.h"
#include "GenericAttributeSet.h"
#include "Net/UnrealNetwork.h"
#include "UtilityAttributeSet.generated.h"

UCLASS()
class TESTGAME_API UUtilityAttributeSet : public UGenericAttributeSet
{
    GENERATED_BODY()

public:
    virtual void GetLifetimeReplicatedProps(
        TArray<FLifetimeProperty>& OutLifetimeProps
    ) const override;

    UPROPERTY(
        BlueprintReadOnly,
        ReplicatedUsing = OnRep_GoldFind,
        Category = "Attributes|Utility"
    )
    FGameplayAttributeData GoldFind;

    PLAY_ATTRIBUTE_ACCESSORS(UUtilityAttributeSet, GoldFind);

    UPROPERTY(
        BlueprintReadOnly,
        ReplicatedUsing = OnRep_PickupRadius,
        Category = "Attributes|Utility"
    )
    FGameplayAttributeData PickupRadius;

    PLAY_ATTRIBUTE_ACCESSORS(UUtilityAttributeSet, PickupRadius);

    UPROPERTY(
        BlueprintReadOnly,
        ReplicatedUsing = OnRep_ResourceCostReduction,
        Category = "Attributes|Utility"
    )
    FGameplayAttributeData ResourceCostReduction;

    PLAY_ATTRIBUTE_ACCESSORS(UUtilityAttributeSet, ResourceCostReduction);

    UPROPERTY(
        BlueprintReadOnly,
        ReplicatedUsing = OnRep_CooldownReduction,
        Category = "Attributes|Utility"
    )
    FGameplayAttributeData CooldownReduction;

    PLAY_ATTRIBUTE_ACCESSORS(UUtilityAttributeSet, CooldownReduction);

protected:
    UFUNCTION()
    void OnRep_GoldFind(
        const FGameplayAttributeData& OldGoldFind
    );

    UFUNCTION()
    void OnRep_PickupRadius(
        const FGameplayAttributeData& OldPickupRadius
    );

    UFUNCTION()
    void OnRep_ResourceCostReduction(
        const FGameplayAttributeData& OldResourceCostReduction
    );

    UFUNCTION()
    void OnRep_CooldownReduction(
        const FGameplayAttributeData& OldCooldownReduction
    );

};
