#ifndef MESSAGE_DISPATCH_H_
#define MESSAGE_DISPATCH_H_

namespace IPC {


template <typename ObjT, typename Method, class A>
inline void DispatchToMethod(ObjT* obj, Method method, const std::tuple<A>& arg) 
{
	(obj->*method)(std::get<0>(arg));
}

template <typename ObjT, typename Method, class A, class B>
inline void DispatchToMethod(ObjT* obj, Method method, const std::tuple<A, B>& arg) 
{
	(obj->*method)(std::get<0>(arg), std::get<1>(arg));
}

template <typename ObjT, typename Method, class A, class B, class C>
inline void DispatchToMethod(ObjT* obj, Method method, const std::tuple<A, B, C>& arg) 
{
	(obj->*method)(std::get<0>(arg), std::get<1>(arg), std::get<2>(arg));
}

template <typename ObjT, typename Method, class A, class B, class C, class D>
inline void DispatchToMethod(ObjT* obj, Method method, const std::tuple<A, B, C, D>& arg) 
{
	(obj->*method)(std::get<0>(arg), std::get<1>(arg), std::get<2>(arg), std::get<3>(arg));
}

template <typename ObjT, typename Method, class A, class B, class C, class D, class E>
inline void DispatchToMethod(ObjT* obj, Method method, const std::tuple<A, B, C, D, E>& arg) 
{
	(obj->*method)(std::get<0>(arg), std::get<1>(arg), std::get<2>(arg), std::get<3>(arg), std::get<4>(arg));
}


}  // namespace IPC

#endif  // MESSAGE_DISPATCH_H_
