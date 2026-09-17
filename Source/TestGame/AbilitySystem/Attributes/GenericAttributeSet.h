#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "../TestGameAbilitySystemComponent.h"
#include "GameplayEffectTypes.h"
#include "GameplayEffectExtension.h"
#include "GenericAttributeSet.generated.h"

#define PLAY_ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
		GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
		GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
		GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
		GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

#define PLAY_ATTRIBUTE_REPNOTIFY(ClassName, PropertyName) \
    GAMEPLAYATTRIBUTE_REPNOTIFY( \
        ClassName, \
        PropertyName, \
        Old##PropertyName \
    )

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(
	FGenericAttributeEvent,
	FGameplayAttribute, Attribute,
	float, EffectMagnitude,
	float, NewValue
);

UCLASS()
class TESTGAME_API UGenericAttributeSet : public UAttributeSet
{
	GENERATED_BODY()

public:

	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;

	UPROPERTY(BlueprintAssignable, Category = "Attributes")
	FGenericAttributeEvent OnAttributeChanged;
};
