#include "LevelChunkDefinition.h"


bool ULevelChunkDefinition::HasConnection(
    EChunkConnectionDirection Direction
) const
{
    return Connections.Contains(Direction);
}


EChunkConnectionDirection ULevelChunkDefinition::GetOppositeDirection(
    EChunkConnectionDirection Direction
)
{
    switch (Direction)
    {
    case EChunkConnectionDirection::North:
        return EChunkConnectionDirection::South;

    case EChunkConnectionDirection::East:
        return EChunkConnectionDirection::West;

    case EChunkConnectionDirection::South:
        return EChunkConnectionDirection::North;

    case EChunkConnectionDirection::West:
        return EChunkConnectionDirection::East;

    default:
        return Direction;
    }
}

EChunkConnectionDirection ULevelChunkDefinition::GetRotatedDirection(
    EChunkConnectionDirection Direction,
    EChunkRotation Rotation
)
{
    int32 DirectionIndex = static_cast<int32>(Direction);
    int32 RotationSteps = 0;

    switch (Rotation)
    {
    case EChunkRotation::Degrees0:
        RotationSteps = 0;
        break;

    case EChunkRotation::Degrees90:
        RotationSteps = 1;
        break;

    case EChunkRotation::Degrees180:
        RotationSteps = 2;
        break;

    case EChunkRotation::Degrees270:
        RotationSteps = 3;
        break;

    default:
        RotationSteps = 0;
        break;
    }

    const int32 RotatedIndex = (DirectionIndex + RotationSteps) % 4;

    return static_cast<EChunkConnectionDirection>(RotatedIndex);
}

TSet<EChunkConnectionDirection>
ULevelChunkDefinition::GetConnectionsForRotation(
    EChunkRotation Rotation
) const
{
    TSet<EChunkConnectionDirection> RotatedConnections;

    for (const EChunkConnectionDirection Connection : Connections)
    {
        RotatedConnections.Add(
            GetRotatedDirection(Connection, Rotation)
        );
    }

    return RotatedConnections;
}