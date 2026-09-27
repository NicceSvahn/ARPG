#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "BuffDisplayData.generated.h"

class UTexture2D;

USTRUCT(BlueprintType)
struct TESTGAME_API FBuffDisplayData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Buff")
    FGameplayTag BuffTag;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Buff")
    FText DisplayName;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Buff")
    TObjectPtr<UTexture2D> Icon = nullptr;
};
