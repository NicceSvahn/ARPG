#pragma once

#include "CoreMinimal.h"
#include "GenericAttributeSet.h"
#include "HealthAttributeSet.generated.h"

UCLASS()
class TESTGAME_API UHealthAttributeSet : public UGenericAttributeSet
{
    GENERATED_BODY()

public:

    UPROPERTY(BlueprintReadOnly, Category = "Attributes")FGameplayAttributeData Health;

    PLAY_ATTRIBUTE_ACCESSORS(UHealthAttributeSet, Health);
};