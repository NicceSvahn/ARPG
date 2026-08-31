// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"

// Sets default values
APlayerCharacter::APlayerCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

    bUseControllerRotationYaw = false;

    GetCharacterMovement()->bOrientRotationToMovement = true;
    GetCharacterMovement()->bUseControllerDesiredRotation = false;
    GetCharacterMovement()->RotationRate = FRotator(0.f, 720.f, 0.f);
}

// Called when the game starts or when spawned
void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

}

// Called every frame
void APlayerCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!bHasMovementTarget) return;

    FVector ToTarget = MovementTarget - GetActorLocation();
    ToTarget.Z = 0.f;

    if (ToTarget.SizeSquared() <= FMath::Square(AcceptanceRadius))
    {
        bHasMovementTarget = false;
        return;
    }

    AddMovementInput(ToTarget.GetSafeNormal());
}

// Called to bind functionality to input
void APlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

    UE_LOG(LogTemp, Warning, TEXT("HELP ME"));
}

void APlayerCharacter::MoveToLocation(FVector& NewTarget)
{
    MovementTarget = NewTarget;
    bHasMovementTarget = true;
}