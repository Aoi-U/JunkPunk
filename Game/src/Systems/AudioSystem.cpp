#include <glm/glm.hpp>

#include "AudioSystem.h"
#include "../Components/Transform.h"
#include "../ECSController.h"
#include "../Components/Physics.h"



extern ECSController controller;

void AudioSystem::Init()
{
	aEngine.Init();
	
	// preload sounds
	aEngine.LoadSound("assets/audio/jazz-background-music-325355.mp3", false);
	aEngine.LoadSound("assets/audio/mariojump.mp3", false);
	aEngine.LoadSound("assets/audio/MenuNavigation.wav", false);
	aEngine.LoadSound("assets/audio/carAudio/carEngineIdle.mp3", false, true);

	// event listeners
	controller.AddEventListener(Events::Audio::PLAY_SOUND, [this](Event& e) {this->AudioEventListener(e); });
	controller.AddEventListener(Events::Player::PLAYER_JUMPED, [this](Event& e) {this->JumpEventListener(e); });

}

void AudioSystem::Update() {
	aEngine.Update();
	if (currentStateGlobal == GameState::GAME)
	{
		Entity player = controller.GetEntityByTag("Player1");
		auto& vehicleVelocity = controller.GetComponent<VehicleBody>(player);
		float speed = glm::length(vehicleVelocity.linearVelocity);

		if (engineSoundChannelId == -1)
		{
			engineSoundChannelId = aEngine.PlaySounds("assets/audio/carAudio/carEngineIdle.mp3", Vector3{ 0, 0, 0 }, -20.0f);
		}

		// Adjust pitch based on speed
		float pitchValue = 0.5f + speed * 0.05f;
		aEngine.SetChannelPitch(engineSoundChannelId, pitchValue);

	}
	else
	{
		// Stop engine sound when not in game (menu, pause, etc.)
		// Reset channel ID when exiting game state
		if (engineSoundChannelId != -1)  // Only stop if valid channel exists
		{
			aEngine.StopChannel(engineSoundChannelId);
			engineSoundChannelId = -1;
		}
	}
}

void AudioSystem::AudioEventListener(Event& e)
{
	std::string soundPath = e.GetParam<std::string>(Events::Audio::Play_Sound::SOUND_NAME);
	glm::vec3 vPos = e.GetParam<glm::vec3>(Events::Audio::Play_Sound::POSITION);
	Vector3 position = { vPos.x, vPos.y, vPos.z };
	float volumeDb = e.GetParam<float>(Events::Audio::Play_Sound::VOLUME_DB);

	aEngine.PlaySounds(soundPath, position, volumeDb);
	//aEngine.PlaySounds("/*assets/audio/jazz-background-music-325355.mp3*/", Vector3{0, 0, 0}, -10.0f);
}

void AudioSystem::JumpEventListener(Event& e)
{
	Entity entity = e.GetParam<Entity>(Events::Player::Player_Jumped::ENTITY);

	// play the jump sound at the entitys position
	if (controller.HasComponent<Transform>(entity))
	{
		auto& transform = controller.GetComponent<Transform>(entity);
		Vector3 position = { transform.position.x, transform.position.y, transform.position.z };
		aEngine.PlaySounds("assets/audio/mariojump.mp3", position, -20.0f);
	}
	else // fall back to origin
	{
		aEngine.PlaySounds("assets/audio/mariojump.mp3", Vector3{ 0, 0, 0 }, -20.0f);
	}
}
