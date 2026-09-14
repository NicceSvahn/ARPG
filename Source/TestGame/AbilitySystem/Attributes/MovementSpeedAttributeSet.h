#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "MovementSpeedAttributeSet.generated.h"

#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
    GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
    GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
    GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
    GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

UCLASS()
class TESTGAME_API UMovementSpeedAttributeSet : public UAttributeSet
{
    GENERATED_BODY()

public:
    UMovementSpeedAttributeSet();

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    FGameplayAttributeData MovementSpeed;

    ATTRIBUTE_ACCESSORS(
        UMovementSpeedAttributeSet,
        MovementSpeed
    )
};