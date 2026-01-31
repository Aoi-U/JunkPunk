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


// https://gist.github.com/Lee-R/3839813 hashing functions
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

// how events work:
//	Setting up the ECSController for a class:
//		include "Core/Types.h", "Event.h" and "ECSController.h" in a class that wants to send or receive events
//		define a global forward declaration "extern ECSController controller;" in that class (make sure it is declared globally, not within a class)
//  Setting up an event listener:
//		create a void listener function with the parameter (Event& e) in any class that wants to listen for events
//		in the constructor of that class, register the event listener to the ecs with controller.AddEventListener(Events::Event_type::EVENT_NAME, [this](Event& e) { this->ListenerFunctionName(e); })
//	Creating and sending an event:
//		events can be sent from anywhere in the program as long as it has access to the global ecs controller
//		create an Event object with the id of the event you want to send: Event event(Events::Event_type::EVENT_NAME);
//		set the parameters of the event with: event.SetParam<ParamType>(Events::Event_type::Event_subtype::PARAM_NAME, value you want to send);
//		send the event with: controller.SendEvent(event);
//		you can also send events with just the EventId but i havnt tested it yet so not exactly sure if it works or not
//	Receiving an event:
//		in the listener function, retrieve the value with: ParamType variableName = e.GetParam<ParamType>(Events::Event_type::Event_subtype::PARAM_NAME);
//		use this variable however you want
//  Example usage in Window.cpp for sending scroll event and receiving the event in CameraEditroPanelRender in Game.cpp

enum Buttons
{
	JUMP = 0, // A
	POWERUP = 1, // B
	X = 2, // X not using yet
	Y = 3, // Y not using yet
	LEFTROLL = 8, // LB
	RIGHTROLL = 9, // RB
	PAUSE = 12, // START
	RESET = 13  // BACK
};

enum GameState
{
	GAME,
	PAUSED,
	STARTMENU,
	ENDMENU,
	SETTINGS
};

// game state events
namespace Events::GameState
{
	const EventId NEW_STATE = "Events::GameState::NEW_STATE"_hash;
}

namespace Events::GameState::New_State
{
	const ParamId STATE = "Events::GameState::New_State"_hash;
}

// window events
namespace Events::Window // window related events
{
	const EventId RESIZED = "Events::Window::RESIZED"_hash; // window resized event
	const EventId INPUT = "Events::Window::INPUT"_hash; // keyboard input event
	const EventId SCROLLED = "Events::Window::SCROLLED"_hash; // mouse scrolled event
	const EventId MOUSEMOVED = "Events::Window::MOUSEMOVED"_hash; // mouse moved event
}

namespace Events::Window::Resized // parameters for window resized event
{
	const ParamId WIDTH = "Events::Window::Resized::WIDTH"_hash; // new width
	const ParamId HEIGHT = "Events::Window::Resized::HEIGHT"_hash; // new height
}

namespace Events::Window::Input // parameters for keyboard input event
{
	const ParamId KEY = "Events::Window::Input::KEY"_hash;  // input data
	const ParamId ACTION = "Events::Window::Input::ACTION"_hash; // action it took
}

namespace Events::Window::Scrolled // parameters for mouse scrolled event
{
	const ParamId XOFFSET = "Events::Window::Scrolled::XOFFSET"_hash; // scroll x offset
	const ParamId YOFFSET = "Events::Window::Scrolled::YOFFSET"_hash; // scroll y offset
}

namespace Events::Window::Mouse_Moved
{
	const ParamId XPOS = "Events::Window::Mouse_Moved::XPOS"_hash; 
	const ParamId YPOS = "Events::Window::Mouse_Moved::YPOS"_hash;
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
	const EventId CREATE_ACTOR = "Events::Physics::CREATE_ACTOR"_hash;
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

namespace Events::Physics::Create_Actor
{
	const ParamId ENTITY = "Events::Physics::Create_Actor::ENTITY"_hash;
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