#pragma once

#include "CoreMinimal.h"
#include "GenericAttributeSet.h"
#include "Net/UnrealNetwork.h"
#include "OffensiveAttributeSet.generated.h"

UCLASS()
class TESTGAME_API UOffensiveAttributeSet : public UGenericAttributeSet
{
    GENERATED_BODY()

public:
    virtual void GetLifetimeReplicatedProps(
        TArray<FLifetimeProperty>& OutLifetimeProps
    ) const override;

    UPROPERTY(
        BlueprintReadOnly,
        ReplicatedUsing = OnRep_CritChance,
        Category = "Attributes|Offensive"
    )
    FGameplayAttributeData CritChance;

    PLAY_ATTRIBUTE_ACCESSORS(UOffensiveAttributeSet, CritChance);

    UPROPERTY(
        BlueprintReadOnly,
        ReplicatedUsing = OnRep_CritDamage,
        Category = "Attributes|Offensive"
    )
    FGameplayAttributeData CritDamage;

    PLAY_ATTRIBUTE_ACCESSORS(UOffensiveAttributeSet, CritDamage);

    UPROPERTY(
        BlueprintReadOnly,
        ReplicatedUsing = OnRep_AttackSpeed,
        Category = "Attributes|Offensive"
    )
    FGameplayAttributeData AttackSpeed;

    PLAY_ATTRIBUTE_ACCESSORS(UOffensiveAttributeSet, AttackSpeed);

    UPROPERTY(
        BlueprintReadOnly,
        ReplicatedUsing = OnRep_CastSpeed,
        Category = "Attributes|Offensive"
    )
    FGameplayAttributeData CastSpeed;

    PLAY_ATTRIBUTE_ACCESSORS(UOffensiveAttributeSet, CastSpeed);

    UPROPERTY(
        BlueprintReadOnly,
        ReplicatedUsing = OnRep_AttackPower,
        Category = "Attributes|Offensive"
    )
    FGameplayAttributeData AttackPower;

    PLAY_ATTRIBUTE_ACCESSORS(UOffensiveAttributeSet, AttackPower);

    UPROPERTY(
        BlueprintReadOnly,
        ReplicatedUsing = OnRep_SpellPower,
        Category = "Attributes|Offensive"
    )
    FGameplayAttributeData SpellPower;

    PLAY_ATTRIBUTE_ACCESSORS(UOffensiveAttributeSet, SpellPower);


    UPROPERTY(
        BlueprintReadOnly,
        ReplicatedUsing = OnRep_WeaponDamage,
        Category = "Attributes|Offensive"
    )
    FGameplayAttributeData WeaponDamage;

    PLAY_ATTRIBUTE_ACCESSORS(UOffensiveAttributeSet, WeaponDamage);

    UPROPERTY(
        BlueprintReadOnly,
        ReplicatedUsing = OnRep_PhysicalDamage,
        Category = "Attributes|Offensive"
    )
    FGameplayAttributeData PhysicalDamage;

    PLAY_ATTRIBUTE_ACCESSORS(UOffensiveAttributeSet, PhysicalDamage);

    UPROPERTY(
        BlueprintReadOnly,
        ReplicatedUsing = OnRep_MagicDamage,
        Category = "Attributes|Offensive"
    )
    FGameplayAttributeData MagicDamage;

    PLAY_ATTRIBUTE_ACCESSORS(UOffensiveAttributeSet, MagicDamage);

protected:
    UFUNCTION()
    void OnRep_CritChance(
        const FGameplayAttributeData& OldCritChance
    );

    UFUNCTION()
    void OnRep_CritDamage(
        const FGameplayAttributeData& OldCritDamage
    );

    UFUNCTION()
    void OnRep_AttackSpeed(
        const FGameplayAttributeData& OldAttackSpeed
    );

    UFUNCTION()
    void OnRep_CastSpeed(
        const FGameplayAttributeData& OldCastSpeed
    );

    UFUNCTION()
    void OnRep_AttackPower(
        const FGameplayAttributeData& OldAttackPower
    );

    UFUNCTION()
    void OnRep_SpellPower(
        const FGameplayAttributeData& OldSpellPower
    );

    UFUNCTION()
    void OnRep_WeaponDamage(
        const FGameplayAttributeData& OldWeaponDamage
    );

    UFUNCTION()
    void OnRep_PhysicalDamage(
        const FGameplayAttributeData& OldPhysicalDamage
    );

    UFUNCTION()
    void OnRep_MagicDamage(
        const FGameplayAttributeData& OldMagicDamage
    );

};
