#include "GenericCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Abilities/GameplayAbility.h"
#include "../AbilitySystem/Attributes/ResourceAttributeSet.h"
#include "../AbilitySystem/Attributes/MovementSpeedAttributeSet.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/Controller.h"
#include "Net/UnrealNetwork.h"
#include "TimerManager.h"
#include "Components/SkeletalMeshComponent.h"

AGenericCharacter::AGenericCharacter()
{
	PrimaryActorTick.bCanEverTick = false;

	bReplicates = true;
	SetReplicateMovement(true);

	AbilitySystemComponent =
		CreateDefaultSubobject<UTestGameAbilitySystemComponent>(
			TEXT("AbilitySystemComponent")
		);

	HealthAttributeSet =
		CreateDefaultSubobject<UHealthAttributeSet>(
			TEXT("HealthAttributeSet")
		);

	ResourceAttributeSet =
		CreateDefaultSubobject<UResourceAttributeSet>(
			TEXT("ResourceAttributeSet")
		);

	MovementSpeedAttributeSet =
		CreateDefaultSubobject<UMovementSpeedAttributeSet>(
			TEXT("MovementSpeedAttributeSet")
		);
}
void AGenericCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (AbilitySystemComponent)
	{

		AbilitySystemComponent->InitAbilityActorInfo(this, this);

		HitReactionTag =
			FGameplayTag::RequestGameplayTag(
				TEXT("State.HitReaction.Hit"));

		HitReactionTagChangedHandle =
			AbilitySystemComponent
			->RegisterGameplayTagEvent(
				HitReactionTag,
				EGameplayTagEventType::NewOrRemoved)
			.AddUObject(
				this,
				&AGenericCharacter::HandleHitReactionTagChanged);

		ApplyResourceRegeneration();

		if (HasAuthority())
		{
			GrantStartupAbilities();
		}
	}

	if (HealthAttributeSet)
	{
		HealthAttributeSet->OnAttributeChanged.AddDynamic(
			this,
			&AGenericCharacter::HandleAttributeChanged
		);

		if (HasAuthority())
		{
			AbilitySystemComponent->SetNumericAttributeBase(
				UHealthAttributeSet::GetMaxHealthAttribute(),
				InitialHealth
			);

			AbilitySystemComponent->SetNumericAttributeBase(
				UHealthAttributeSet::GetHealthAttribute(),
				InitialHealth
			);
		}

		PreviousHealth =
			HealthAttributeSet->GetHealth();

		OnHealthChanged.Broadcast(
			GetCurrentHealth(),
			GetMaxHealth()
		);
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
	return HealthAttributeSet
		? HealthAttributeSet->GetMaxHealth()
		: 0.0f;
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

		if (NewValue <= 0 &&
			HasAuthority())
		{
			EnterDeathState();
		}

		if (NewValue < OldHealth)
		{
			const float DamageAmount =
				OldHealth - NewValue;

			OnDamageReceived.Broadcast(
				DamageAmount);

			if (NewValue > 0.0f &&
				HasAuthority())
			{
				MulticastHitReaction();
			}
		}
		return;
	}
	return;
}

void AGenericCharacter::MulticastHitReaction_Implementation()
{
	if (bIsDead ||
		!AbilitySystemComponent)
	{
		return;
	}

	if (!AbilitySystemComponent->HasMatchingGameplayTag(
		HitReactionTag))
	{
		AbilitySystemComponent->AddLooseGameplayTag(
			HitReactionTag);
	}

	GetWorldTimerManager().ClearTimer(
		HitReactionTimerHandle);

	if (HitFlashDuration <= 0.0f)
	{
		ClearHitReactionState();
		return;
	}

	GetWorldTimerManager().SetTimer(
		HitReactionTimerHandle,
		this,
		&AGenericCharacter::ClearHitReactionState,
		HitFlashDuration,
		false);
}

void AGenericCharacter::HandleHitReactionTagChanged(
	const FGameplayTag Tag,
	int32 NewCount)
{
	USkeletalMeshComponent* CharacterMesh =
		GetMesh();

	if (!CharacterMesh)
	{
		return;
	}

	if (NewCount > 0 &&
		HitFlashMaterial)
	{
		CharacterMesh->SetOverlayMaterial(
			HitFlashMaterial);
	}
	else
	{
		CharacterMesh->SetOverlayMaterial(
			nullptr);
	}
}

void AGenericCharacter::ClearHitReactionState()
{
	if (!AbilitySystemComponent)
	{
		return;
	}

	AbilitySystemComponent->RemoveLooseGameplayTag(
		HitReactionTag);
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
	if (!HasAuthority() ||
		bIsDead)
	{
		return;
	}

	GetWorldTimerManager().ClearTimer(
		HitReactionTimerHandle);

	ClearHitReactionState();

	bIsDead = true;

	ApplyDeathState();

	if (DeathCleanupDelay <= 0.0f)
	{
		Destroy();
		return;
	}

	SetLifeSpan(DeathCleanupDelay);
}

void AGenericCharacter::ApplyDeathState()
{
	if (AbilitySystemComponent)
	{
		const FGameplayTag DeadTag =
			FGameplayTag::RequestGameplayTag(
				FName(TEXT("State.Dead"))
			);

		AbilitySystemComponent->AddLooseGameplayTag(
			DeadTag
		);

		AbilitySystemComponent->HandleOwnerDeath();
	}

	if (Controller)
	{
		Controller->StopMovement();
	}

	if (UCharacterMovementComponent* MovementComponent =
		GetCharacterMovement())
	{
		MovementComponent->StopMovementImmediately();
		MovementComponent->DisableMovement();
	}

	if (UCapsuleComponent* CharacterComponent =
		GetCapsuleComponent())
	{
		CharacterComponent->SetCollisionEnabled(
			ECollisionEnabled::NoCollision
		);
	}

	OnDeathStarted();
	ReceiveDeath();
}

void AGenericCharacter::OnRep_IsDead()
{
	if (!bIsDead)
	{
		return;
	}

	ApplyDeathState();
}

void AGenericCharacter::OnDeathStarted()
{
}

void AGenericCharacter::GetLifetimeReplicatedProps(
	TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(
		OutLifetimeProps
	);

	DOREPLIFETIME(
		AGenericCharacter,
		bIsDead
	);
}