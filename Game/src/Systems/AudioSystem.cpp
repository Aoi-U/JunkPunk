#include <glm/glm.hpp>

#include "AudioSystem.h"
#include "../Components/Transform.h"
#include "../ECSController.h"


extern ECSController controller;

void AudioSystem::Init()
{
	aEngine.Init();
	
	// preload sounds
	aEngine.LoadSound("assets/audio/jazz-background-music-325355.mp3", false);
	aEngine.LoadSound("assets/audio/mariojump.mp3", false);

	controller.AddEventListener(Events::Audio::PLAY_SOUND, [this](Event& e) {this->AudioEventListener(e); });
	controller.AddEventListener(Events::Player::PLAYER_JUMPED, [this](Event& e) {this->JumpEventListener(e); });
}

void AudioSystem::AudioEventListener(Event& e)
{
	std::string soundPath = e.GetParam<std::string>(Events::Audio::Play_Sound::SOUND_NAME);
	Vector3 position = e.GetParam<Vector3>(Events::Audio::Play_Sound::POSITION);
	float volumeDb = e.GetParam<float>(Events::Audio::Play_Sound::VOLUME_DB);

	aEngine.PlaySounds(soundPath, position, volumeDb);
	//aEngine.PlaySounds("/*assets/audio/jazz-background-music-325355.mp3*/", Vector3{0, 0, 0}, -10.0f);
}

void AudioSystem::JumpEventListener(Event& e)
{
	Entity entity = e.GetParam<Entity>(Events::Player::Player_Jumped::ENTITY);

	if (controller.HasComponent<Transform>(entity))
	{
		auto& transform = controller.GetComponent<Transform>(entity);
		Vector3 position = { transform.position.x, transform.position.y, transform.position.z };
		aEngine.PlaySounds("assets/audio/mariojump.mp3", position, -20.0f);
	}
	else
	{
		aEngine.PlaySounds("assets/audio/mariojump.mp3", Vector3{ 0, 0, 0 }, -20.0f);
	}
}
