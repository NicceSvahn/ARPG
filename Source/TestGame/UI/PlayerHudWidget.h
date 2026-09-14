#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "HealthBarWidget.h"
#include "../AbilitySystem/TestGameAbilitySystemComponent.h"
#include "PlayerHudWidget.generated.h"

class UHorizontalBox;
class UAbilitySlotWidget;
class UTestGameAbilitySystemComponent;
class AGenericCharacter;
class UProgressBar;
class UResourceAttributeSet;
class UResourceBarWidget;

UCLASS()
class TESTGAME_API UPlayerHudWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "HUD|Health")
	void SetHealth(float CurrentHealth, float MaxHealth);

	void InitializeHud(AGenericCharacter* InCharacter);

	void RefreshAbilitySlots();

protected:
	virtual void NativeConstruct() override;

	virtual void NativeDestruct() override;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UHealthBarWidget> WBP_PlayerHealthBar;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UHorizontalBox> HP_AbilityBar;

	UPROPERTY(EditDefaultsOnly, Category = "Ability Bar")
	TSubclassOf<UAbilitySlotWidget>
		AbilitySlotWidgetClass;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UResourceBarWidget> WBP_PlayerResourceBar;

	void HandleResourceChanged(
		const FOnAttributeChangeData& Data
	);

	void HandleMaxResourceChanged(
		const FOnAttributeChangeData& Data
	);

private:
	void BuildAbilitySlots();

	void RefreshResourceBar();

	UPROPERTY()
	TObjectPtr<UTestGameAbilitySystemComponent>
		AbilitySystemComponent;

	UPROPERTY()
	TArray<TObjectPtr<UAbilitySlotWidget>>
		AbilitySlotWidgets;

	FDelegateHandle AbilityBarChangedHandle;

	UPROPERTY()
	TObjectPtr<AGenericCharacter> PlayerCharacter;

	FDelegateHandle HealthChangedHandle;

	FDelegateHandle ResourceChangedHandle;
	FDelegateHandle MaxResourceChangedHandle;
};
