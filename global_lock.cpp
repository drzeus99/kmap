#include "global_lock.h"

global_lock::global_lock()
{
    //on construction global_lock is in a unlocked state
}

global_lock::~global_lock()
{
	//lock.unlock(); //need to unlock the std::mutex to ensure
	//the behavior is defined for the global lock object
}

void global_lock::set_lock(bool setting)
{
	if (setting) {
		lock.lock();
	}
	else {
		lock.unlock();
	}
}

bool global_lock::try_lock()
{
	return lock.try_lock();
}
