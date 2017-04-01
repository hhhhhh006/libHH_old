#ifndef CALLBACK_H__
#define CALLBACK_H__

#include <functional>

namespace base
{

template <typename Sig>
class Callback;

template <typename R>
class CallBack
{
public:
	typedef std::function<R> InvokeFuncStorage;

	CallBack(InvokeFuncStorage func) {
		polymorphic_invoke_ = func;
	}

	void Run() const
	{
		polymorphic_invoke_();
	}

private:
	InvokeFuncStorage polymorphic_invoke_;
};


typedef CallBack<void(void)> Closure;

}

#endif // CALLBACK_H__
