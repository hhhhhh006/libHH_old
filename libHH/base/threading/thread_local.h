#ifndef THREAD_LOCAL_H
#define THREAD_LOCAL_H

#include "base/base_export.h"
#include "base/macros.h"

#include <QThreadStorage>

//线程局部存储类，只能存储指针；
//线程退出时，会自动删除所有存储指针。

namespace base {

template <typename Type>
class ThreadLocalPointer : private QThreadStorage<Type*>
{
public:
	ThreadLocalPointer() {}
	~ThreadLocalPointer() {}

	Type* Get() {
		return localData();
	}

	//再次调用，会自动删除上次存储的指针
	void Set(Type* ptr) {
		setLocalData(ptr);
	}

	DISALLOW_COPY_AND_ASSIGN(ThreadLocalPointer);
};


class BASE_EXPORT ThreadLocalBoolean
{
public:
	ThreadLocalBoolean() {}
	~ThreadLocalBoolean() {}

	bool Get() {
		return tlp_.Get() != nullptr;
	}

	void Set(bool val) {
		tlp_.Set(val ? this : nullptr);
	}

private:
	ThreadLocalPointer<void> tlp_;

	DISALLOW_COPY_AND_ASSIGN(ThreadLocalBoolean);
};

}


#endif // THREAD_LOCAL_H
