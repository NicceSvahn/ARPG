#pragma once

#include "CoreMinimal.h"
#include "GenericAttributeSet.h"
#include "StrengthAttributeSet.generated.h"

UCLASS()
class TESTGAME_API UStrengthAttributeSet : public UGenericAttributeSet
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadOnly, Category = "Attributes")FGameplayAttributeData Strength;

	PLAY_ATTRIBUTE_ACCESSORS(UStrengthAttributeSet, Strength);
};
