#include "GenericCharacter.h"
#include "Abilities/GameplayAbility.h"


AGenericCharacter::AGenericCharacter()
{
	PrimaryActorTick.bCanEverTick = false;

	AbilitySystemComponent = CreateDefaultSubobject<UTestGameAbilitySystemComponent>(TEXT("AbilitySystemComponent"));

	HealthAttributeSet = CreateDefaultSubobject<UHealthAttributeSet>(TEXT("HealthAttributeSet"));
}

void AGenericCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (AbilitySystemComponent)
	{

		AbilitySystemComponent->InitAbilityActorInfo(this, this);

		UE_LOG(
			LogTemp,
			Warning,
			TEXT("ASC AFTER INIT: Character=%s Owner=%s Avatar=%s"),
			*GetNameSafe(this),
			*GetNameSafe(AbilitySystemComponent->GetOwnerActor()),
			*GetNameSafe(AbilitySystemComponent->GetAvatarActor())
		);

		UE_LOG(
			LogTemp,
			Warning,
			TEXT("STARTUP COUNT=%d AUTHORITY=%s"),
			StartupAbilities.Num(),
			HasAuthority() ? TEXT("TRUE") : TEXT("FALSE")
		);

		if (HasAuthority())
		{
			GrantStartupAbilities();
		}
	}

	if (HealthAttributeSet)
	{
		HealthAttributeSet->OnAttributeChanged.AddDynamic(this, &AGenericCharacter::HandleAttributeChanged);

		HealthAttributeSet->InitHealth(GetMaxHealth());
		PreviousHealth = HealthAttributeSet->GetHealth();

		OnHealthChanged.Broadcast(GetCurrentHealth(), GetMaxHealth());

		UE_LOG(LogTemp, Warning, TEXT("BeginPlay Health=%f"), HealthAttributeSet->GetHealth());
	}
}

float AGenericCharacter::GetCurrentHealth() const
{
	return HealthAttributeSet ? HealthAttributeSet->GetHealth() : 0.0f;
}

float AGenericCharacter::GetMaxHealth() const
{
	return InitialHealth;
}

void AGenericCharacter::HandleAttributeChanged(FGameplayAttribute Attribute, float Magnitude, float NewValue)
{
	UE_LOG(LogTemp, Warning, TEXT("Attribute=%s,NewValue=%f"),*Attribute.GetName(), NewValue);

	const float OldHealth = PreviousHealth;

	PreviousHealth = NewValue;

	OnHealthChanged.Broadcast(NewValue, GetMaxHealth());

	if (Attribute == UHealthAttributeSet::GetHealthAttribute())
	{
		if (NewValue <= 0)
		{
			UE_LOG(LogTemp, Error, TEXT("Actor name=%s, DEAD! NewHealth=%f"), *GetName(), NewValue);
			Destroy();
		}

		if (NewValue < OldHealth)
		{
			const float DamageAmount = OldHealth - NewValue;

			OnDamageReceived.Broadcast(DamageAmount);

			UE_LOG(
				LogTemp,
				Warning,
				TEXT("%s RECEIVED DAMAGE: %f"),
				*GetName(),
				DamageAmount
			);
		}

		return;
	}
	return;
}

UTestGameAbilitySystemComponent* AGenericCharacter::GetAbilitySystemComponent() const
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

	for (const FGrantedAbility& StartupAbility : StartupAbilities)
	{
		if (!StartupAbility.AbilityClass)
		{
			continue;
		}

		FGameplayAbilitySpec AbilitySpec(
			StartupAbility.AbilityClass);

		if (StartupAbility.InputTag.IsValid())
		{
			AbilitySpec
				.GetDynamicSpecSourceTags()
				.AddTag(StartupAbility.InputTag);
		}

		AbilitySystemComponent->GiveAbility(
			AbilitySpec);
	}

	AbilitySystemComponent->NotifyAbilityBarChanged();
}
