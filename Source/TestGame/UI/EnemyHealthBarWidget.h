#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "EnemyHealthBarWidget.generated.h"

class UProgressBar;
class UTextBlock;

UCLASS()
class TESTGAME_API UEnemyHealthBarWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "Health")
    void SetHealth(float CurrentHealth);

protected:
    UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "Health")
    TObjectPtr<UProgressBar> HealthBar;
};