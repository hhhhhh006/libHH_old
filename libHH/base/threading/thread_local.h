#ifndef THREAD_LOCAL_H
#define THREAD_LOCAL_H

#include "base/base_export.h"
#include "base/macros.h"
#include "base/threading/thread_local_storage.h"

// 线程退出不删除指针内存
// 自己管理指针内存的释放

namespace base {

template <typename Type>
class ThreadLocalPointer 
{
public:
    ThreadLocalPointer() {}
    ~ThreadLocalPointer() {}

    Type* Get() {
        return static_cast<Type*>(slot_.Get());
    }

    void Set(Type* ptr) {
        slot_.Set(const_cast<void*>(static_cast<const void*>(ptr)));
    }

private:
    ThreadLocalStorage::Slot slot_;

    DISALLOW_COPY_AND_ASSIGN(ThreadLocalPointer<Type>);
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
