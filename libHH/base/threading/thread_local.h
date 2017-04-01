#ifndef THREAD_LOCAL_H
#define THREAD_LOCAL_H

#include "base/base_export.h"
#include "base/macros.h"

#include <QThreadStorage>

//�ֲ߳̾��洢�ֻ࣬�ܴ洢ָ�룻
//�߳��˳�ʱ�����Զ�ɾ�����д洢ָ�롣

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

	//�ٴε��ã����Զ�ɾ���ϴδ洢��ָ��
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
