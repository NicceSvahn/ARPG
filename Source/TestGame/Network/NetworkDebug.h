#pragma once

#include "CoreMinimal.h"

namespace NetworkDebug
{
    FString GetNetModeString(const UObject* WorldContext);

    FString GetRoleString(ENetRole Role);

    void LogActor(
        const AActor* Actor,
        const FString& Event
    );
}