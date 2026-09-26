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

    const bool bIsHealthChange =
        Data.EvaluatedData.Attribute == GetHealthAttribute();

    const bool bIsIncomingDamage =
        Data.EvaluatedData.Attribute == GetIncomingDamageAttribute();

    if ((bIsHealthChange || bIsIncomingDamage) &&
        ASC &&
        ASC->HasMatchingGameplayTag(DeadTag))
    {
        return false;
    }

    return Super::PreGameplayEffectExecute(Data);
}

void UHealthAttributeSet::PostGameplayEffectExecute(
    const FGameplayEffectModCallbackData& Data)
{
    if (Data.EvaluatedData.Attribute == GetIncomingDamageAttribute())
    {
        const float Damage = FMath::Max(GetIncomingDamage(), 0.0f);
        SetIncomingDamage(0.0f);

        if (Damage > 0.0f)
        {
            const float OldHealth = GetHealth();
            const float NewHealth = FMath::Clamp(
                OldHealth - Damage,
                0.0f,
                GetMaxHealth()
            );

            SetHealth(NewHealth);

            // Your GenericCharacter listens to UGenericAttributeSet::OnAttributeChanged
            // rather than the ASC's native attribute delegate. Because Health is changed
            // here as a consequence of IncomingDamage, broadcast the Health change
            // explicitly so death, hit reactions, damage numbers, etc. keep working.
            OnAttributeChanged.Broadcast(
                GetHealthAttribute(),
                NewHealth - OldHealth,
                NewHealth
            );
        }
    }
    else if (Data.EvaluatedData.Attribute == GetHealthAttribute())
    {
        const float ClampedHealth = FMath::Clamp(
            GetHealth(),
            0.0f,
            GetMaxHealth()
        );

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