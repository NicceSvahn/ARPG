#include "TestGamePlayerState.h"
#include "../AbilitySystem/TestGameAbilitySystemComponent.h"

#include "Net/UnrealNetwork.h"
#include "GameplayTagContainer.h"
#include "AbilitySystemInterface.h"
#include "../Characters/PlayerCharacter.h"
#include "../Characters/PlayerClassDefinitions.h"

ATestGamePlayerState::ATestGamePlayerState()
{
    bReplicates = true;
}

FString ATestGamePlayerState::GetDebugPlayerName() const
{
    const FString CurrentPlayerName = GetPlayerName();

    if (!CurrentPlayerName.IsEmpty())
    {
        return CurrentPlayerName;
    }

    return FString::Printf(
        TEXT("Player %d"),
        GetPlayerId()
    );
}

FString ATestGamePlayerState::GetDebugPlayerLabel() const
{
    return FString::Printf(
        TEXT("%s [ID=%d]"),
        *GetDebugPlayerName(),
        GetPlayerId()
    );
}

void ATestGamePlayerState::GetLifetimeReplicatedProps(
    TArray<FLifetimeProperty>& OutLifetimeProps
) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(
        ATestGamePlayerState,
        SelectedClass
    );
}

void ATestGamePlayerState::SetPlayerClass(EPlayerClass NewClass)
{
    if (!HasAuthority())
    {
        return;
    }

    if (SelectedClass == NewClass)
    {
        SyncPlayerClassTag();

        APlayerCharacter* PlayerCharacter =
            Cast<APlayerCharacter>(GetPawn());

        if (PlayerCharacter)
        {
            const UPlayerClassDefinitions* Definition =
                FindClassDefinition(SelectedClass);

            if (Definition)
            {
                PlayerCharacter->ApplyPlayerClassDefinition(
                    Definition
                );
            }
        }

        return;
    }

    APlayerCharacter* PlayerCharacter =
        Cast<APlayerCharacter>(GetPawn());

    //Remove abilities belonging to the OLD class.
    if (PlayerCharacter)
    {
        const UPlayerClassDefinitions* OldDefinition =
            FindClassDefinition(SelectedClass);

        if (OldDefinition)
        {
            PlayerCharacter->RemovePlayerClassDefinition(
                OldDefinition
            );
        }
    }

    //Change class.
    SelectedClass = NewClass;

    //Update Class.Warrior / Class.Mage / etc.
    SyncPlayerClassTag();

    //Grant abilities belonging to the NEW class.
    if (PlayerCharacter)
    {
        const UPlayerClassDefinitions* NewDefinition =
            FindClassDefinition(SelectedClass);

        if (NewDefinition)
        {
            PlayerCharacter->ApplyPlayerClassDefinition(
                NewDefinition
            );
        }
    }
}

void ATestGamePlayerState::SyncPlayerClassTag()
{
    if (!HasAuthority())
    {
        return;
    }

    ApplyPlayerClassTagToASC();
}

void ATestGamePlayerState::OnRep_SelectedClass()
{
    ApplyPlayerClassTagToASC();
}

void ATestGamePlayerState::ApplyPlayerClassTagToASC()
{
    APawn* PlayerPawn = GetPawn();

    if (!IsValid(PlayerPawn))
    {
        return;
    }

    IAbilitySystemInterface* AbilitySystemInterface =
        Cast<IAbilitySystemInterface>(PlayerPawn);

    if (!AbilitySystemInterface)
    {
        return;
    }

    UAbilitySystemComponent* ASC =
        AbilitySystemInterface->GetAbilitySystemComponent();

    if (!IsValid(ASC))
    {
        return;
    }

    const FGameplayTag WarriorTag =
        FGameplayTag::RequestGameplayTag(
            FName(TEXT("Class.Warrior"))
        );

    const FGameplayTag MageTag =
        FGameplayTag::RequestGameplayTag(
            FName(TEXT("Class.Mage"))
        );

    const FGameplayTag RangerTag =
        FGameplayTag::RequestGameplayTag(
            FName(TEXT("Class.Ranger"))
        );

    ASC->RemoveLooseGameplayTag(WarriorTag);
    ASC->RemoveLooseGameplayTag(MageTag);
    ASC->RemoveLooseGameplayTag(RangerTag);

    switch (SelectedClass)
    {
    case EPlayerClass::Warrior:
        ASC->AddLooseGameplayTag(WarriorTag);
        break;

    case EPlayerClass::Mage:
        ASC->AddLooseGameplayTag(MageTag);
        break;

    case EPlayerClass::Ranger:
        ASC->AddLooseGameplayTag(RangerTag);
        break;

    case EPlayerClass::None:
    default:
        break;
    }

    FGameplayTagContainer OwnedTags;
    ASC->GetOwnedGameplayTags(OwnedTags);
}

const UPlayerClassDefinitions*
ATestGamePlayerState::FindClassDefinition(
    EPlayerClass PlayerClass
) const
{
    for (const UPlayerClassDefinitions* Definition :
        ClassDefinitions)
    {
        if (!Definition)
        {
            continue;
        }

        if (Definition->PlayerClass == PlayerClass)
        {
            return Definition;
        }
    }

    return nullptr;
}