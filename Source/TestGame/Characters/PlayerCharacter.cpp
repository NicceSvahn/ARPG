#include "PlayerCharacter.h"

#include "../UI/HealthBarWidget.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "TestGame/AbilitySystem/Attributes/HealthAttributeSet.h"

APlayerCharacter::APlayerCharacter()
{
    PrimaryActorTick.bCanEverTick = false;

    bUseControllerRotationYaw = false;

    HealthWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("HealthWidget"));

    GetCharacterMovement()->bOrientRotationToMovement = true;
    GetCharacterMovement()->bUseControllerDesiredRotation = false;
    GetCharacterMovement()->RotationRate = FRotator(0.0f, 720.0f, 0.0f);
}

void APlayerCharacter::BeginPlay()
{
    Super::BeginPlay();

    HealthWidget->InitWidget();
    RefreshHealthBar(HealthAttributeSet->GetHealth());
}

void APlayerCharacter::RefreshHealthBar(float CurrentHealth)
{
    if (!AbilitySystemComponent)
    {
        return;
    }

    const float MaxHealth = 100.0f;

    UpdatePlayerHealthUI(CurrentHealth, MaxHealth);
}

void APlayerCharacter::HandleAttributeChanged(
    FGameplayAttribute Attribute,
    float Magnitude,
    float NewValue
)
{
    Super::HandleAttributeChanged(Attribute, Magnitude, NewValue);

    UE_LOG(
        LogTemp,
        Warning,
        TEXT("PLAYER HEALTH CHANGED -> %f"),
        NewValue
    );

    if (Attribute == UHealthAttributeSet::GetHealthAttribute())
    {
        RefreshHealthBar(NewValue);
        return;
    }

    return;
}
