#pragma once

#include "CoreMinimal.h"
#include "GenericAttributeSet.h"
#include "ArmorAttributeSet.generated.h"

UCLASS()
class TESTGAME_API UArmorAttributeSet : public UGenericAttributeSet
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadOnly, Category = "Attributes")FGameplayAttributeData Armor;

	PLAY_ATTRIBUTE_ACCESSORS(UArmorAttributeSet, Armor);
};
