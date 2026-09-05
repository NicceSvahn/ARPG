#include "GenericAttributeSet.h"
#include "AbilitySystemComponent.h"
#include "GameplayEffectExtension.h"

void UGenericAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	const FGameplayAttribute Attribute = Data.EvaluatedData.Attribute;
	const float EffectMagnitude = Data.EvaluatedData.Magnitude;

	UAbilitySystemComponent* ASC = GetOwningAbilitySystemComponent();
	if (!ASC)
	{
		return;
	}

	bool bFound = false;
	const float NewValue = ASC->GetGameplayAttributeValue(Attribute, bFound);

	if (!bFound)
	{
		return;
	}

	OnAttributeChanged.Broadcast(Attribute, EffectMagnitude, NewValue);
}








