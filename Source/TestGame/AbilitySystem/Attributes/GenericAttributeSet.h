#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "GenericAttributeSet.generated.h"

#define PLAY_ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
		GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
		GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
		GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
		GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FGenericAttributeEvent, float, EffectMagnitude, float, NewValue);

UCLASS()
class TESTGAME_API UGenericAttributeSet : public UAttributeSet
{
	GENERATED_BODY()
};
