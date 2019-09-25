#pragma once

class cDeltaTime {
public:
	cDeltaTime(float limit = 1.0f);
	~cDeltaTime() = default;

	void reset();
	void update();
	void IgnoreReset();
	void IgnoreBegin();
	void IgnoreEnd();
	float operator()();

private:
	std::chrono::time_point<std::chrono::system_clock> mStart;
	std::chrono::time_point<std::chrono::system_clock> mStartIgnore;
	float mElapsedTime;
	float mElapsedTimeIgnore;
	const float mLimit;
};