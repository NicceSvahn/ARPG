#pragma once

#include "CoreMinimal.h"
#include "GenericAttributeSet.h"
#include "Net/UnrealNetwork.h"
#include "DefensiveAttributeSet.generated.h"

UCLASS()
class TESTGAME_API UDefensiveAttributeSet : public UGenericAttributeSet
{
    GENERATED_BODY()

public:
    virtual void GetLifetimeReplicatedProps(
        TArray<FLifetimeProperty>& OutLifetimeProps
    ) const override;

    UPROPERTY(
        BlueprintReadOnly,
        ReplicatedUsing = OnRep_Armour,
        Category = "Attributes|Defensive"
    )
    FGameplayAttributeData Armour;

    PLAY_ATTRIBUTE_ACCESSORS(UDefensiveAttributeSet, Armour);

    UPROPERTY(
        BlueprintReadOnly,
        ReplicatedUsing = OnRep_BlockChance,
        Category = "Attributes|Defensive"
    )
    FGameplayAttributeData BlockChance;

    PLAY_ATTRIBUTE_ACCESSORS(UDefensiveAttributeSet, BlockChance);

    UPROPERTY(
        BlueprintReadOnly,
        ReplicatedUsing = OnRep_MagicResistance,
        Category = "Attributes|Defensive"
    )
    FGameplayAttributeData MagicResistance;

    PLAY_ATTRIBUTE_ACCESSORS(UDefensiveAttributeSet, MagicResistance);

    UPROPERTY(
        BlueprintReadOnly,
        ReplicatedUsing = OnRep_Evasion,
        Category = "Attributes|Defensive"
    )
    FGameplayAttributeData Evasion;

    PLAY_ATTRIBUTE_ACCESSORS(UDefensiveAttributeSet, Evasion);

protected:
    UFUNCTION()
    void OnRep_Armour(
        const FGameplayAttributeData& OldArmour
    );

    UFUNCTION()
    void OnRep_BlockChance(
        const FGameplayAttributeData& OldBlockChance
    );

    UFUNCTION()
    void OnRep_MagicResistance(
        const FGameplayAttributeData& OldMagicResistance
    );

    UFUNCTION()
    void OnRep_Evasion(
        const FGameplayAttributeData& OldEvasion
    );

};
