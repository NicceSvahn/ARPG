#pragma once

#include "CoreMinimal.h"
#include "GenericAttributeSet.h"
#include "DexterityAttributeSet.generated.h"

UCLASS()
class TESTGAME_API UDexterityAttributeSet : public UGenericAttributeSet
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadOnly, Category = "Attributes")FGameplayAttributeData Dexterity;

	PLAY_ATTRIBUTE_ACCESSORS(UDexterityAttributeSet, Dexterity);
};
