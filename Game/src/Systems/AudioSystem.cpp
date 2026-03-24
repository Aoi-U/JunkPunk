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

	aEngine.LoadSound("assets/audio/main.mp3", false);
	aEngine.LoadSound("assets/audio/main2.mp3", false);
	aEngine.LoadSound("assets/audio/intro.mp3", false);
	aEngine.LoadSound("assets/audio/outro.mp3", false);
	aEngine.LoadSound("assets/audio/pause.mp3", false);

	// event listeners
	controller.AddEventListener(Events::Audio::PLAY_SOUND, [this](Event& e) {this->AudioEventListener(e); });
	controller.AddEventListener(Events::Player::PLAYER_JUMPED, [this](Event& e) {this->JumpEventListener(e); });

}

void AudioSystem::Update() {
	aEngine.Update();

	if (firstUpdate || currentStateGlobal != lastState)
	{
		firstUpdate = false;

		if (currentStateGlobal != GameState::PAUSED)
		{
			lastState = currentStateGlobal;

			switch (currentStateGlobal)
			{
			case GameState::STARTMENU:
			case GameState::SETTINGS:
				PlayMusic("assets/audio/main.mp3", -25.0f);
				break;

			case GameState::CONTROLS:
				PlayMusic("assets/audio/pause.mp3", -25.0f);
				break;

			case GameState::GAME:
				PlayMusic("assets/audio/intro.mp3", -25.0f);
				break;

			case GameState::ENDMENU:
				PlayMusic("assets/audio/outro.mp3", -25.0f);
				break;
			}
		}
	}

	if (currentStateGlobal == GameState::GAME && lastState == GameState::GAME && firstUpdate == false)
	{
		// Detect restart case manually
		if (musicChannelId == -1)
		{
			PlayMusic("assets/audio/main.mp3", -25.0f);
		}
	}

	static bool wasPaused = false;

	if (currentStateGlobal == GameState::PAUSED)
	{
		if (!wasPaused)
		{
			wasPaused = true;

			if (musicChannelId != -1)
			{
				aEngine.SetChannelVolume(musicChannelId, -40.0f);
				aEngine.SetChannelPitch(musicChannelId, 0.7f);
			}
		}
	}
	else
	{
		if (wasPaused)
		{
			wasPaused = false;

			if (musicChannelId != -1)
			{
				aEngine.SetChannelVolume(musicChannelId, -25.0f);
				aEngine.SetChannelPitch(musicChannelId, 1.0f);
			}
		}
	}

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

void AudioSystem::PlayMusic(const std::string& path, float volumeDb) {

	if (musicChannelId != -1) {
		aEngine.StopChannel(musicChannelId);
		musicChannelId = -1;
	}
	musicChannelId = aEngine.PlaySounds(path, Vector3{ 0,0,0 }, volumeDb);
}