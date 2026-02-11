#pragma once

struct Powerup {
	int type = 0;
	bool active = false;
	float duration = 5.0f;
	float elapsed = 0.0f;
};