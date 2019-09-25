#include "sTime.h"

SINGLETON_INSTANCE(sTime);

sTime::sTime() : cDeltaTime() {

}
sTime::~sTime() {

}
float sTime::getDeltaTime() {
	return operator()();
}