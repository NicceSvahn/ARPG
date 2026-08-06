// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerCharacter.h"
#include "DamageCalculationLibrary.h"
#include "AttributeComponent.h"
#include "HealthComponent.h"
#include "Engine/World.h"
#include "GameFramework/CharacterMovementComponent.h"

APlayerCharacter::APlayerCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	bUseControllerRotationYaw = false;

	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->bUseControllerDesiredRotation = false;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 720.f, 0.f);

	BasicAttackSpec.AbilityType =
		EAbilityType::BasicAttack;

	BasicAttackSpec.BaseDamage = 30.0f;
	BasicAttackSpec.Range = 200.0f;
	BasicAttackSpec.Cooldown = 0.8f;
	BasicAttackSpec.DamageElement =
		EDamageElement::Physical;
	BasicAttackSpec.bCanCrit = true;

	BashSpec.AbilityType =
		EAbilityType::Bash;

	BashSpec.BaseDamage = 50.0f;
	BashSpec.Range = 180.0f;
	BashSpec.Cooldown = 2.0f;
	BashSpec.DamageElement =
		EDamageElement::Physical;
	BashSpec.bCanCrit = true;
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

bool APlayerCharacter::IsPendingAbilityInRange() const
{
	if (!IsValid(PendingAbilityTarget))
	{
		return false;
	}

	const FAbilitySpec* AbilitySpec =
		GetAbilitySpec(PendingAbilityType);

	if (!AbilitySpec)
	{
		return false;
	}

	const float DistanceToTarget = FVector::Dist(
		GetActorLocation(),
		PendingAbilityTarget->GetActorLocation()
	);

	return DistanceToTarget <= AbilitySpec->Range;
}

bool APlayerCharacter::RequestTargetedAbility(
	EAbilityType AbilityType,
	AGenericCharacter* Target
)
{
	if (!IsValid(Target) || Target == this)
	{
		return false;
	}

	const FAbilitySpec* AbilitySpec =
		GetAbilitySpec(AbilityType);

	if (!AbilitySpec)
	{
		return false;
	}

	PendingAbilityType = AbilityType;
	PendingAbilityTarget = Target;

	return true;
}

FDamageResult APlayerCharacter::TryExecutePendingAbility()
{
	FDamageResult EmptyResult;

	if (!IsValid(PendingAbilityTarget))
	{
		return EmptyResult;
	}

	const FAbilitySpec* AbilitySpec =
		GetAbilitySpec(PendingAbilityType);

	if (!AbilitySpec)
	{
		return EmptyResult;
	}

	if (!IsPendingAbilityInRange())
	{
		return EmptyResult;
	}

	if (IsAbilityOnCooldown(*AbilitySpec))
	{
		return EmptyResult;
	}

	FaceTarget(PendingAbilityTarget);

	const FDamageResult Result =
		ExecuteDamageAbility(
			PendingAbilityTarget,
			*AbilitySpec
		);

	StartAbilityCooldown(PendingAbilityType);

	return Result;
}

FDamageResult APlayerCharacter::ExecuteDamageAbility(
	AGenericCharacter* Target,
	const FAbilitySpec& AbilitySpec
)
{
	FDamageResult EmptyResult;

	if (!IsValid(Target))
	{
		return EmptyResult;
	}

	UHealthComponent* TargetHealth =
		Target->GetHealthComponent();

	if (!IsValid(TargetHealth))
	{
		return EmptyResult;
	}

	FDamageRequest Request;
	Request.BaseDamage = AbilitySpec.BaseDamage;
	Request.Element = AbilitySpec.DamageElement;
	Request.SourceActor = this;
	Request.TargetActor = Target;
	Request.DamageCauser = this;
	Request.bCanCrit = AbilitySpec.bCanCrit;

	const FDamageResult Result =
		UDamageCalculationLibrary::CalculateDamage(
			Request,
			GetAttributeComponent(),
			Target->GetAttributeComponent()
		);

	TargetHealth->TakeDamage(Result.FinalDamage);

	return Result;
}

const FAbilitySpec* APlayerCharacter::GetAbilitySpec(
	EAbilityType AbilityType
) const
{
	switch (AbilityType)
	{
	case EAbilityType::BasicAttack:
		return &BasicAttackSpec;

	case EAbilityType::Bash:
		return &BashSpec;

	default:
		return nullptr;
	}
}

bool APlayerCharacter::IsAbilityOnCooldown(
	const FAbilitySpec& AbilitySpec
) const
{
	const UWorld* World = GetWorld();

	if (!World)
	{
		return true;
	}

	const float* LastUseTime =
		LastAbilityUseTimes.Find(
			AbilitySpec.AbilityType
		);

	if (!LastUseTime)
	{
		return false;
	}

	const float TimeSinceLastUse =
		World->GetTimeSeconds() - *LastUseTime;

	return TimeSinceLastUse < AbilitySpec.Cooldown;
}

void APlayerCharacter::StartAbilityCooldown(
	EAbilityType AbilityType
)
{
	if (!GetWorld())
	{
		return;
	}

	LastAbilityUseTimes.Add(
		AbilityType,
		GetWorld()->GetTimeSeconds()
	);
}

void APlayerCharacter::ClearPendingAbility()
{
	PendingAbilityTarget = nullptr;
	PendingAbilityType =
		EAbilityType::BasicAttack;
}

void APlayerCharacter::FaceTarget(
	const AActor* Target
)
{
	if (!IsValid(Target))
	{
		return;
	}

	FVector Direction =
		Target->GetActorLocation() -
		GetActorLocation();

	Direction.Z = 0.0f;

	if (Direction.IsNearlyZero())
	{
		return;
	}

	const FRotator TargetRotation =
		Direction.Rotation();

	SetActorRotation(
		FRotator(
			0.0f,
			TargetRotation.Yaw,
			0.0f
		)
	);
}