#ifndef BASE_LOCK_H
#define BASE_LOCK_H

#include "base/base_export.h"
#include "base/macros.h"

#include <QMutex>

namespace base {

class BASE_EXPORT Lock
{
public:
	Lock() : lock_() {}
	~Lock() {}

	void Acquire() { lock_.lock(); }
	void Release() { lock_.unlock(); }

	// If the lock is not held, take it and return true.
	// If the lock is already held by another thread, immediately return false
	bool Try() { return lock_.tryLock(); }

private:
	QMutex lock_;

	DISALLOW_COPY_AND_ASSIGN(Lock);
};


class BASE_EXPORT AutoLock
{
public:
	explicit AutoLock(Lock &lock) : lock_(lock) {
		lock_.Acquire();
	}

	~AutoLock() {
		lock_.Release();
	}

private:
	Lock &lock_;

	DISALLOW_COPY_AND_ASSIGN(AutoLock);
};


class BASE_EXPORT AutoUnlock 
{
public:
	explicit AutoUnlock(Lock &lock) : lock_(lock) {
		lock_.Release();
	}

	~AutoUnlock() {
		lock_.Acquire();
	}

private:
	Lock& lock_;

	DISALLOW_COPY_AND_ASSIGN(AutoUnlock);
};

}
#endif // BASE_LOCK_H
