#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameplayTagContainer.h"
#include "AbilitySlotWidget.generated.h"

class UImage;
class UTextBlock;
class UTestGameAbilitySystemComponent;
class UBorder;

UCLASS()
class TESTGAME_API UAbilitySlotWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    void InitializeSlot(
        UTestGameAbilitySystemComponent* InASC,
        const FGameplayTag& InInputTag,
        const FText& InInputLabel);

    void RefreshAbility();

protected:
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UImage> Icon;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UTextBlock> InputText;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UBorder> CooldownOverlay;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UTextBlock> CooldownText;

    virtual void NativeTick(
        const FGeometry& MyGeometry,
        float InDeltaTime
    ) override;

private:
    UPROPERTY()
    TObjectPtr<UTestGameAbilitySystemComponent>
        AbilitySystemComponent;

    FGameplayTag InputTag;

    void RefreshCooldown();
};