#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GA_GenericAoE.generated.h"

class AGenericCharacter;

UCLASS()
class TESTGAME_API UGA_GenericAoE : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:

	UFUNCTION(
		BlueprintCallable,
		Category = "Ability|AOE",
		meta = (WorldContext = "WorldContextObject")
	)
	static TArray<AGenericCharacter*>
		FindCharactersInRadius(
			UObject* WorldContextObject,
			const FVector& Origin,
			float Radius,
			AActor* ActorToIgnore,
			TSubclassOf<AGenericCharacter>RequiredCharacterClass
		);
};
