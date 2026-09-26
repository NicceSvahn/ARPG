#include "OffensiveAttributeSet.h"

#include "Net/UnrealNetwork.h"

void UOffensiveAttributeSet::OnRep_CritChance(
    const FGameplayAttributeData& OldCritChance)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(
        UOffensiveAttributeSet,
        CritChance,
        OldCritChance
    );
}

void UOffensiveAttributeSet::OnRep_CritDamage(
    const FGameplayAttributeData& OldCritDamage)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(
        UOffensiveAttributeSet,
        CritDamage,
        OldCritDamage
    );
}

void UOffensiveAttributeSet::OnRep_AttackSpeed(
    const FGameplayAttributeData& OldAttackSpeed)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(
        UOffensiveAttributeSet,
        AttackSpeed,
        OldAttackSpeed
    );
}

void UOffensiveAttributeSet::OnRep_CastSpeed(
    const FGameplayAttributeData& OldCastSpeed)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(
        UOffensiveAttributeSet,
        CastSpeed,
        OldCastSpeed
    );
}

void UOffensiveAttributeSet::OnRep_AttackPower(
    const FGameplayAttributeData& OldAttackPower)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(
        UOffensiveAttributeSet,
        AttackPower,
        OldAttackPower
    );
}

void UOffensiveAttributeSet::OnRep_SpellPower(
    const FGameplayAttributeData& OldSpellPower)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(
        UOffensiveAttributeSet,
        SpellPower,
        OldSpellPower
    );
}


void UOffensiveAttributeSet::OnRep_WeaponDamage(
    const FGameplayAttributeData& OldWeaponDamage)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(
        UOffensiveAttributeSet,
        WeaponDamage,
        OldWeaponDamage
    );
}

void UOffensiveAttributeSet::OnRep_PhysicalDamage(
    const FGameplayAttributeData& OldPhysicalDamage)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(
        UOffensiveAttributeSet,
        PhysicalDamage,
        OldPhysicalDamage
    );
}

void UOffensiveAttributeSet::OnRep_MagicDamage(
    const FGameplayAttributeData& OldMagicDamage)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(
        UOffensiveAttributeSet,
        MagicDamage,
        OldMagicDamage
    );
}

void UOffensiveAttributeSet::GetLifetimeReplicatedProps(
    TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME_CONDITION_NOTIFY(
        UOffensiveAttributeSet,
        CritChance,
        COND_None,
        REPNOTIFY_Always
    );

    DOREPLIFETIME_CONDITION_NOTIFY(
        UOffensiveAttributeSet,
        CritDamage,
        COND_None,
        REPNOTIFY_Always
    );

    DOREPLIFETIME_CONDITION_NOTIFY(
        UOffensiveAttributeSet,
        AttackSpeed,
        COND_None,
        REPNOTIFY_Always
    );

    DOREPLIFETIME_CONDITION_NOTIFY(
        UOffensiveAttributeSet,
        CastSpeed,
        COND_None,
        REPNOTIFY_Always
    );

    DOREPLIFETIME_CONDITION_NOTIFY(
        UOffensiveAttributeSet,
        AttackPower,
        COND_None,
        REPNOTIFY_Always
    );

    DOREPLIFETIME_CONDITION_NOTIFY(
        UOffensiveAttributeSet,
        SpellPower,
        COND_None,
        REPNOTIFY_Always
    );


    DOREPLIFETIME_CONDITION_NOTIFY(
        UOffensiveAttributeSet,
        WeaponDamage,
        COND_None,
        REPNOTIFY_Always
    );

    DOREPLIFETIME_CONDITION_NOTIFY(
        UOffensiveAttributeSet,
        PhysicalDamage,
        COND_None,
        REPNOTIFY_Always
    );

    DOREPLIFETIME_CONDITION_NOTIFY(
        UOffensiveAttributeSet,
        MagicDamage,
        COND_None,
        REPNOTIFY_Always
    );

}
