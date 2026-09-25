#include "PlayerCharacter.h"

#include "../UI/HealthBarWidget.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "TestGame/AbilitySystem/Attributes/HealthAttributeSet.h"
#include "../Network/NetworkDebug.h"
#include "../Game/TestGamePlayerState.h"
#include "../Game/TestGameGameState.h"
#include "PlayerClassDefinitions.h"

APlayerCharacter::APlayerCharacter()
{
    PrimaryActorTick.bCanEverTick = false;

    bUseControllerRotationYaw = false;

    GetCharacterMovement()->bOrientRotationToMovement = true;
    GetCharacterMovement()->bUseControllerDesiredRotation = false;
    GetCharacterMovement()->RotationRate = FRotator(0.0f, 720.0f, 0.0f);
}

void APlayerCharacter::BeginPlay()
{
    Super::BeginPlay();

    NetworkDebug::LogActor(
        this,
        TEXT("PlayerCharacter BeginPlay")
    );

    LogPlayerIdentity();
}

void APlayerCharacter::HandleAttributeChanged(
    FGameplayAttribute Attribute,
    float Magnitude,
    float NewValue
)
{
    Super::HandleAttributeChanged(Attribute, Magnitude, NewValue);

    if (Attribute == UHealthAttributeSet::GetHealthAttribute())
    {
        return;
    }

    return;
}

void APlayerCharacter::OnRep_PlayerState()
{
    Super::OnRep_PlayerState();

    LogPlayerIdentity();
}

void APlayerCharacter::LogPlayerIdentity() const
{
    const ATestGamePlayerState* PS =
        GetPlayerState<ATestGamePlayerState>();

    if (!PS)
    {
        return;
    }
}

void APlayerCharacter::OnDeathStarted()
{
    Super::OnDeathStarted();

    if (!HasAuthority())
    {
        return;
    }

    ATestGameGameState* GameState =
        GetWorld()
        ? GetWorld()->GetGameState<ATestGameGameState>()
        : nullptr;

    if (!GameState)
    {
        return;
    }

    GameState->SetLevelState(
        ELevelState::Failed
    );
}

void APlayerCharacter::ApplyPlayerClassDefinition(
    const UPlayerClassDefinitions* ClassDefinition
)
{
    if (!HasAuthority())
    {
        return;
    }

    if (!ClassDefinition)
    {
        return;
    }

    GrantAbilities(
        ClassDefinition->Abilities
    );
}

void APlayerCharacter::RemovePlayerClassDefinition(
    const UPlayerClassDefinitions* ClassDefinition
)
{
    if (!HasAuthority())
    {
        return;
    }

    if (!ClassDefinition)
    {
        return;
    }

    RemoveAbilities(
        ClassDefinition->Abilities
    );
}