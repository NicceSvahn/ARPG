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

void UHealthAttributeSet::OnRep_Health(
    const FGameplayAttributeData& OldHealth)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(
        UHealthAttributeSet,
        Health,
        OldHealth
    );
}

void UHealthAttributeSet::OnRep_MaxHealth(
    const FGameplayAttributeData& OldMaxHealth)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(
        UHealthAttributeSet,
        MaxHealth,
        OldMaxHealth
    );
}

void UHealthAttributeSet::OnRep_LifeRegeneration(
    const FGameplayAttributeData& OldLifeRegeneration)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(
        UHealthAttributeSet,
        LifeRegeneration,
        OldLifeRegeneration
    );
}

void UHealthAttributeSet::GetLifetimeReplicatedProps(
    TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(
        OutLifetimeProps
    );

    DOREPLIFETIME_CONDITION_NOTIFY(
        UHealthAttributeSet,
        Health,
        COND_None,
        REPNOTIFY_Always
    );

    DOREPLIFETIME_CONDITION_NOTIFY(
        UHealthAttributeSet,
        MaxHealth,
        COND_None,
        REPNOTIFY_Always
    );

    DOREPLIFETIME_CONDITION_NOTIFY(
        UHealthAttributeSet,
        LifeRegeneration,
        COND_None,
        REPNOTIFY_Always
    );
}