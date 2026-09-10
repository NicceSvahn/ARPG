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
    UE_LOG(
        LogTemp,
        Warning,
        TEXT("GRANT START: %s | Count=%d"),
        *GetName(),
        StartupAbilities.Num()
    );

    if (!AbilitySystemComponent)
    {
        UE_LOG(
            LogTemp,
            Error,
            TEXT("GRANT FAILED: NO ASC")
        );

        return;
    }

    for (int32 Index = 0; Index < StartupAbilities.Num(); ++Index)
    {
        const FGrantedAbility& StartupAbility = StartupAbilities[Index];

        UE_LOG(
            LogTemp,
            Warning,
            TEXT("GRANT ENTRY %d: Class=%s Tag=%s"),
            Index,
            *GetNameSafe(StartupAbility.AbilityClass),
            *StartupAbility.InputTag.ToString()
        );

        if (!StartupAbility.AbilityClass)
        {
            UE_LOG(
                LogTemp,
                Error,
                TEXT("GRANT ENTRY %d: AbilityClass is NULL"),
                Index
            );

            continue;
        }

        FGameplayAbilitySpec AbilitySpec(
            StartupAbility.AbilityClass
        );

        if (StartupAbility.InputTag.IsValid())
        {
            AbilitySpec
                .GetDynamicSpecSourceTags()
                .AddTag(StartupAbility.InputTag);
        }

        AbilitySystemComponent->GiveAbility(
            AbilitySpec
        );

        UE_LOG(
            LogTemp,
            Warning,
            TEXT("GRANTED: %s | Tag=%s"),
            *GetNameSafe(StartupAbility.AbilityClass),
            *StartupAbility.InputTag.ToString()
        );
    }

    UE_LOG(
        LogTemp,
        Warning,
        TEXT("GRANT END: ASC now has %d abilities"),
        AbilitySystemComponent
        ->GetActivatableAbilities()
        .Num()
    );

    OnAbilitiesGranted.Broadcast();
}