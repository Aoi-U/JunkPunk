#pragma once

#include "../Core/Types.h"

#include "System.h"
#include "../Core/AudioEngine.h"

class Event;

class AudioSystem : public System
{
public:
	void Init();

private:
	CAudioEngine aEngine;

	void AudioEventListener(Event& e);

	void JumpEventListener(Event& e);
};