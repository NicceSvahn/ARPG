#include "DamageNumberActor.h"

#include "Components/WidgetComponent.h"
#include "FloatingDamageWidget.h"

ADamageNumberActor::ADamageNumberActor()
{
    PrimaryActorTick.bCanEverTick = false;

    WidgetComponent =
        CreateDefaultSubobject<UWidgetComponent>(
            TEXT("WidgetComponent")
        );

    RootComponent = WidgetComponent;

    WidgetComponent->SetWidgetSpace(
        EWidgetSpace::Screen
    );
}

void ADamageNumberActor::BeginPlay()
{
    Super::BeginPlay();

    SetLifeSpan(1.0f);
}

void ADamageNumberActor::InitializeCombatText(
    const FCombatTextData& Data)
{
    if (!WidgetComponent)
    {
        return;
    }

    WidgetComponent->InitWidget();

    UFloatingDamageWidget* CombatTextWidget =
        Cast<UFloatingDamageWidget>(
            WidgetComponent->GetUserWidgetObject()
        );

    if (!CombatTextWidget)
    {
        return;
    }

    CombatTextWidget->SetCombatText(Data);
}

void ADamageNumberActor::InitializeDamage(
    float DamageAmount,
    bool bCritical)
{
    FCombatTextData Data;

    Data.Amount = DamageAmount;
    Data.Type = bCritical
        ? ECombatTextType::Critical
        : ECombatTextType::Damage;

    InitializeCombatText(Data);
}

void ADamageNumberActor::InitializeHealing(
    float HealingAmount)
{
    FCombatTextData Data;

    Data.Amount = HealingAmount;
    Data.Type = ECombatTextType::Healing;

    InitializeCombatText(Data);
}