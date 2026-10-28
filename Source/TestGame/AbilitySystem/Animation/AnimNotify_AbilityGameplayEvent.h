#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "GameplayTagContainer.h"
#include "AnimNotify_AbilityGameplayEvent.generated.h"

UCLASS()
class TESTGAME_API UAnimNotify_AbilityGameplayEvent
    : public UAnimNotify
{
    GENERATED_BODY()

public:
    virtual void Notify(
        USkeletalMeshComponent* MeshComp,
        UAnimSequenceBase* Animation,
        const FAnimNotifyEventReference& EventReference
    ) override;

protected:
    UPROPERTY(
        EditAnywhere,
        BlueprintReadOnly,
        Category = "Ability Event"
    )
    FGameplayTag EventTag;
};