#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "HealthBarWidget.generated.h"

class UProgressBar;
class UTextBlock;

UCLASS()
class TESTGAME_API UHealthBarWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "Health")
    void SetHealth(float CurrentHealth, float MaxHealth);

protected:
    UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "Health")
    TObjectPtr<UProgressBar> HealthBar;
};