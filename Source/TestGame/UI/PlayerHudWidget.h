#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "HealthBarWidget.h"
#include "PlayerHudWidget.generated.h"

class UHorizontalBox;
class UAbilitySlotWidget;
class UTestGameAbilitySystemComponent;
class AGenericCharacter;

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

private:
	void BuildAbilitySlots();

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
};
