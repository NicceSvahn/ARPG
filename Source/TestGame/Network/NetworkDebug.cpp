#include "NetworkDebug.h"

#include "GameFramework/Actor.h"
#include "Engine/World.h"

FString NetworkDebug::GetNetModeString(
    const UObject* WorldContext)
{
    if (!WorldContext)
    {
        return TEXT("Unknown");
    }

    UWorld* World = WorldContext->GetWorld();

    if (!World)
    {
        return TEXT("Unknown");
    }

    switch (World->GetNetMode())
    {
    case NM_Standalone:
        return TEXT("Standalone");

    case NM_DedicatedServer:
        return TEXT("DedicatedServer");

    case NM_ListenServer:
        return TEXT("ListenServer");

    case NM_Client:
        return TEXT("Client");

    default:
        return TEXT("Unknown");
    }
}

FString NetworkDebug::GetRoleString(
    ENetRole Role)
{
    switch (Role)
    {
    case ROLE_None:
        return TEXT("None");

    case ROLE_SimulatedProxy:
        return TEXT("SimulatedProxy");

    case ROLE_AutonomousProxy:
        return TEXT("AutonomousProxy");

    case ROLE_Authority:
        return TEXT("Authority");

    default:
        return TEXT("Unknown");
    }
}

void NetworkDebug::LogActor(
    const AActor* Actor,
    const FString& Event)
{
    if (!Actor)
    {
        return;
    }

    UE_LOG(
        LogTemp,
        Warning,
        TEXT("[NET][%s] %s | Actor=%s | LocalRole=%s | RemoteRole=%s | LocalControl=%s"),
        *GetNetModeString(Actor),
        *Event,
        *GetNameSafe(Actor),
        *GetRoleString(Actor->GetLocalRole()),
        *GetRoleString(Actor->GetRemoteRole()),
        Actor->HasLocalNetOwner()
        ? TEXT("TRUE")
        : TEXT("FALSE")
    );
}