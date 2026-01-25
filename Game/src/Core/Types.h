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
namespace Events::Window 
{
	const EventId RESIZED = "Events::Window::RESIZED"_hash;
	const EventId INPUT = "Events::Window::INPUT"_hash;
	const EventId SCROLLED = "Events::Window::SCROLLED"_hash;
}

namespace Events::Window::Resized 
{
	const ParamId WIDTH = "Events::Window::Resized::WIDTH"_hash; // new width
	const ParamId HEIGHT = "Events::Window::Resized::HEIGHT"_hash; // new height
}

namespace Events::Window::Input 
{
	const ParamId INPUT = "Events::Window::Input::INPUT"_hash;  // input data
}

namespace Events::Window::Scrolled
{
	const ParamId XOFFSET = "Events::Window::Scrolled::XOFFSET"_hash; // scroll x offset
	const ParamId YOFFSET = "Events::Window::Scrolled::YOFFSET"_hash; // scroll y offset
}

// audio events
namespace Events::Audio 
{
	const EventId PLAY_SOUND = "Events::Audio::PLAY_SOUND"_hash;
	const EventId STOP_SOUND = "Events::Audio::STOP_SOUND"_hash;
}

namespace Events::Audio::Play_Sound 
{
	const ParamId SOUND_NAME = "Events::Audio::PlaySound::SOUND_NAME"_hash; // sound file name
	const ParamId POSITION = "Events::Audio::PlaySound::POSITION"_hash; // position to play sound at
	const ParamId VOLUME_DB = "Events::Audio::PlaySound::VOLUME_DB"_hash; // volume in decibels
}

// physics events
namespace Events::Physics 
{
	const EventId TRIGGER_ENTER = "Events::Physics::TRIGGER_ENTER"_hash;
	const EventId TRIGGER_EXIT = "Events::Physics::TRIGGER_EXIT"_hash;
}

namespace Events::Physics::Trigger_Enter 
{
	const ParamId ENTITY_ONE = "Events::Physics::TriggerEnter::ENTITY_ONE"_hash; // first entity involved
	const ParamId ENTITY_TWO = "Events::Physics::TriggerEnter::ENTITY_TWO"_hash; // second entity involved
}

namespace Events::Physics::Trigger_Exit
{
	const ParamId ENTITY_ONE = "Events::Physics::TriggerExit::ENTITY_ONE"_hash; // first entity involved
	const ParamId ENTITY_TWO = "Events::Physics::TriggerExit::ENTITY_TWO"_hash; // second entity involved
}

// player events
namespace Events::Player
{
	const EventId PLAYER_JUMPED = "Events::Player::PLAYER_JUMPED"_hash;
	const EventId RESET_VEHICLE = "Events::Player::RESET_VEHICLE"_hash;
}

namespace Events::Player::Player_Jumped
{
	const ParamId ENTITY = "Events::Physics::Player_Jumped::ENTITY"_hash; // entity that jumped
}

namespace Events::Player::Reset_Vehicle
{
	const ParamId ENTITY = "Events::Player::Reset_Vehicle::ENTITY"_hash;
}