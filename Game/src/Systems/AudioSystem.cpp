#include <glm/glm.hpp>

#include "AudioSystem.h"
#include "../ECSController.h"


extern ECSController controller;

void AudioSystem::Init()
{
	aEngine.Init();
	
	// preload sounds
	aEngine.LoadSound("assets/audio/jazz-background-music-325355.mp3", false);

	controller.AddEventListener(Events::Audio::PLAY_SOUND, [this](Event& e) {this->AudioEventListener(e); });
}

void AudioSystem::AudioEventListener(Event& e)
{
	std::string soundPath = e.GetParam<std::string>(Events::Audio::Play_Sound::SOUND_NAME);
	Vector3 position = e.GetParam<Vector3>(Events::Audio::Play_Sound::POSITION);
	float volumeDb = e.GetParam<float>(Events::Audio::Play_Sound::VOLUME_DB);

	aEngine.PlaySounds(soundPath, position, volumeDb);
	//aEngine.PlaySounds("/*assets/audio/jazz-background-music-325355.mp3*/", Vector3{0, 0, 0}, -10.0f);
}
