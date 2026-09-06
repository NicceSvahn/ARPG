#pragma once

#include "CoreMinimal.h"
#include "GenericAttributeSet.h"
#include "IntellectAttributeSet.generated.h"

UCLASS()
class TESTGAME_API UIntellectAttributeSet : public UGenericAttributeSet
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadOnly, Category = "Attributes")FGameplayAttributeData Intellect;

	PLAY_ATTRIBUTE_ACCESSORS(UIntellectAttributeSet, Intellect);
};
