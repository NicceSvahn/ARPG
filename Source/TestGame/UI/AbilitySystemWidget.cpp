#include "AbilitySystemWidget.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"

void UAbilitySystemWidget::InitializeFromActor(AActor* InActor)
{
    // Remove delegates from a previously observed ASC first.
    UnbindFromAbilitySystem();

    ObservedActor.Reset();
    ObservedAbilitySystem.Reset();

    if (!InActor)
    {
        return;
    }

    IAbilitySystemInterface* AbilitySystemInterface =
        Cast<IAbilitySystemInterface>(InActor);

    if (!AbilitySystemInterface)
    {
        return;
    }

    UAbilitySystemComponent* ASC =
        AbilitySystemInterface->GetAbilitySystemComponent();

    if (!ASC)
    {
        return;
    }

    ObservedActor = InActor;
    ObservedAbilitySystem = ASC;

    OnAbilitySystemReady();
}

void UAbilitySystemWidget::NativeDestruct()
{
    UnbindFromAbilitySystem();

    ObservedAbilitySystem.Reset();
    ObservedActor.Reset();

    Super::NativeDestruct();
}

void UAbilitySystemWidget::OnAbilitySystemReady()
{
    // Base implementation intentionally empty.
}

void UAbilitySystemWidget::UnbindFromAbilitySystem()
{
    // Base implementation intentionally empty.
}

UAbilitySystemComponent*
UAbilitySystemWidget::GetObservedAbilitySystem() const
{
    return ObservedAbilitySystem.Get();
}

AActor* UAbilitySystemWidget::GetObservedActor() const
{
    return ObservedActor.Get();
}