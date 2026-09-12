#include "EnemyCharacter.h"

#include "../AI/EnemyAIController.h"
#include "../UI/EnemyHealthBarWidget.h"
#include "../UI/FloatingCombatText/DamageNumberActor.h"

#include "AbilitySystemComponent.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "TestGame/AbilitySystem/Attributes/HealthAttributeSet.h"


// Sets default values
AEnemyCharacter::AEnemyCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	AIControllerClass = AEnemyAIController::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

    AggroSphere =
        CreateDefaultSubobject<USphereComponent>(
            TEXT("AggroSphere")
        );

    AggroSphere->SetupAttachment(RootComponent);

    AggroSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    AggroSphere->SetCollisionObjectType(ECC_WorldDynamic);
    AggroSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
    AggroSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

    AggroSphere->SetGenerateOverlapEvents(true);

    AggroSphere->OnComponentBeginOverlap.AddDynamic(
        this,
        &AEnemyCharacter::HandleAggroBeginOverlap
    );

    AggroSphere->OnComponentEndOverlap.AddDynamic(
        this,
        &AEnemyCharacter::HandleAggroEndOverlap
    );

	EnemyHealthWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("EnemyHealthWidget"));

	EnemyHealthWidget->SetupAttachment(RootComponent);
	EnemyHealthWidget->SetWidgetSpace(EWidgetSpace::Screen);
	EnemyHealthWidget->SetDrawAtDesiredSize(true);

    EnemyHealthWidget->SetRelativeLocation(FVector(0.0f, 0.0f, 120.0f));
}

// Called when the game starts or when spawned
void AEnemyCharacter::BeginPlay()
{
	Super::BeginPlay();	

    if (AggroSphere)
    {
        AggroSphere->SetSphereRadius(
            AggroRange,
            true
        );
    }

    UE_LOG(
        LogTemp,
        Warning,
        TEXT("ENEMY BEGINPLAY: Binding damage event for %s"),
        *GetName()
    );

    OnDamageReceived.AddUObject(
        this,
        &AEnemyCharacter::HandleDamageReceived
    );

    if (HealthAttributeSet)
    {
        RefreshHealthBar(HealthAttributeSet->GetHealth());
    }

    GetWorldTimerManager().SetTimerForNextTick(
        this,
        &AEnemyCharacter::InitializeAggroTarget
    );
}

void AEnemyCharacter::InitializeAggroTarget()
{
    if (!AggroSphere)
    {
        return;
    }

    AggroSphere->UpdateOverlaps();

    APawn* PlayerPawn =
        UGameplayStatics::GetPlayerPawn(
            GetWorld(),
            0
        );

    if (!IsValid(PlayerPawn))
    {
        return;
    }

    if (!AggroSphere->IsOverlappingActor(PlayerPawn))
    {
        return;
    }

    AEnemyAIController* EnemyController =
        Cast<AEnemyAIController>(
            GetController()
        );

    if (!EnemyController)
    {
        UE_LOG(
            LogTemp,
            Error,
            TEXT("%s has the wrong AI controller"),
            *GetName()
        );

        return;
    }

    EnemyController->SetAggroTarget(PlayerPawn);

    UE_LOG(
        LogTemp,
        Warning,
        TEXT("%s started with player inside aggro range"),
        *GetName()
    );
}

void AEnemyCharacter::HandleAggroBeginOverlap(
    UPrimitiveComponent* OverlappedComponent,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComponent,
    int32 OtherBodyIndex,
    bool bFromSweep,
    const FHitResult& SweepResult)
{

    UE_LOG(
        LogTemp,
        Warning,
        TEXT("AGGRO OVERLAP: Enemy=%s Other=%s"),
        *GetName(),
        OtherActor ? *OtherActor->GetName() : TEXT("None")
    );


    if (!IsValid(OtherActor) || OtherActor == this)
    {
        return;
    }

    APawn* PlayerPawn =
        UGameplayStatics::GetPlayerPawn(
            GetWorld(),
            0
        );

    if (OtherActor != PlayerPawn)
    {
        return;
    }

    AEnemyAIController* EnemyController =
        Cast<AEnemyAIController>(
            GetController()
        );

    if (!EnemyController)
    {
        return;
    }

    EnemyController->SetAggroTarget(OtherActor);

    UE_LOG(
        LogTemp,
        Log,
        TEXT("%s entered %s's aggro range"),
        *OtherActor->GetName(),
        *GetName()
    );
}

void AEnemyCharacter::HandleAggroEndOverlap(
    UPrimitiveComponent* OverlappedComponent,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComponent,
    int32 OtherBodyIndex)
{
    UE_LOG(
        LogTemp,
        Warning,
        TEXT("AGGRO END: Enemy=%s Other=%s"),
        *GetName(),
        IsValid(OtherActor)
        ? *OtherActor->GetName()
        : TEXT("None")
    );

    if (!IsValid(OtherActor))
    {
        return;
    }

    APawn* PlayerPawn =
        UGameplayStatics::GetPlayerPawn(
            GetWorld(),
            0
        );

    if (OtherActor != PlayerPawn)
    {
        return;
    }

    AEnemyAIController* EnemyController =
        Cast<AEnemyAIController>(
            GetController()
        );

    if (!EnemyController)
    {
        return;
    }

    EnemyController->ClearAggroTarget(OtherActor);
}

void AEnemyCharacter::RefreshHealthBar(float CurrentHealth)
{
    if (!EnemyHealthWidget)
    {
        return;
    }

    UEnemyHealthBarWidget* HealthBarWidget = Cast<UEnemyHealthBarWidget>(EnemyHealthWidget->GetUserWidgetObject());

    if (HealthBarWidget)
    {
        HealthBarWidget->SetHealth(CurrentHealth);
    }
}

void AEnemyCharacter::HandleAttributeChanged(FGameplayAttribute Attribute, float Magnitude, float NewHealth)
{
    Super::HandleAttributeChanged(Attribute, Magnitude, NewHealth);

    if (Attribute != UHealthAttributeSet::GetHealthAttribute())
    {
        return;
    }

    RefreshHealthBar(NewHealth);
}

void AEnemyCharacter::HandleDamageReceived(
    float DamageAmount)
{

    UE_LOG(
        LogTemp,
        Warning,
        TEXT("ENEMY SCT DAMAGE: %f"),
        DamageAmount
    );

    if (!DamageNumberActorClass)
    {
        return;
    }

    UWorld* World = GetWorld();

    if (!World)
    {
        return;
    }

    const FVector SpawnLocation =
        GetActorLocation() +
        FVector(0.0f, 0.0f, 120.0f);

    ADamageNumberActor* DamageNumber =
        World->SpawnActor<ADamageNumberActor>(
            DamageNumberActorClass,
            SpawnLocation,
            FRotator::ZeroRotator
        );

    if (!DamageNumber)
    {
        return;
    }

    DamageNumber->InitializeDamage(
        DamageAmount
    );
}