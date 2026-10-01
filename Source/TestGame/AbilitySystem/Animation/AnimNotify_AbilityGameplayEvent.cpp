#include "AnimNotify_AbilityGameplayEvent.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
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

    // Animation Editor preview actors are not gameplay actors and do not have
    // an AbilitySystemComponent. Guard explicitly instead of using
    // SendGameplayEventToActor(), which logs an error for those preview actors.
    IAbilitySystemInterface* AbilitySystemInterface =
        Cast<IAbilitySystemInterface>(Owner);

    UAbilitySystemComponent* ASC =
        AbilitySystemInterface
        ? AbilitySystemInterface->GetAbilitySystemComponent()
        : nullptr;

    if (!ASC)
    {
        return;
    }

    FGameplayEventData Payload;

    Payload.EventTag = EventTag;
    Payload.Instigator = Owner;
    Payload.Target = Owner;

    ASC->HandleGameplayEvent(
        EventTag,
        &Payload
    );
}
