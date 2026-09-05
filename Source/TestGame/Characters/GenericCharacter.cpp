#include "GenericCharacter.h"
#include "AbilitySystemComponent.h"
#include "Abilities/GameplayAbility.h"


AGenericCharacter::AGenericCharacter()
{
	PrimaryActorTick.bCanEverTick = false;

	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(
		TEXT("AbilitySystemComponent")
	);

	HealthAttributeSet = CreateDefaultSubobject<UHealthAttributeSet>(TEXT("HealthAttributeSet"));
}

void AGenericCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (AbilitySystemComponent)
	{

		AbilitySystemComponent->InitAbilityActorInfo(this, this);

		GrantStartupAbilities();
	}

	if (HealthAttributeSet)
	{

		HealthAttributeSet->InitHealth(InitialHealth);
		UE_LOG(LogTemp, Warning, TEXT("BeginPlay Health=%f"), HealthAttributeSet->GetHealth());

		HealthAttributeSet->OnAttributeChanged.AddDynamic(this, &AGenericCharacter::HandleAttributeChanged);
	}
}

void AGenericCharacter::HandleAttributeChanged(FGameplayAttribute Attribute, float Magnitude, float NewValue)
{
	UE_LOG(LogTemp, Warning, TEXT("Attribute=%s,NewValue=%f"),*Attribute.GetName(), NewValue);

	if (Attribute != UHealthAttributeSet::GetHealthAttribute())
	{
		return;
	}

	if (NewValue <= 0)
	{
		UE_LOG(LogTemp, Error, TEXT("Actor name=%s, DEAD! NewHealth=%f"), *GetName(), NewValue);
		Destroy();
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

void AGenericCharacter::GrantStartupAbilities()
{
	if (!AbilitySystemComponent)
	{
		return;
	}

	int32 inLevel = 1;

	for (const TSubclassOf<UGameplayAbility>& AbilityClass : StartupAbilities)
	{
		if (!AbilityClass)
		{
			continue;
		}

		AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(AbilityClass, inLevel, INDEX_NONE, this));
	}
}