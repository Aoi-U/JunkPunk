#pragma once

#include <functional>
#include <unordered_map>

#include "Core/Types.h"
#include "Event.h"

class EventManager
{
public:
	// usage: controller.addEventListener(Events::EventType::EVENT_NAME, [this](Event& e) { this->listenerFunction(e); });
	void AddListener(EventId id, std::function<void(Event&)> const& listener) // add a listener for the given event type
	{
		listeners[id].push_back(listener);
	}

	// send event to all listeners of that event type
	void SendEvent(Event& event)
	{
		uint32_t type = event.GetType();

		for (auto const& listener : listeners[type])
		{
			listener(event);
		}
	}

	// send event by id
	void SendEvent(EventId id)
	{
		Event event(id);

		for (auto const& listener : listeners[id])
		{
			listener(event);
		}
	}

private:
	std::unordered_map<EventId, std::list<std::function<void(Event&)>>> listeners;
};