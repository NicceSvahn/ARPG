#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "FloatingDamageWidget.generated.h"

class UTextBlock;
class UWidgetAnimation;

UCLASS()
class TESTGAME_API UFloatingDamageWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    void SetDamageValue(float DamageAmount);

protected:
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UTextBlock> DamageText;

    virtual void NativeConstruct() override;

    UPROPERTY(Transient, meta = (BindWidgetAnim))
    TObjectPtr<UWidgetAnimation> FloatingDamageAnim;
};