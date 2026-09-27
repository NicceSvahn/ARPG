#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "BuffIconWidget.generated.h"

class UImage;
class UTextBlock;
class UTexture2D;

UCLASS()
class TESTGAME_API UBuffIconWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    void InitializeBuff(const FText& BuffName, UTexture2D* Icon);
    void SetRemainingSeconds(float RemainingTime);

protected:
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UImage> BuffIcon;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UTextBlock> DurationText;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UTextBlock> BuffNameText;
};
