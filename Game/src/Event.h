#pragma once

#include <unordered_map>
#include <any>

#include "Core/Types.h"

class Event
{
public:
	Event() = delete;

	Event(EventId id) // create anevent with the given event id
		: id(id)
	{
	}

	template<typename T>
	void SetParam(EventId id, T value) // set a parameter for the event. the caller will use this to set event data
	{
		data[id] = value;
	}

	// usage: type value = event.GetParam<type>(Events::EventType::Param_Name::PARAM_ID);
	template<typename T>
	T GetParam(EventId id) // get a parameter for the event. the listener will use this to retrieve event data
	{
		return std::any_cast<T>(data[id]);
	}

	EventId GetType() const { return id; } // get the event type

private:
	EventId id;
	std::unordered_map<EventId, std::any> data;
};