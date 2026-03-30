#pragma once

struct Clock {
	double timerDuration = 0.0f; // how long you want to wait (1.0f = 1 second timer)
	double remaining = 0.0f; // how much time is left in the timer

	void start() {
		remaining = timerDuration; // on timer start, remaining time left is total duration
	}

	void start(double duration) { // set a custom timerDuration
		timerDuration = duration;
		remaining = duration;
	}

	void update(double dt) {
		if (remaining > 0)
			remaining -= dt;
	}

	bool activeTimer() const {
		// returns a boolean True if there is still an active timer, else False.
		return remaining > 0;
	}

	bool completedTimer() const {
		// returns a boolean True if the timer is completed
		return remaining <= 0;
	}

	void resetTimer() {
		// call this after the timer is finished
		remaining = 0.0f;
	}
};