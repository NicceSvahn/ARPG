#include "GenericCharacter.h"
#include "AbilitySystemComponent.h"
#include "AttributeComponent.h"
#include "HealthComponent.h"

AGenericCharacter::AGenericCharacter()
{
	PrimaryActorTick.bCanEverTick = false;

	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(
		TEXT("AbilitySystemComponent")
	);

	HealthComponent = CreateDefaultSubobject<UHealthComponent>(
		TEXT("HealthComponent")
	);

	AttributeComponent = CreateDefaultSubobject<UAttributeComponent>(
		TEXT("AttributeComponent")
	);
}

void AGenericCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->InitAbilityActorInfo(this, this);
	}
}

UAbilitySystemComponent* AGenericCharacter::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void AGenericCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AGenericCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}