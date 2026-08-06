// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerCharacter.h"
#include "DamageCalculationLibrary.h"
#include "AttributeComponent.h"
#include "HealthComponent.h"
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

FDamageResult APlayerCharacter::DebugDealDamageTo(
	AGenericCharacter* Target,
	float BaseDamage
)
{
	FDamageResult EmptyResult;

	if (!IsValid(Target) || Target == this)
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("DebugDealDamageTo received an invalid target.")
		);

		return EmptyResult;
	}

	UAttributeComponent* SourceAttributes =
		GetAttributeComponent();

	UAttributeComponent* TargetAttributes =
		Target->GetAttributeComponent();

	UHealthComponent* TargetHealth =
		Target->GetHealthComponent();

	if (!IsValid(TargetHealth))
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("%s has no HealthComponent."),
			*GetNameSafe(Target)
		);

		return EmptyResult;
	}

	FDamageRequest Request;
	Request.BaseDamage = BaseDamage;
	Request.Element = EDamageElement::Physical;
	Request.SourceActor = this;
	Request.TargetActor = Target;
	Request.DamageCauser = this;
	Request.bCanCrit = true;

	const FDamageResult Result =
		UDamageCalculationLibrary::CalculateDamage(
			Request,
			SourceAttributes,
			TargetAttributes
		);

	const float AppliedDamage =
		TargetHealth->TakeDamage(Result.FinalDamage);

	UE_LOG(
		LogTemp,
		Log,
		TEXT(
			" --- "
			"%s damaged %s | "
			"Raw: %.2f | Before mitigation: %.2f | "
			"Mitigated: %.2f | Final: %.2f | "
			"Applied: %.2f | Critical: %s"
			" --- "
		),
		*GetNameSafe(this),
		*GetNameSafe(Target),
		Result.RawDamage,
		Result.DamageBeforeMitigation,
		Result.MitigatedDamage,
		Result.FinalDamage,
		AppliedDamage,
		Result.bWasCritical ? TEXT("true") : TEXT("false")
	);

	return Result;
}