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

void ADamageNumberActor::InitializeDamage(
    float DamageAmount)
{
    const FLinearColor DamageColor = FLinearColor::White;

    InitializeCombatText(DamageAmount, DamageColor);
}

void ADamageNumberActor::InitializeHealing(
    const float HealingAmount)
{
    const FLinearColor HealingColor(0.1f, 1.0f, 0.1f, 1.0f);

    InitializeCombatText(HealingAmount, HealingColor);
}

void ADamageNumberActor::InitializeCombatText(
    const float Amount,
    const FLinearColor& Color)
{
    if (!WidgetComponent)
    {
        return;
    }

    WidgetComponent->InitWidget();

    UUserWidget* UserWidget = WidgetComponent->GetUserWidgetObject();

    if (!UserWidget)
    {
        return;
    }

    UFloatingDamageWidget* CombatTextWidget =
        Cast<UFloatingDamageWidget>(UserWidget);

    if (!CombatTextWidget)
    {
        return;
    }

    CombatTextWidget->SetCombatTextValue(Amount, Color);
}