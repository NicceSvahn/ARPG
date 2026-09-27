#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "BuffDisplayData.h"
#include "BuffDisplayDataAsset.generated.h"

UCLASS(BlueprintType)
class TESTGAME_API UBuffDisplayDataAsset : public UDataAsset
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Buffs")
    TArray<FBuffDisplayData> Buffs;

    const FBuffDisplayData* FindBuff(const FGameplayTag& BuffTag) const;
};
