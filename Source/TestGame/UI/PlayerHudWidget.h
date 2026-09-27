#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemWidget.h"
#include "PlayerHudWidget.generated.h"

class AGenericCharacter;
class UAbilitySlotWidget;
class UHealthBarWidget;
class UHorizontalBox;
class UResourceBarWidget;
class UTestGameAbilitySystemComponent;
class UBuffBarWidget;

/**
 * Root player HUD. It coordinates child widgets but does not own health or
 * resource attribute-update logic. Those widgets observe GAS themselves.
 */
UCLASS()
class TESTGAME_API UPlayerHudWidget : public UAbilitySystemWidget
{
    GENERATED_BODY()

public:
    // Compatibility wrapper for existing PlayerController/Blueprint calls.
    void InitializeHud(AGenericCharacter* InCharacter);

    void RefreshAbilitySlots();

protected:
    virtual void OnAbilitySystemReady() override;
    virtual void UnbindFromAbilitySystem() override;

    UPROPERTY(EditDefaultsOnly, Category = "Ability Bar")
    TSubclassOf<UAbilitySlotWidget> AbilitySlotWidgetClass;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UHorizontalBox> HP_AbilityBar;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    TObjectPtr<UHealthBarWidget> WBP_PlayerHealthBar;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UResourceBarWidget> WBP_PlayerResourceBar;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UBuffBarWidget> WBP_BuffBar;

private:
    void BuildAbilitySlots();

    TWeakObjectPtr<UTestGameAbilitySystemComponent> TestGameAbilitySystemComponent;

    UPROPERTY()
    TArray<TObjectPtr<UAbilitySlotWidget>> AbilitySlotWidgets;

    FDelegateHandle AbilityBarChangedHandle;
};
