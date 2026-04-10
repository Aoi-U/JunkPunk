#pragma once

#include "../Core/Types.h"
#include "../Core/AudioEngine.h"

#include "System.h"

class Event;

class AudioSystem : public System
{
public:
	void Init();
	void Update();
	void ResetMusicState() { lastState = GameState::ENDMENU; }

private:
	CAudioEngine aEngine;
	std::vector<int> engineSoundChannelIds = {-1,-1,-1,-1};
	int musicChannelId = -1;
	bool firstUpdate = true;

	void AudioEventListener(Event& e);

	void JumpEventListener(Event& e);

	void PlayMusic(const std::string& path, float volumeDb);
	GameState lastState = GameState::STARTMENU;
};