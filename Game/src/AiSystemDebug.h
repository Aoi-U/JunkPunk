#pragma once
#include "Core/Types.h"
#include <glm/glm.hpp>

class AiSystem; // forward declaration

class AiSystemDebug
{
public:
    static void SpawnDebugWaypoints(AiSystem& aiSystem, Entity aiEntity);
    static void SpawnDebugZoneTriggers(AiSystem& aiSystem);
};