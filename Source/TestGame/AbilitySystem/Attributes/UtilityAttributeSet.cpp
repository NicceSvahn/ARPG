#include "UtilityAttributeSet.h"

#include "Net/UnrealNetwork.h"

void UUtilityAttributeSet::OnRep_GoldFind(
    const FGameplayAttributeData& OldGoldFind)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(
        UUtilityAttributeSet,
        GoldFind,
        OldGoldFind
    );
}

void UUtilityAttributeSet::OnRep_PickupRadius(
    const FGameplayAttributeData& OldPickupRadius)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(
        UUtilityAttributeSet,
        PickupRadius,
        OldPickupRadius
    );
}

void UUtilityAttributeSet::OnRep_ResourceCostReduction(
    const FGameplayAttributeData& OldResourceCostReduction)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(
        UUtilityAttributeSet,
        ResourceCostReduction,
        OldResourceCostReduction
    );
}

void UUtilityAttributeSet::OnRep_CooldownReduction(
    const FGameplayAttributeData& OldCooldownReduction)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(
        UUtilityAttributeSet,
        CooldownReduction,
        OldCooldownReduction
    );
}

void UUtilityAttributeSet::GetLifetimeReplicatedProps(
    TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME_CONDITION_NOTIFY(
        UUtilityAttributeSet,
        GoldFind,
        COND_None,
        REPNOTIFY_Always
    );

    DOREPLIFETIME_CONDITION_NOTIFY(
        UUtilityAttributeSet,
        PickupRadius,
        COND_None,
        REPNOTIFY_Always
    );

    DOREPLIFETIME_CONDITION_NOTIFY(
        UUtilityAttributeSet,
        ResourceCostReduction,
        COND_None,
        REPNOTIFY_Always
    );

    DOREPLIFETIME_CONDITION_NOTIFY(
        UUtilityAttributeSet,
        CooldownReduction,
        COND_None,
        REPNOTIFY_Always
    );

}
