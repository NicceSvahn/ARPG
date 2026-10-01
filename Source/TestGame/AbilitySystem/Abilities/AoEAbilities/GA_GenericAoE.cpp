#include "GA_GenericAoE.h"

#include "Engine/EngineTypes.h"
#include "Kismet/KismetSystemLibrary.h"
#include "TestGame/Characters/GenericCharacter.h"

TArray<AGenericCharacter*>
UGA_GenericAoE::FindCharactersInRadius(
    UObject* WorldContextObject,
    const FVector& Origin,
    const float Radius,
    AActor* ActorToIgnore,
    TSubclassOf<AGenericCharacter>
    RequiredCharacterClass)
{
    TArray<AGenericCharacter*> Characters;

    if (!WorldContextObject)
    {
        return Characters;
    }

    if (Radius <= 0.0f)
    {
        return Characters;
    }

    TArray<TEnumAsByte<EObjectTypeQuery>>ObjectTypes;

    const EObjectTypeQuery PawnObjectType =
        UEngineTypes::ConvertToObjectType(ECC_Pawn);

    ObjectTypes.Add(PawnObjectType);

    TArray<AActor*> ActorsToIgnore;

    if (IsValid(ActorToIgnore))
    {
        ActorsToIgnore.Add(ActorToIgnore);
    }

    TSubclassOf<AActor> ClassFilter = AGenericCharacter::StaticClass();

    if (RequiredCharacterClass)
    {
        ClassFilter = RequiredCharacterClass;
    }

    TArray<AActor*> OverlappingActors;

    const bool bFoundActors =
        UKismetSystemLibrary::
        SphereOverlapActors(
            WorldContextObject,
            Origin,
            Radius,
            ObjectTypes,
            ClassFilter,
            ActorsToIgnore,
            OverlappingActors
        );

    if (!bFoundActors)
    {
        return Characters;
    }

    TSet<AGenericCharacter*>UniqueCharacters;

    for (AActor* OverlappingActor : OverlappingActors)
    {
        AGenericCharacter* Character = 
            Cast<AGenericCharacter>(OverlappingActor);

        if (!IsValid(Character))
        {
            continue;
        }

        if (Character == ActorToIgnore)
        {
            continue;
        }

        if (!Character->GetAbilitySystemComponent())
        {
            continue;
        }

        UniqueCharacters.Add(Character);
    }

    Characters.Reserve(UniqueCharacters.Num());

    for (AGenericCharacter* Character : UniqueCharacters)
    {
        Characters.Add(Character);
    }

    return Characters;
}

TArray<AGenericCharacter*>
UGA_GenericAoE::FindCharactersInArc(
    UObject* WorldContextObject,
    const FVector& Origin,
    const FVector& ForwardDirection,
    const float Radius,
    const float ArcDegrees,
    AActor* ActorToIgnore,
    TSubclassOf<AGenericCharacter> RequiredCharacterClass)
{
    TArray<AGenericCharacter*> CharactersInArc;

    const TArray<AGenericCharacter*> CharactersInRadius =
        FindCharactersInRadius(
            WorldContextObject,
            Origin,
            Radius,
            ActorToIgnore,
            RequiredCharacterClass
        );

    FVector Forward = ForwardDirection;
    Forward.Z = 0.0f;

    if (!Forward.Normalize())
    {
        return CharactersInArc;
    }

    const float ClampedArc =
        FMath::Clamp(ArcDegrees, 0.0f, 360.0f);

    if (ClampedArc >= 360.0f)
    {
        return CharactersInRadius;
    }

    const float HalfArcRadians =
        FMath::DegreesToRadians(
            ClampedArc * 0.5f
        );

    const float MinimumDot =
        FMath::Cos(HalfArcRadians);

    for (AGenericCharacter* Character : CharactersInRadius)
    {
        if (!IsValid(Character))
        {
            continue;
        }

        FVector DirectionToTarget =
            Character->GetActorLocation() - Origin;

        DirectionToTarget.Z = 0.0f;

        if (!DirectionToTarget.Normalize())
        {
            continue;
        }

        const float Dot =
            FVector::DotProduct(
                Forward,
                DirectionToTarget
            );

        if (Dot >= MinimumDot)
        {
            CharactersInArc.Add(Character);
        }
    }

    return CharactersInArc;
}