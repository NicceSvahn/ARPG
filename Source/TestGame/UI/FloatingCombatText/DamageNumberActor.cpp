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
    if (!WidgetComponent)
    {
        return;
    }

    WidgetComponent->InitWidget();

    UFloatingDamageWidget* DamageWidget =
        Cast<UFloatingDamageWidget>(
            WidgetComponent->GetUserWidgetObject()
        );

    if (!DamageWidget)
    {
        return;
    }

    DamageWidget->SetDamageValue(
        DamageAmount
    );
}