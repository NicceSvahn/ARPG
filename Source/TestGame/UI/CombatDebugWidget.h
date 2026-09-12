#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CombatDebugWidget.generated.h"

class UTextBlock;
class AGenericCharacter;
class UTestGameAbilitySystemComponent;
class UCombatDebugWidget;
class UInputAction;

UCLASS()
class TESTGAME_API UCombatDebugWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    void InitializeDebugWidget(AGenericCharacter* InPlayerCharacter);

protected:
    virtual void NativeTick(
        const FGeometry& MyGeometry,
        float InDeltaTime
    ) override;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UTextBlock> PlayerHealthText;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UTextBlock> PlayerResourceText;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UTextBlock> TargetActorText;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UTextBlock> TargetHealthText;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UTextBlock> TargetDistanceText;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UTextBlock> GameplayTagsText;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UTextBlock> AbilitiesText;

private:
    void RefreshDebugInfo();

    UPROPERTY()
    TObjectPtr<AGenericCharacter> PlayerCharacter;

    UPROPERTY()
    TObjectPtr<UTestGameAbilitySystemComponent> AbilitySystemComponent;
};