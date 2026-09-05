#include "EnemyCharacter.h"
#include "GenericCharacter.h"
#include "../UI/EnemyHealthBarWidget.h"
#include "TestGame/AbilitySystem/Attributes/HealthAttributeSet.h"
#include "AbilitySystemComponent.h"
#include "Components/WidgetComponent.h"

// Sets default values
AEnemyCharacter::AEnemyCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	AIControllerClass = AEnemyAIController::StaticClass();

	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

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

    if (HealthAttributeSet)
    {
        RefreshHealthBar(HealthAttributeSet->GetHealth());
    }
}

// Called every frame
void AEnemyCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
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