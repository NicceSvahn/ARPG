#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CombatTextData.h"
#include "FloatingDamageWidget.generated.h"

class UTextBlock;
class UWidgetAnimation;

UCLASS()
class TESTGAME_API UFloatingDamageWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    void SetCombatText(const FCombatTextData& Data);

protected:
    virtual void NativeConstruct() override;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UTextBlock> DamageText;

    UPROPERTY(Transient, meta = (BindWidgetAnim))
    TObjectPtr<UWidgetAnimation> FloatingDamageAnim;

private:
    FText BuildDisplayText(const FCombatTextData& Data) const;
    FLinearColor GetCombatTextColor(ECombatTextType Type) const;
};