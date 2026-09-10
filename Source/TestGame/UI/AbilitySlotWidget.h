#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameplayTagContainer.h"
#include "AbilitySlotWidget.generated.h"

class UImage;
class UTextBlock;
class UTestGameAbilitySystemComponent;

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

private:
    UPROPERTY()
    TObjectPtr<UTestGameAbilitySystemComponent>
        AbilitySystemComponent;

    FGameplayTag InputTag;
};