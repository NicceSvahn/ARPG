#include "GenericCharacter.h"
#include "AttributeComponent.h"
#include "HealthComponent.h"

AGenericCharacter::AGenericCharacter()
{
	PrimaryActorTick.bCanEverTick = false;

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
}

void AGenericCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AGenericCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}