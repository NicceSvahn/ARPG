#include "AnimNotify_AbilityGameplayEvent.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "Components/SkeletalMeshComponent.h"

void UAnimNotify_AbilityGameplayEvent::Notify(
    USkeletalMeshComponent* MeshComp,
    UAnimSequenceBase* Animation,
    const FAnimNotifyEventReference& EventReference)
{
    Super::Notify(
        MeshComp,
        Animation,
        EventReference
    );

    if (!MeshComp ||
        !EventTag.IsValid())
    {
        return;
    }

    AActor* Owner =
        MeshComp->GetOwner();

    if (!Owner)
    {
        return;
    }

    FGameplayEventData Payload;

    Payload.EventTag = EventTag;
    Payload.Instigator = Owner;
    Payload.Target = Owner;

    UAbilitySystemBlueprintLibrary::
        SendGameplayEventToActor(
            Owner,
            EventTag,
            Payload
        );
}