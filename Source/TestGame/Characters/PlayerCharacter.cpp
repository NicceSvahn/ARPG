// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerCharacter.h"
#include "GenericCharacter.h"
#include "EnhancedInputComponent.h"
#include "InputAction.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/CharacterMovementComponent.h"

// Sets default values
APlayerCharacter::APlayerCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

    bUseControllerRotationYaw = false;

    GetCharacterMovement()->bOrientRotationToMovement = true;
    GetCharacterMovement()->bUseControllerDesiredRotation = false;
    GetCharacterMovement()->RotationRate = FRotator(0.f, 720.f, 0.f);
}

// Called when the game starts or when spawned
void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

    if (AbilitySystemComponent)
    {
        AbilitySystemComponent->InitAbilityActorInfo(this, this);
        AbilitySystemComponent->SetNumericAttributeBase(UHealthAttributeSet::GetHealthAttribute(), InitialHealth);

        if (HasAuthority() && BashAbility)
        {
            AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(BashAbility, 1, 0));
        }
    }
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

// Called to bind functionality to input
void APlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

    UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(PlayerInputComponent);

    if (!EnhancedInput)
    {
        UE_LOG(LogTemp, Error, TEXT("BASH: EnhancedInputComponent not found"));
        return;
    }

    if (!BashInputAction)
    {
        UE_LOG(LogTemp, Error, TEXT("BASH: BashInputAction is not assigned!"));
        return;
    }

    EnhancedInput->BindAction(
        BashInputAction,
        ETriggerEvent::Started,
        this,
        &APlayerCharacter::BashPressed
    );

    UE_LOG(LogTemp, Warning, TEXT("BASH: Input bound!"));

    //UE_LOG(LogTemp, Warning, TEXT("HELP ME"));
}

void APlayerCharacter::MoveToLocation(FVector& NewTarget)
{
    MovementTarget = NewTarget;
    bHasMovementTarget = true;
}

void APlayerCharacter::BashPressed()
{
    if (!AbilitySystemComponent)
    {
        UE_LOG(LogTemp, Error, TEXT("BASH: No AbilitySystemComponent!"));
        return;
    }

    if (!BashAbility)
    {
        UE_LOG(LogTemp, Error, TEXT("BASH: BashAbility is NOT assigned!"));
        return;
    }

    APlayerController* PlayerController = Cast<APlayerController>(GetController());

    if (!PlayerController)
    {
        UE_LOG(LogTemp, Error, TEXT("BASH: No PlayerController!"))
        return;
    }

    FHitResult HitResult;

    bool bHit = PlayerController->GetHitResultUnderCursor(
        ECC_Visibility,
        false,
        HitResult
    );

    if (!bHit)
    {
        UE_LOG(LogTemp, Warning, TEXT("BASH: Mouse is not pointing at anything!"));
        return;
    }

    AGenericCharacter* Target = Cast<AGenericCharacter>(HitResult.GetActor());

    if (!Target)
    {
        UE_LOG(LogTemp, Warning, TEXT("BASH: Mouse is not over a GenericCharacter!"));
        return;
    }

    BashTarget = Target;

    UE_LOG(
        LogTemp,
        Warning,
        TEXT("BASH: Mouse is pointing at %s"),
        *Target->GetName()
    );

    FGameplayEventData EventData;
    EventData.Target = Target;

    FGameplayAbilitySpec* Spec =
        AbilitySystemComponent->FindAbilitySpecFromClass(BashAbility);

    if (!Spec)
    {
        UE_LOG(LogTemp, Error, TEXT("BASH: Ability spec not found!"));
        return;
    }

    const bool bActivated = AbilitySystemComponent->TryActivateAbility(Spec->Handle);

    UE_LOG(
        LogTemp,
        Warning,
        TEXT("BASH: TryActivateAbility = %s"),
        bActivated ? TEXT("TRUE") : TEXT("FALSE")
    );
}