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

		if (HasAuthority())
		{
			GrantStartupAbilities();
		}
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

		FGameplayAbilitySpec AbilitySpec(StartupAbility.AbilityClass);

		if (StartupAbility.InputTag.IsValid())
		{
			AbilitySpec.GetDynamicSpecSourceTags().AddTag(StartupAbility.InputTag);
		}

		UE_LOG(
			LogTemp,
			Warning,
			TEXT("GRANTING: %s | ASC Owner=%s Avatar=%s"),
			*GetNameSafe(StartupAbility.AbilityClass),
			*GetNameSafe(AbilitySystemComponent->GetOwnerActor()),
			*GetNameSafe(AbilitySystemComponent->GetAvatarActor())
		);

		AbilitySystemComponent->GiveAbility(AbilitySpec);
	}
}