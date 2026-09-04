#include "GA_Bash.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "../../Characters/PlayerCharacter.h"
#include "Kismet/KismetSystemLibrary.h"
#include "GameplayEffect.h"

UGA_Bash::UGA_Bash()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

void UGA_Bash::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	APlayerCharacter* PlayerCharacter =
		Cast<APlayerCharacter>(ActorInfo->AvatarActor.Get());

	if (!PlayerCharacter || !PlayerCharacter->BashTarget.IsValid())
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	CurrentTarget = PlayerCharacter->BashTarget.Get();

	AActor* Target = CurrentTarget.Get();

	AGenericCharacter* GenericCharacter =
		Cast<AGenericCharacter>(ActorInfo->AvatarActor.Get());

	if (!GenericCharacter || !Target)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	const float Distance =
		FVector::Dist(
			GenericCharacter->GetActorLocation(),
			Target->GetActorLocation()
		);

	if (Distance <= BashRange)
	{
		PerformBash(Target);
	}
	else
	{
		MoveToTarget(Target);
	}
}



void UGA_Bash::PerformBash(AActor* Target)
{
	AGenericCharacter* GenericCharacter = Cast<AGenericCharacter>(GetAvatarActorFromActorInfo());

	UE_LOG(LogTemp, Warning, TEXT("Performing BASH"));

	if (!GenericCharacter || !Target)
	{
		EndAbility(
			CurrentSpecHandle,
			CurrentActorInfo,
			CurrentActivationInfo,
			true,
			true
		);

		return;
	}

	CurrentTarget = Target;

	FVector Direction = Target->GetActorLocation() - GenericCharacter->GetActorLocation();

	Direction.Z = 0.0f;

	if (!Direction.IsNearlyZero())
	{
		GenericCharacter->SetActorRotation(Direction.Rotation());
	}

	if (BashMontage)
	{
		GenericCharacter->PlayAnimMontage(BashMontage);
	}

	if (DamageEffect)
	{
		UAbilitySystemComponent* SourceASC = GetAbilitySystemComponentFromActorInfo();

		UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Target);

		if (SourceASC && TargetASC)
		{
			FGameplayEffectContextHandle Context = SourceASC->MakeEffectContext();

			Context.AddSourceObject(this);

			FGameplayEffectSpecHandle Spec = SourceASC->MakeOutgoingSpec(DamageEffect, 1.0f, Context);

			if (Spec.IsValid())
			{
				SourceASC->ApplyGameplayEffectSpecToTarget(*Spec.Data.Get(), TargetASC);
			}
		}
	}
	EndAbility(
		CurrentSpecHandle,
		CurrentActorInfo,
		CurrentActivationInfo,
		true,
		false
	);
}

void UGA_Bash::OnTargetReached(AActor* Target)
{
	PerformBash(Target);
}




