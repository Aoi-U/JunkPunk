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

private:
	CAudioEngine aEngine;
	int engineSoundChannelId = -1;

	void AudioEventListener(Event& e);

	void JumpEventListener(Event& e);
};