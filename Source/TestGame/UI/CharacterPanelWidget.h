#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemWidget.h"
#include "AttributeSet.h"
#include "CharacterPanelWidget.generated.h"

class UTextBlock;
struct FOnAttributeChangeData;
class UInventoryComponent;
class UItemSlotWidget;
class UUniformGridPanel;

UCLASS()
class TESTGAME_API UCharacterPanelWidget : public UAbilitySystemWidget
{
    GENERATED_BODY()

protected:
    virtual void OnAbilitySystemReady() override;
    virtual void UnbindFromAbilitySystem() override;

    virtual void NativeConstruct() override;
private:
    void RefreshAllStats();

    void HandleAttributeChanged(
        const FOnAttributeChangeData& Data
    );

    void BindAttribute(
        const FGameplayAttribute& Attribute
    );
    

    //Inventory
    void RefreshInventory();

    void HandleItemSlotPressed(
        int32 Index,
        FGuid ExpectedItemId
    );

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UUniformGridPanel> InventoryGrid;

    UPROPERTY(EditDefaultsOnly, Category = "Inventory")
    TSubclassOf<UItemSlotWidget> InventorySlotWidgetClass;

    UPROPERTY(
        EditDefaultsOnly,
        Category = "Inventory",
        meta = (ClampMin = "1")
    )
    int32 InventoryColumns = 12;

    TWeakObjectPtr<UInventoryComponent> ObservedInventory;

    // --------------------------------------------------
    // PRIMARY
    // --------------------------------------------------

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UTextBlock> StrengthText;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UTextBlock> DexterityText;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UTextBlock> IntellectText;


    // --------------------------------------------------
    // OFFENSE
    // --------------------------------------------------

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UTextBlock> WeaponDamageText;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UTextBlock> AttackPowerText;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UTextBlock> SpellPowerText;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UTextBlock> PhysicalDamageText;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UTextBlock> MagicDamageText;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UTextBlock> CritChanceText;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UTextBlock> CritDamageText;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UTextBlock> AttackSpeedText;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UTextBlock> CastSpeedText;


    // --------------------------------------------------
    // DEFENSE
    // --------------------------------------------------

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UTextBlock> ArmourText;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UTextBlock> PhysicalMitigationText;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UTextBlock> MagicResistanceText;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UTextBlock> MagicMitigationText;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UTextBlock> BlockChanceText;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UTextBlock> EvasionText;


    // --------------------------------------------------
    // LIFE
    // --------------------------------------------------

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UTextBlock> HealthText;


    // --------------------------------------------------
    // RESOURCE
    // --------------------------------------------------

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UTextBlock> ResourceText;

    TArray<FDelegateHandle> AttributeChangedHandles;
    TArray<FGameplayAttribute> BoundAttributes;
};