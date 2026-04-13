#include "AiSystemHelperFunctions.h"
#include "../ECSController.h"
#include "../Components/AiDriver.h"
#include "../Components/Transform.h"
#include "../Components/DangerZone.h"
#include "../Components/Obstacle.h"
#include "../Components/Powerup.h"
#include "../Components/Player.h"
#include "../Game.h"
#include <iostream>
#include <cmath>

extern ECSController controller;

// ===== LOGGING HELPERS =====

bool AiSystemHelperFunctions::ShouldLog(float& timer, float interval, float deltaTime)
{
	timer += deltaTime;
	if (timer >= interval)
	{
		timer = 0.0f;
		return true;
	}
	return false;
}

// ===== DANGER ZONE HELPERS =====

float AiSystemHelperFunctions::CheckDangerZone(const glm::vec3& position)
{
	// Placeholder implementation - could be enhanced to return actual danger level
	return IsPointInDangerZone(position) ? 0.0f : -1.0f;
}

bool AiSystemHelperFunctions::IsPointInDangerZone(const glm::vec3& point)
{
	auto dangerArray = controller.GetComponentArray<DangerZone>();
	if (!dangerArray)
		return false;

	for (auto& [dzEntity, idx] : dangerArray->GetEntityToIndexMap())
	{
		auto& dz = controller.GetComponent<DangerZone>(dzEntity);

		// Simple AABB point-in-box test (no obstacle position check)
		glm::vec3 diff = glm::abs(point - dz.center);
		if (diff.x <= dz.halfExtents.x &&
			diff.y <= dz.halfExtents.y &&
			diff.z <= dz.halfExtents.z)
		{
			return true; // point is in A danger zone
		}
	}
	return false;
}

bool AiSystemHelperFunctions::IsPointInActiveDangerZone(const glm::vec3& point)
{
	auto dangerArray = controller.GetComponentArray<DangerZone>();
	for (auto& [dzEntity, idx] : dangerArray->GetEntityToIndexMap())
	{
		auto& dz = controller.GetComponent<DangerZone>(dzEntity);

		// Check if the linked obstacle is currently extended (dangerous)
		if (dz.obstacleEntity != 0 && controller.HasComponent<MovingObstacle>(dz.obstacleEntity))
		{
			auto& obstacle = controller.GetComponent<MovingObstacle>(dz.obstacleEntity);

			// Path indices 0->1 = extending, 1->2 = holding extended
			// Path indices 2->3 = retracting
			// Only dangerous when extending or holding (indices 0 or 1)
			bool gloveIsOut = (obstacle.currentPathIndex <= 1);
			if (!gloveIsOut)
				continue; // glove is retracting, safe to pass
		}

		// Simple AABB point-in-box test (ignoring Y for a flat arena)
		glm::vec3 diff = glm::abs(point - dz.center);
		if (diff.x <= dz.halfExtents.x &&
			diff.y <= dz.halfExtents.y &&
			diff.z <= dz.halfExtents.z)
		{
			return true;
		}
	}
	return false;
}

bool AiSystemHelperFunctions::IsObstacleInDangerZone(const glm::vec3& point)
{
	auto dangerArray = controller.GetComponentArray<DangerZone>();
	if (!dangerArray)
		return false;

	for (auto& [dzEntity, idx] : dangerArray->GetEntityToIndexMap())
	{
		auto& dz = controller.GetComponent<DangerZone>(dzEntity);

		// First check if the POINT is in this danger zone (AABB test)
		glm::vec3 diff = glm::abs(point - dz.center);
		if (!(diff.x <= dz.halfExtents.x && diff.z <= dz.halfExtents.z))
			continue; // point not in this zone, skip it

		// Point IS in the zone, now check if the obstacle is physically present
		if (dz.obstacleEntity != 0 && controller.HasComponent<MovingObstacle>(dz.obstacleEntity))
		{
			auto& obstacle = controller.GetComponent<MovingObstacle>(dz.obstacleEntity);

			// Is the glove currently extended (blocking the zone)?
			// Path indices 0->1 = extending/extended (GLOVE IS THERE)
			// Path indices 2->3 = retracting/retracted (GLOVE IS GONE)
			bool gloveIsPhysicallyPresent = (obstacle.currentPathIndex <= 1);

			if (gloveIsPhysicallyPresent)
			{
				// Glove is blocking - return true without logging (caller will log if needed)
				return true; // YES, glove is blocking the zone!
			}
		}
	}

	return false; // either no zone found, or glove is retracted
}

bool AiSystemHelperFunctions::HasDangerZone(Entity obstacleEntity)
{
	auto dangerArray = controller.GetComponentArray<DangerZone>();
	if (!dangerArray)
		return false;

	for (auto& [dzEntity, idx] : dangerArray->GetEntityToIndexMap())
	{
		auto& dz = controller.GetComponent<DangerZone>(dzEntity);
		if (dz.obstacleEntity == obstacleEntity)
			return true;
	}
	return false;
}

float AiSystemHelperFunctions::GetDistanceToDangerZone(const glm::vec3& point)
{
	auto dangerArray = controller.GetComponentArray<DangerZone>();
	if (!dangerArray)
		return 999.0f; // no danger zones exist

	float minDist = 999.0f;

	for (auto& [dzEntity, idx] : dangerArray->GetEntityToIndexMap())
	{
		auto& dz = controller.GetComponent<DangerZone>(dzEntity);

		// Calculate AABB bounds
		glm::vec3 minBounds = dz.center - dz.halfExtents;
		glm::vec3 maxBounds = dz.center + dz.halfExtents;

		// Find closest point on AABB to the given point
		glm::vec3 closestPoint;
		closestPoint.x = glm::clamp(point.x, minBounds.x, maxBounds.x);
		closestPoint.y = glm::clamp(point.y, minBounds.y, maxBounds.y);
		closestPoint.z = glm::clamp(point.z, minBounds.z, maxBounds.z);

		// Calculate distance (ignoring Y for flat distance)
		glm::vec3 diff = point - closestPoint;
		//diff.y = 0.0f;
		float dist = glm::length(diff);

		if (dist < minDist)
			minDist = dist;
	}

	return minDist;
}

glm::vec3 AiSystemHelperFunctions::GetBoxingGloveZoneExitPoint()
{
    // Target exit point at the end of the boxing glove zone
    return glm::vec3(-76.0f, -185.0f, -58.0f);
}

bool AiSystemHelperFunctions::IsArmBlocking(const glm::vec3& carPos)
{
	auto dangerArray = controller.GetComponentArray<DangerZone>();
	if (!dangerArray)
		return false;

	for (auto& [dzEntity, idx] : dangerArray->GetEntityToIndexMap())
	{
		auto& dz = controller.GetComponent<DangerZone>(dzEntity);

		// Check if car is near this danger zone
		glm::vec3 diff = glm::abs(carPos - dz.center);
		if (diff.x > dz.halfExtents.x + 15.0f || diff.z > dz.halfExtents.z + 15.0f)
			continue; // too far from this zone

		// Check if the glove is physically extended
		if (dz.obstacleEntity != 0 && controller.HasComponent<MovingObstacle>(dz.obstacleEntity))
		{
			auto& obstacle = controller.GetComponent<MovingObstacle>(dz.obstacleEntity);
			if (obstacle.currentPathIndex < 3) // glove not fully retracted
			{
				return true;
			}
		}
	}

	return false;
}

bool AiSystemHelperFunctions::IsBoxingGloveArmExtending(const glm::vec3& carPos, float checkDistance)
{
	auto dangerArray = controller.GetComponentArray<DangerZone>();
	if (!dangerArray)
		return false;

	for (auto& [dzEntity, idx] : dangerArray->GetEntityToIndexMap())
	{
		auto& dz = controller.GetComponent<DangerZone>(dzEntity);

		// Check if danger zone is within checkDistance
		glm::vec3 diff = carPos - dz.center;
		diff.y = 0.0f; // ignore vertical
		float dist = glm::length(diff);

		if (dist > checkDistance)
			continue; // too far

		// Check arm state
		if (dz.obstacleEntity != 0 && controller.HasComponent<MovingObstacle>(dz.obstacleEntity))
		{
			auto& obstacle = controller.GetComponent<MovingObstacle>(dz.obstacleEntity);

			// Check if arm is extending or extended (dangerous)
			if (obstacle.currentPathIndex == 0 || obstacle.currentPathIndex == 1)
			{
				return true; // ARM IS EXTENDING/EXTENDED - WAIT!
			}
		}
	}

	return false; // No extending arms nearby
}

float AiSystemHelperFunctions::ScanForDangerZoneInCone(const glm::vec3& carPos, const glm::vec3& forward,
	float detectionRange, float detectionCone, Entity* outClosestDangerEntity)
{
	auto dangerArray = controller.GetComponentArray<DangerZone>();
	if (!dangerArray)
		return -1.0f; // no danger zones exist

	float closestDist = detectionRange;
	Entity closestDangerEntity = 0;

	for (auto& [dzEntity, idx] : dangerArray->GetEntityToIndexMap())
	{
		auto& dz = controller.GetComponent<DangerZone>(dzEntity);

		// Calculate vector from car to danger zone center
		glm::vec3 toDanger = dz.center - carPos;
		toDanger.y = 0.0f; // flatten to horizontal plane

		float dist = glm::length(toDanger);

		// Skip if too far or too close
		if (dist < 1e-5f || dist > closestDist)
			continue;

		// Check if danger zone is in the forward cone
		glm::vec3 dirToDanger = glm::normalize(toDanger);
		float dot = glm::dot(forward, dirToDanger);

		if (dot < detectionCone)
			continue; // outside detection cone

		// This danger zone is in range and in the cone
		closestDist = dist;
		closestDangerEntity = dzEntity;
	}

	// Output the closest entity if requested
	if (outClosestDangerEntity != nullptr)
		*outClosestDangerEntity = closestDangerEntity;

	// Return distance to closest danger zone, or -1 if none found
	return (closestDangerEntity != 0) ? closestDist : -1.0f;
}

// ===== ZONE DETECTION HELPERS =====

bool AiSystemHelperFunctions::IsInBoxingGloveZone(const glm::vec3& pos)
{
	return (pos.x > -76.0f && pos.x < 48.0f &&
		pos.z > -74.0f && pos.z < -41.0f &&
		pos.y > -185.0f && pos.y < -168.0f);
}

float AiSystemHelperFunctions::ScanForBoxingGloveZoneInCone(const glm::vec3& carPos, const glm::vec3& forward,
	float detectionRange, float detectionCone)
{
	// Boxing glove zone center and bounds
	glm::vec3 zoneBegin(48.0f, -168.0f, -41.0f);
	glm::vec3 zoneEnd(-76.0f, -185.0f, -74.0f);

	// Vector from car to zone center
	glm::vec3 toZone = zoneBegin - carPos;
	toZone.y = 0.0f; // flatten to horizontal plane

	float dist = glm::length(toZone);

	// Check range
	if (dist < 1e-5f || dist > detectionRange)
		return -1.0f;

	// Check cone
	glm::vec3 dirToZone = glm::normalize(toZone);
	float dot = glm::dot(forward, dirToZone);

	if (dot < detectionCone)
		return -1.0f; // outside cone

	return dist; // zone detected in cone
}

bool AiSystemHelperFunctions::IsInGapZone(const glm::vec3& pos)
{
	// Gap area: X between -65 and +155, Z around 165-195, Y around -31
	// Covers the entire gap section from approach to landing
	return (pos.x > -65.0f && pos.x < 155.0f &&
		pos.z > 138.0f && pos.z < 219.0f &&
		pos.y > -25.0f && pos.y < -35.0f);
}

float AiSystemHelperFunctions::ScanForGapZoneInCone(const glm::vec3& carPos, const glm::vec3& forward,
	float detectionRange, float detectionCone)
{
	// Gap zone center
	glm::vec3 zoneCenter(45.0f, -30.0f, 178.5f); // approximate center

	glm::vec3 toZone = zoneCenter - carPos;
	toZone.y = 0.0f;

	float dist = glm::length(toZone);

	if (dist < 1e-5f || dist > detectionRange)
		return -1.0f;

	glm::vec3 dirToZone = glm::normalize(toZone);
	float dot = glm::dot(forward, dirToZone);

	if (dot < detectionCone)
		return -1.0f;

	return dist;
}

bool AiSystemHelperFunctions::IsInTunnelZone(const glm::vec3& pos)
{
	// Tunnel area: X between -140 and +155, Z between 240 and 400
	// Covers entire tunnel from entrance (beforeTunnel) to exit (midTunnel curve)
	return (pos.x > -145.0f && pos.x < 160.0f &&
		pos.z > 235.0f && pos.z < 405.0f &&
		pos.y > -35.0f && pos.y < -20.0f);
}

float AiSystemHelperFunctions::ScanForTunnelZoneInCone(const glm::vec3& carPos, const glm::vec3& forward,
	float detectionRange, float detectionCone)
{
	// Tunnel zone center
	glm::vec3 zoneCenter(7.5f, -27.5f, 320.0f); // approximate center

	glm::vec3 toZone = zoneCenter - carPos;
	toZone.y = 0.0f;

	float dist = glm::length(toZone);

	if (dist < 1e-5f || dist > detectionRange)
		return -1.0f;

	glm::vec3 dirToZone = glm::normalize(toZone);
	float dot = glm::dot(forward, dirToZone);

	if (dot < detectionCone)
		return -1.0f;

	return dist;
}

// ===== UTILITY FUNCTIONS =====

float AiSystemHelperFunctions::CalculateSteeringAngle(const glm::vec3& forward, const glm::vec3& toTarget)
{
	glm::vec2 fwd2D(forward.x, forward.z);
	glm::vec2 target2D(toTarget.x, toTarget.z);

	if (glm::length(target2D) < 1e-5f)
		return 0.0f;

	target2D = glm::normalize(target2D);

	float cross = fwd2D.x * target2D.y - fwd2D.y * target2D.x;
	float dot = glm::clamp(fwd2D.x * target2D.x + fwd2D.y * target2D.y, -1.0f, 1.0f);
	float angle = std::atan2(cross, dot);

	return glm::clamp(-angle * 1.0f, -1.0f, 1.0f);
}

// ===== ACTIONS =====

void AiSystemHelperFunctions::AdvanceThroughBoxingGlove(Entity entity)
{
	// Placeholder - currently unused
	// Could be implemented to handle special logic for navigating boxing glove zones
}

void AiSystemHelperFunctions::TryUsePowerup(Entity entity, Game* gameInstance)
{
	auto& ai = controller.GetComponent<AiDriver>(entity);
	auto& transform = controller.GetComponent<Transform>(entity);

	if (!ai.hasPowerup)
		return;

	glm::vec3 forward = transform.quatRotation * glm::vec3(0.0f, 0.0f, 1.0f);
	forward.y = 0.0f;
	if (glm::length(forward) < 1e-6f) forward = glm::vec3(0, 0, 1);
	forward = glm::normalize(forward);

	if (ai.heldPowerupType == 1)
	{
		// Speed boost: use on straightaways
		if (ai.currentWaypointIndex < static_cast<uint32_t>(ai.navWaypoints.size()))
		{
			glm::vec3 toWp = ai.navWaypoints[ai.currentWaypointIndex] - transform.position;
			toWp.y = 0.0f;
			float dist = glm::length(toWp);

			if (dist > 1e-5f)
			{
				float dot = glm::dot(forward, glm::normalize(toWp));

				if (dot > ai.useBoostDot)
				{
					controller.AddComponent(entity, Powerup{ 1, true, 5.0f, 0.0f });
					ai.hasPowerup = false;
					ai.heldPowerupType = 0;
					std::cout << "[AI] Using speed boost on straightaway" << std::endl;
				}
			}
		}
	}
	else if (ai.heldPowerupType == 2)
	{
		// Banana peel: drop when the player is close behind
		Entity playerEntity = 0;
		auto playerArray = controller.GetComponentArray<PlayerController>();
		for (auto& [pEntity, idx] : playerArray->GetEntityToIndexMap())
		{
			if (controller.HasComponent<Transform>(pEntity))
			{
				playerEntity = pEntity;
				break;
			}
		}

		if (playerEntity != 0)
		{
			auto& playerTransform = controller.GetComponent<Transform>(playerEntity);
			glm::vec3 toPlayer = playerTransform.position - transform.position;
			toPlayer.y = 0.0f;
			float dist = glm::length(toPlayer);

			if (dist > 1e-5f && dist < ai.dropBananaPlayerRange)
			{
				float dot = glm::dot(forward, glm::normalize(toPlayer));

				if (dot < -0.3f)
				{
					if (gameInstance)
					{
						gameInstance->SpawnBananaPeel(entity);
						ai.hasPowerup = false;
						ai.heldPowerupType = 0;
						std::cout << "[AI] Dropped banana peel behind" << std::endl;
					}
				}
			}
		}
	}
	else if (ai.heldPowerupType == 3) {
		// Bomb/Blast: use when the player is close (area-of-effect explosion)
		Entity playerEntity = 0;
		auto playerArray = controller.GetComponentArray<PlayerController>();
		for (auto& [pEntity, idx] : playerArray->GetEntityToIndexMap())
		{
			if (controller.HasComponent<Transform>(pEntity))
			{
				playerEntity = pEntity;
				break;
			}
		}

		if (playerEntity != 0)
		{
			auto& playerTransform = controller.GetComponent<Transform>(playerEntity);
			glm::vec3 toPlayer = playerTransform.position - transform.position;
			toPlayer.y = 0.0f;
			float dist = glm::length(toPlayer);

			if (dist > 1e-5f && dist < ai.dropBananaPlayerRange)
			{
				Event blastEvent(Events::Player::BLAST);
				blastEvent.SetParam<Entity>(Events::Player::Blast::ENTITY, entity);
				controller.SendEvent(blastEvent);

				Event soundEvent(Events::Audio::PLAY_SOUND);
				soundEvent.SetParam<std::string>(Events::Audio::Play_Sound::SOUND_NAME, "assets/audio/explosion.wav");
				soundEvent.SetParam<glm::vec3>(Events::Audio::Play_Sound::POSITION, glm::vec3(0.0f, 0.0f, 0.0f));
				soundEvent.SetParam<float>(Events::Audio::Play_Sound::VOLUME_DB, -10.0f);
				controller.SendEvent(soundEvent);

				ai.hasPowerup = false;
				ai.heldPowerupType = 0;
				std::cout << "[AI] Used bomb blast on nearby player" << std::endl;
			}
		}
	}
}


float AiSystemHelperFunctions::GetCurrentSpeed(Entity entity)
{
	if (!controller.HasComponent<VehicleBody>(entity))
		return 0.0f;

	auto& body = controller.GetComponent<VehicleBody>(entity);
	return glm::length(glm::vec3(body.linearVelocity.x, 0.0f, body.linearVelocity.z));
}