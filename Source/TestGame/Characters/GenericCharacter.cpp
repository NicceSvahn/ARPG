#include "GenericCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Abilities/GameplayAbility.h"
#include "../AbilitySystem/Attributes/ResourceAttributeSet.h"
#include "../AbilitySystem/Attributes/MovementSpeedAttributeSet.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/Controller.h"

AGenericCharacter::AGenericCharacter()
{
	PrimaryActorTick.bCanEverTick = false;

	AbilitySystemComponent = CreateDefaultSubobject<UTestGameAbilitySystemComponent>(TEXT("AbilitySystemComponent"));

	HealthAttributeSet = CreateDefaultSubobject<UHealthAttributeSet>(TEXT("HealthAttributeSet"));

	ResourceAttributeSet = CreateDefaultSubobject<UResourceAttributeSet>(TEXT("ResourceAttributeSet"));

	MovementSpeedAttributeSet =	CreateDefaultSubobject<UMovementSpeedAttributeSet>(TEXT("MovementSpeedAttributeSet"));
}

void AGenericCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (AbilitySystemComponent)
	{

		AbilitySystemComponent->InitAbilityActorInfo(this, this);

		ApplyResourceRegeneration();

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
	}

	if (ResourceAttributeSet) 
	{
		ResourceAttributeSet->InitMaxResource(100.0f);
		ResourceAttributeSet->InitResource(100.0f);
	}

	if (MovementSpeedAttributeSet) 
	{
		MovementSpeedChangedHandle =
			AbilitySystemComponent
			->GetGameplayAttributeValueChangeDelegate(
				UMovementSpeedAttributeSet::GetMovementSpeedAttribute()
			)
			.AddUObject(
				this,
				&AGenericCharacter::HandleMovementSpeedChanged
			);

		MovementSpeedAttributeSet->InitMovementSpeed(InitialMovementSpeed);

		GetCharacterMovement()->MaxWalkSpeed = MovementSpeedAttributeSet->GetMovementSpeed();
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

void AGenericCharacter::HandleAttributeChanged(
	FGameplayAttribute Attribute, 
	float Magnitude, 
	float NewValue
)
{
	if (bIsDead)
	{
		return;
	}

	if (Attribute == UHealthAttributeSet::GetHealthAttribute())
	{
		const float OldHealth = PreviousHealth;
		PreviousHealth = NewValue;
		OnHealthChanged.Broadcast(NewValue, GetMaxHealth());

		if (NewValue <= 0)
		{
			EnterDeathState();
		}

		if (NewValue < OldHealth)
		{
			const float DamageAmount = OldHealth - NewValue;
			OnDamageReceived.Broadcast(DamageAmount);
		}
		else if (NewValue > OldHealth)
		{
			const float HealingAmount = NewValue - OldHealth;
			OnHealingReceived.Broadcast(HealingAmount);
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

void AGenericCharacter::ApplyResourceRegeneration()
{
	if (!AbilitySystemComponent ||
		!ResourceRegenerationEffect)
	{
		return;
	}

	if (!HasAuthority())
	{
		return;
	}

	FGameplayEffectContextHandle EffectContext =
		AbilitySystemComponent->MakeEffectContext();

	EffectContext.AddSourceObject(this);

	FGameplayEffectSpecHandle EffectSpec =
		AbilitySystemComponent->MakeOutgoingSpec(
			ResourceRegenerationEffect,
			1.0f,
			EffectContext
		);

	if (!EffectSpec.IsValid())
	{
		return;
	}

	AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(
		*EffectSpec.Data.Get()
	);
}

void AGenericCharacter::HandleMovementSpeedChanged(const FOnAttributeChangeData& Data)
{
	if (!GetCharacterMovement())
	{
		return;
	}

	GetCharacterMovement()->MaxWalkSpeed =
		FMath::Max(0.0f, Data.NewValue);
}

void AGenericCharacter::EnterDeathState()
{
	if (bIsDead)
	{
		return;
	}

	bIsDead = true;

	if (AbilitySystemComponent)
	{
		const FGameplayTag DeadTag =
			FGameplayTag::RequestGameplayTag(
				FName(TEXT("State.Dead")));

		AbilitySystemComponent->AddLooseGameplayTag(DeadTag);
		AbilitySystemComponent->HandleOwnerDeath();
	}

	if (Controller)
	{
		Controller->StopMovement();
	}

	if (UCharacterMovementComponent*
		MovementComponent = GetCharacterMovement())
	{
		MovementComponent->StopMovementImmediately();
		MovementComponent->DisableMovement();
	}

	if (UCapsuleComponent*
		CharacterComponent = GetCapsuleComponent())
	{
		CharacterComponent->SetCollisionEnabled(
			ECollisionEnabled::NoCollision);
	}

	OnDeathStarted();
	ReceiveDeath();

	if (DeathCleanupDelay <= 0.0f)
	{
		Destroy();
		return;
	}

	SetLifeSpan(DeathCleanupDelay);
}

void AGenericCharacter::OnDeathStarted()
{
}