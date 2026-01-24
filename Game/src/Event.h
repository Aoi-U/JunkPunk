#pragma once

#include <unordered_map>
#include <any>

#include "Core/Types.h"

class Event
{
public:
	Event() = delete;

	Event(EventId id)
		: id(id)
	{
	}

	template<typename T>
	void SetParam(EventId id, T value)
	{
		data[id] = value;
	}

	template<typename T>
	T GetParam(EventId id)
	{
		return std::any_cast<T>(data[id]);
	}

	EventId GetType() const { return id; }

private:
	EventId id;
	std::unordered_map<EventId, std::any> data;
};