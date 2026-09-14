#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ResourceBarWidget.generated.h"

class UProgressBar;

UCLASS()
class TESTGAME_API UResourceBarWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "HUD|Resource")
    void SetResource(
        float CurrentResource,
        float MaxResource
    );

protected:
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UProgressBar> ResourceBar;
};