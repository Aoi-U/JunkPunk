#pragma once

#include <bitset>
#include <cstdint>

// defines types used for ECS

using Entity = std::uint32_t;
const Entity MAX_ENTITIES = 5000;
using ComponentType = std::uint8_t;
const ComponentType MAX_COMPONENTS = 32;
using Signature = std::bitset<MAX_COMPONENTS>;

using EventId = std::uint32_t;
using ParamId = std::uint32_t;


// https://gist.github.com/Lee-R/3839813 hasing functions
constexpr std::uint32_t fnv1a_32(char const* s, std::size_t count)
{
	return ((count ? fnv1a_32(s, count - 1) : 2166136261u) ^ s[count]) * 16777619u; // NOLINT (hicpp-signed-bitwise)
}

constexpr std::uint32_t operator "" _hash(char const* s, std::size_t count)
{
	return fnv1a_32(s, count);
}
// end hashing functions

#define METHOD_LISTENER(EventType, Listener) EventType, std::bind(&Listener, this, std::placeholders::_1)
#define FUNCTION_LISTENER(EventType, Listener) EventType, std::bind(&Listener, std::placeholders::_1)

// window events
namespace Events::Window // window related events
{
	const EventId RESIZED = "Events::Window::RESIZED"_hash; // window resized event
	const EventId INPUT = "Events::Window::INPUT"_hash; // keyboard input event
	const EventId SCROLLED = "Events::Window::SCROLLED"_hash; // mouse scrolled event
}

namespace Events::Window::Resized // parameters for window resized event
{
	const ParamId WIDTH = "Events::Window::Resized::WIDTH"_hash; // new width
	const ParamId HEIGHT = "Events::Window::Resized::HEIGHT"_hash; // new height
}

namespace Events::Window::Input // parameters for keyboard input event
{
	const ParamId INPUT = "Events::Window::Input::INPUT"_hash;  // input data
}

namespace Events::Window::Scrolled // parameters for mouse scrolled event
{
	const ParamId XOFFSET = "Events::Window::Scrolled::XOFFSET"_hash; // scroll x offset
	const ParamId YOFFSET = "Events::Window::Scrolled::YOFFSET"_hash; // scroll y offset
}

// audio events
namespace Events::Audio // audio related events
{
	const EventId PLAY_SOUND = "Events::Audio::PLAY_SOUND"_hash; // play sound event
	const EventId STOP_SOUND = "Events::Audio::STOP_SOUND"_hash; // stop sound event
}

namespace Events::Audio::Play_Sound // parameters for play sound event
{
	const ParamId SOUND_NAME = "Events::Audio::PlaySound::SOUND_NAME"_hash; // sound file name
	const ParamId POSITION = "Events::Audio::PlaySound::POSITION"_hash; // position to play sound at
	const ParamId VOLUME_DB = "Events::Audio::PlaySound::VOLUME_DB"_hash; // volume in decibels
}

namespace Events::Audio::Stop_Sound // parameter for stop sound event
{
	const ParamId SOUND_NAME = "Events::Audio::StopSound::SOUND_NAME"_hash; // sound file name
}

// physics events
namespace Events::Physics 
{
	const EventId TRIGGER_ENTER = "Events::Physics::TRIGGER_ENTER"_hash; // trigger enter event
	const EventId TRIGGER_EXIT = "Events::Physics::TRIGGER_EXIT"_hash; // trigger exit event
}

namespace Events::Physics::Trigger_Enter // parameters for trigger enter event
{
	const ParamId ENTITY_ONE = "Events::Physics::TriggerEnter::ENTITY_ONE"_hash; // first entity involved
	const ParamId ENTITY_TWO = "Events::Physics::TriggerEnter::ENTITY_TWO"_hash; // second entity involved
}

namespace Events::Physics::Trigger_Exit // parameters for trigger exit event
{
	const ParamId ENTITY_ONE = "Events::Physics::TriggerExit::ENTITY_ONE"_hash; // first entity involved
	const ParamId ENTITY_TWO = "Events::Physics::TriggerExit::ENTITY_TWO"_hash; // second entity involved
}

// player events
namespace Events::Player // player related events
{
	const EventId PLAYER_JUMPED = "Events::Player::PLAYER_JUMPED"_hash; // player jumped event
	const EventId RESET_VEHICLE = "Events::Player::RESET_VEHICLE"_hash; // reset vehicle event
}

namespace Events::Player::Player_Jumped // parameters for player jumped event
{
	const ParamId ENTITY = "Events::Physics::Player_Jumped::ENTITY"_hash; // entity that jumped
}

namespace Events::Player::Reset_Vehicle // parameters for reset vehicle event
{
	const ParamId ENTITY = "Events::Player::Reset_Vehicle::ENTITY"_hash;
}