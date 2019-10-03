#include "cDeltaTime.h"

cDeltaTime::cDeltaTime(float limit) : mStart(), mElapsedTime(0.f), mFixedTime(-1.f), mLimit(limit) {
}
void cDeltaTime::reset() {
	mStart = std::chrono::system_clock::now();
	mElapsedTime = 0.f;
	mElapsedTimeIgnore = 0.0f;
}
void cDeltaTime::update() {
	auto end = std::chrono::system_clock::now();
	std::chrono::duration<float> elapsed = end - mStart;
	mStart = end;
	if (isElapsedFixedTime()) {
		mElapsedTime = min(mLimit, elapsed.count());
	} else {
		mElapsedTime += min(mLimit, elapsed.count());
	}
}
void cDeltaTime::IgnoreReset() {
	mElapsedTimeIgnore = 0.0f;
}
void cDeltaTime::IgnoreBegin() {
	mStartIgnore = std::chrono::system_clock::now();
}
void cDeltaTime::IgnoreEnd() {
	auto end = std::chrono::system_clock::now();
	std::chrono::duration<float> elapsed = end - mStartIgnore;
	mElapsedTimeIgnore += elapsed.count();
}
void cDeltaTime::setFiexedTime(float fixedTime) {
	mFixedTime = fixedTime;
}
bool cDeltaTime::isElapsedFixedTime() {
	return mFixedTime < 0 || mFixedTime <= (*this)();
}
float cDeltaTime::operator()() {
	return mElapsedTime - mElapsedTimeIgnore;
}