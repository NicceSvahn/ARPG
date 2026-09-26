#include "PrimaryAttributeSet.h"

#include "Net/UnrealNetwork.h"

void UPrimaryAttributeSet::OnRep_Strength(
    const FGameplayAttributeData& OldStrength)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(
        UPrimaryAttributeSet,
        Strength,
        OldStrength
    );
}

void UPrimaryAttributeSet::OnRep_Dexterity(
    const FGameplayAttributeData& OldDexterity)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(
        UPrimaryAttributeSet,
        Dexterity,
        OldDexterity
    );
}

void UPrimaryAttributeSet::OnRep_Intellect(
    const FGameplayAttributeData& OldIntellect)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(
        UPrimaryAttributeSet,
        Intellect,
        OldIntellect
    );
}

void UPrimaryAttributeSet::GetLifetimeReplicatedProps(
    TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME_CONDITION_NOTIFY(
        UPrimaryAttributeSet,
        Strength,
        COND_None,
        REPNOTIFY_Always
    );

    DOREPLIFETIME_CONDITION_NOTIFY(
        UPrimaryAttributeSet,
        Dexterity,
        COND_None,
        REPNOTIFY_Always
    );

    DOREPLIFETIME_CONDITION_NOTIFY(
        UPrimaryAttributeSet,
        Intellect,
        COND_None,
        REPNOTIFY_Always
    );

}
