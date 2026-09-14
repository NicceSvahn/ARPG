#include "HealthAttributeSet.h"
#include "../../Characters/GenericCharacter.h"
#include "../../AbilitySystem/TestGameAbilitySystemComponent.h"

bool UHealthAttributeSet::PreGameplayEffectExecute(
    FGameplayEffectModCallbackData& Data)
{
    UAbilitySystemComponent* ASC =
        GetOwningAbilitySystemComponent();

    const FGameplayTag DeadTag =
        FGameplayTag::RequestGameplayTag(
            FName(TEXT("State.Dead"))
        );

    if (Data.EvaluatedData.Attribute ==
        GetHealthAttribute() &&
        ASC &&
        ASC->HasMatchingGameplayTag(DeadTag))
    {
        return false;
    }

    return Super::PreGameplayEffectExecute(
        Data
    );
}

void UHealthAttributeSet::PostGameplayEffectExecute(
    const FGameplayEffectModCallbackData& Data)
{

    if (Data.EvaluatedData.Attribute == GetHealthAttribute())
    {
        const AGenericCharacter* Character =
            Cast<AGenericCharacter>(GetOwningActor());

        float MaximumHealth =
            TNumericLimits<float>::Max();

        if (Character)
        {
            MaximumHealth =
                Character->GetMaxHealth();
        }

        const float CurrentHealth =
            GetHealth();

        //This forces the value to be between 0 and max health
        const float ClampedHealth =
            FMath::Clamp(
                CurrentHealth,
                0.0f,
                MaximumHealth);


        SetHealth(ClampedHealth);
        
    }
    Super::PostGameplayEffectExecute(Data);
}


