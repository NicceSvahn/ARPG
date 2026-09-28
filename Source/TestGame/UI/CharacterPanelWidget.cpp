#include "CharacterPanelWidget.h"

#include "AbilitySystemComponent.h"
#include "Components/TextBlock.h"

#include "../AbilitySystem/Attributes/PrimaryAttributeSet.h"
#include "../AbilitySystem/Attributes/OffensiveAttributeSet.h"
#include "../AbilitySystem/Attributes/DefensiveAttributeSet.h"
#include "../AbilitySystem/Attributes/HealthAttributeSet.h"
#include "../AbilitySystem/Attributes/ResourceAttributeSet.h"

void UCharacterPanelWidget::OnAbilitySystemReady()
{
    UAbilitySystemComponent* ASC =
        GetObservedAbilitySystem();

    if (!ASC)
    {
        return;
    }

    // --------------------------------------------------
    // PRIMARY
    // --------------------------------------------------

    BindAttribute(
        UPrimaryAttributeSet::GetStrengthAttribute()
    );

    BindAttribute(
        UPrimaryAttributeSet::GetDexterityAttribute()
    );

    BindAttribute(
        UPrimaryAttributeSet::GetIntellectAttribute()
    );


    // --------------------------------------------------
    // OFFENSE
    // --------------------------------------------------

    BindAttribute(
        UOffensiveAttributeSet::GetWeaponDamageAttribute()
    );

    BindAttribute(
        UOffensiveAttributeSet::GetAttackPowerAttribute()
    );

    BindAttribute(
        UOffensiveAttributeSet::GetSpellPowerAttribute()
    );

    BindAttribute(
        UOffensiveAttributeSet::GetPhysicalDamageAttribute()
    );

    BindAttribute(
        UOffensiveAttributeSet::GetMagicDamageAttribute()
    );

    BindAttribute(
        UOffensiveAttributeSet::GetCritChanceAttribute()
    );

    BindAttribute(
        UOffensiveAttributeSet::GetCritDamageAttribute()
    );

    BindAttribute(
        UOffensiveAttributeSet::GetAttackSpeedAttribute()
    );

    BindAttribute(
        UOffensiveAttributeSet::GetCastSpeedAttribute()
    );


    // --------------------------------------------------
    // DEFENSE
    // --------------------------------------------------

    BindAttribute(
        UDefensiveAttributeSet::GetArmourAttribute()
    );

    BindAttribute(
        UDefensiveAttributeSet::GetMagicResistanceAttribute()
    );

    BindAttribute(
        UDefensiveAttributeSet::GetBlockChanceAttribute()
    );

    BindAttribute(
        UDefensiveAttributeSet::GetEvasionAttribute()
    );


    // --------------------------------------------------
    // LIFE
    // --------------------------------------------------

    BindAttribute(
        UHealthAttributeSet::GetHealthAttribute()
    );

    BindAttribute(
        UHealthAttributeSet::GetMaxHealthAttribute()
    );


    // --------------------------------------------------
    // RESOURCE
    // --------------------------------------------------

    BindAttribute(
        UResourceAttributeSet::GetResourceAttribute()
    );

    BindAttribute(
        UResourceAttributeSet::GetMaxResourceAttribute()
    );


    RefreshAllStats();
}

void UCharacterPanelWidget::BindAttribute(
    const FGameplayAttribute& Attribute)
{
    UAbilitySystemComponent* ASC =
        GetObservedAbilitySystem();

    if (!ASC)
    {
        return;
    }

    const FDelegateHandle Handle =
        ASC
        ->GetGameplayAttributeValueChangeDelegate(Attribute)
        .AddUObject(
            this,
            &UCharacterPanelWidget::HandleAttributeChanged
        );

    BoundAttributes.Add(Attribute);
    AttributeChangedHandles.Add(Handle);
}

void UCharacterPanelWidget::UnbindFromAbilitySystem()
{
    UAbilitySystemComponent* ASC =
        GetObservedAbilitySystem();

    if (ASC)
    {
        const int32 Count =
            FMath::Min(
                BoundAttributes.Num(),
                AttributeChangedHandles.Num()
            );

        for (int32 Index = 0; Index < Count; ++Index)
        {
            if (!AttributeChangedHandles[Index].IsValid())
            {
                continue;
            }

            ASC
                ->GetGameplayAttributeValueChangeDelegate(
                    BoundAttributes[Index]
                )
                .Remove(
                    AttributeChangedHandles[Index]
                );
        }
    }

    BoundAttributes.Reset();
    AttributeChangedHandles.Reset();
}

void UCharacterPanelWidget::HandleAttributeChanged(
    const FOnAttributeChangeData& Data)
{
    RefreshAllStats();
}

void UCharacterPanelWidget::RefreshAllStats()
{
    UAbilitySystemComponent* ASC =
        GetObservedAbilitySystem();

    if (!ASC)
    {
        return;
    }

    const auto GetValue =
        [ASC](const FGameplayAttribute& Attribute)
        {
            return ASC->GetNumericAttribute(Attribute);
        };


    // ==================================================
    // PRIMARY
    // ==================================================

    const float Strength =
        GetValue(
            UPrimaryAttributeSet::GetStrengthAttribute()
        );

    const float Dexterity =
        GetValue(
            UPrimaryAttributeSet::GetDexterityAttribute()
        );

    const float Intellect =
        GetValue(
            UPrimaryAttributeSet::GetIntellectAttribute()
        );


    // ==================================================
    // OFFENSE
    // ==================================================

    const float WeaponDamage =
        GetValue(
            UOffensiveAttributeSet::GetWeaponDamageAttribute()
        );

    const float AttackPower =
        GetValue(
            UOffensiveAttributeSet::GetAttackPowerAttribute()
        );

    const float SpellPower =
        GetValue(
            UOffensiveAttributeSet::GetSpellPowerAttribute()
        );

    const float PhysicalDamage =
        GetValue(
            UOffensiveAttributeSet::GetPhysicalDamageAttribute()
        );

    const float MagicDamage =
        GetValue(
            UOffensiveAttributeSet::GetMagicDamageAttribute()
        );

    const float CritChance =
        GetValue(
            UOffensiveAttributeSet::GetCritChanceAttribute()
        );

    const float CritDamage =
        GetValue(
            UOffensiveAttributeSet::GetCritDamageAttribute()
        );

    const float AttackSpeed =
        GetValue(
            UOffensiveAttributeSet::GetAttackSpeedAttribute()
        );

    const float CastSpeed =
        GetValue(
            UOffensiveAttributeSet::GetCastSpeedAttribute()
        );


    // ==================================================
    // DEFENSE
    // ==================================================

    const float Armour =
        GetValue(
            UDefensiveAttributeSet::GetArmourAttribute()
        );

    const float MagicResistance =
        GetValue(
            UDefensiveAttributeSet::GetMagicResistanceAttribute()
        );

    const float BlockChance =
        GetValue(
            UDefensiveAttributeSet::GetBlockChanceAttribute()
        );

    const float Evasion =
        GetValue(
            UDefensiveAttributeSet::GetEvasionAttribute()
        );


    // Same formula currently used by ExecCalc_Damage.
    constexpr float MitigationConstant = 400.0f;

    const float PhysicalMitigation =
        Armour > 0.0f
        ? Armour /
        (Armour + MitigationConstant)
        * 100.0f
        : 0.0f;

    const float MagicMitigation =
        MagicResistance > 0.0f
        ? MagicResistance /
        (MagicResistance + MitigationConstant)
        * 100.0f
        : 0.0f;


    // ==================================================
    // LIFE
    // ==================================================

    const float Health =
        GetValue(
            UHealthAttributeSet::GetHealthAttribute()
        );

    const float MaxHealth =
        GetValue(
            UHealthAttributeSet::GetMaxHealthAttribute()
        );


    // ==================================================
    // RESOURCE
    // ==================================================

    const float Resource =
        GetValue(
            UResourceAttributeSet::GetResourceAttribute()
        );

    const float MaxResource =
        GetValue(
            UResourceAttributeSet::GetMaxResourceAttribute()
        );


    // ==================================================
    // UPDATE UI
    // ==================================================

    if (StrengthText)
    {
        StrengthText->SetText(
            FText::AsNumber(
                FMath::RoundToInt(Strength)
            )
        );
    }

    if (DexterityText)
    {
        DexterityText->SetText(
            FText::AsNumber(
                FMath::RoundToInt(Dexterity)
            )
        );
    }

    if (IntellectText)
    {
        IntellectText->SetText(
            FText::AsNumber(
                FMath::RoundToInt(Intellect)
            )
        );
    }


    // --------------------------------------------------
    // OFFENSE
    // --------------------------------------------------

    if (WeaponDamageText)
    {
        WeaponDamageText->SetText(
            FText::AsNumber(
                FMath::RoundToInt(WeaponDamage)
            )
        );
    }

    if (AttackPowerText)
    {
        AttackPowerText->SetText(
            FText::AsNumber(
                FMath::RoundToInt(AttackPower)
            )
        );
    }

    if (SpellPowerText)
    {
        SpellPowerText->SetText(
            FText::AsNumber(
                FMath::RoundToInt(SpellPower)
            )
        );
    }

    if (PhysicalDamageText)
    {
        PhysicalDamageText->SetText(
            FText::FromString(
                FString::Printf(
                    TEXT("%.1f%%"),
                    PhysicalDamage
                )
            )
        );
    }

    if (MagicDamageText)
    {
        MagicDamageText->SetText(
            FText::FromString(
                FString::Printf(
                    TEXT("%.1f%%"),
                    MagicDamage
                )
            )
        );
    }

    if (CritChanceText)
    {
        CritChanceText->SetText(
            FText::FromString(
                FString::Printf(
                    TEXT("%.1f%%"),
                    CritChance
                )
            )
        );
    }

    if (CritDamageText)
    {
        CritDamageText->SetText(
            FText::FromString(
                FString::Printf(
                    TEXT("%.1f%%"),
                    CritDamage
                )
            )
        );
    }

    if (AttackSpeedText)
    {
        AttackSpeedText->SetText(
            FText::FromString(
                FString::Printf(
                    TEXT("%.1f"),
                    AttackSpeed
                )
            )
        );
    }

    if (CastSpeedText)
    {
        CastSpeedText->SetText(
            FText::FromString(
                FString::Printf(
                    TEXT("%.1f"),
                    CastSpeed
                )
            )
        );
    }


    // --------------------------------------------------
    // DEFENSE
    // --------------------------------------------------

    if (ArmourText)
    {
        ArmourText->SetText(
            FText::AsNumber(
                FMath::RoundToInt(Armour)
            )
        );
    }

    if (PhysicalMitigationText)
    {
        PhysicalMitigationText->SetText(
            FText::FromString(
                FString::Printf(
                    TEXT("%.1f%%"),
                    PhysicalMitigation
                )
            )
        );
    }

    if (MagicResistanceText)
    {
        MagicResistanceText->SetText(
            FText::AsNumber(
                FMath::RoundToInt(
                    MagicResistance
                )
            )
        );
    }

    if (MagicMitigationText)
    {
        MagicMitigationText->SetText(
            FText::FromString(
                FString::Printf(
                    TEXT("%.1f%%"),
                    MagicMitigation
                )
            )
        );
    }

    if (BlockChanceText)
    {
        BlockChanceText->SetText(
            FText::FromString(
                FString::Printf(
                    TEXT("%.1f%%"),
                    BlockChance
                )
            )
        );
    }

    if (EvasionText)
    {
        EvasionText->SetText(
            FText::FromString(
                FString::Printf(
                    TEXT("%.1f%%"),
                    Evasion
                )
            )
        );
    }


    // --------------------------------------------------
    // LIFE
    // --------------------------------------------------

    if (HealthText)
    {
        HealthText->SetText(
            FText::FromString(
                FString::Printf(
                    TEXT("%.0f / %.0f"),
                    Health,
                    MaxHealth
                )
            )
        );
    }


    // --------------------------------------------------
    // RESOURCE
    // --------------------------------------------------

    if (ResourceText)
    {
        ResourceText->SetText(
            FText::FromString(
                FString::Printf(
                    TEXT("%.0f / %.0f"),
                    Resource,
                    MaxResource
                )
            )
        );
    }
}
