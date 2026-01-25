#pragma once

#include "../Core/Types.h"
#include "../Core/AudioEngine.h"

#include "System.h"

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