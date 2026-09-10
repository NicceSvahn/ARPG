#pragma once

#include "CoreMinimal.h"
#include "GenericAttributeSet.h"
#include "ManaAttributeSet.generated.h"

UCLASS()
class TESTGAME_API UManaAttributeSet : public UGenericAttributeSet
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadOnly, Category = "Attributes")FGameplayAttributeData Mana;

	PLAY_ATTRIBUTE_ACCESSORS(UManaAttributeSet, Mana);
};

