#include "DefensiveAttributeSet.h"

#include "Net/UnrealNetwork.h"

void UDefensiveAttributeSet::OnRep_Armour(
    const FGameplayAttributeData& OldArmour)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(
        UDefensiveAttributeSet,
        Armour,
        OldArmour
    );
}

void UDefensiveAttributeSet::OnRep_BlockChance(
    const FGameplayAttributeData& OldBlockChance)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(
        UDefensiveAttributeSet,
        BlockChance,
        OldBlockChance
    );
}

void UDefensiveAttributeSet::OnRep_MagicResistance(
    const FGameplayAttributeData& OldMagicResistance)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(
        UDefensiveAttributeSet,
        MagicResistance,
        OldMagicResistance
    );
}

void UDefensiveAttributeSet::OnRep_Evasion(
    const FGameplayAttributeData& OldEvasion)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(
        UDefensiveAttributeSet,
        Evasion,
        OldEvasion
    );
}

void UDefensiveAttributeSet::GetLifetimeReplicatedProps(
    TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME_CONDITION_NOTIFY(
        UDefensiveAttributeSet,
        Armour,
        COND_None,
        REPNOTIFY_Always
    );

    DOREPLIFETIME_CONDITION_NOTIFY(
        UDefensiveAttributeSet,
        BlockChance,
        COND_None,
        REPNOTIFY_Always
    );

    DOREPLIFETIME_CONDITION_NOTIFY(
        UDefensiveAttributeSet,
        MagicResistance,
        COND_None,
        REPNOTIFY_Always
    );

    DOREPLIFETIME_CONDITION_NOTIFY(
        UDefensiveAttributeSet,
        Evasion,
        COND_None,
        REPNOTIFY_Always
    );

}
