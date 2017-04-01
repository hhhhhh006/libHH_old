#undef IPC_MESSAGE_EXPORT
#define IPC_MESSAGE_EXPORT

#ifndef IPC_MESSAGE_MACROS_H_
#define IPC_MESSAGE_MACROS_H_

#include <stdint.h>
#include <tuple>

#include "ipc_message_utils.h"
#include "param_traits_macros.h"
#include "message_dispatch.h"

#if defined(IPC_MESSAGE_IMPL)
#include "ipc_message_utils_impl.h"
#endif


// Convenience macro for defining structs without inheritance. Should not need
// to be subsequently redefined.
#define IPC_STRUCT_BEGIN(struct_name) \
	IPC_STRUCT_BEGIN_WITH_PARENT(struct_name, IPC::NoParams)

// Macros for defining structs. Will be subsequently redefined.
#define IPC_STRUCT_BEGIN_WITH_PARENT(struct_name, parent) \
	struct struct_name; \
	IPC_STRUCT_TRAITS_BEGIN(struct_name) \
	IPC_STRUCT_TRAITS_END() \
	struct IPC_MESSAGE_EXPORT struct_name : parent { \
	struct_name(); \
	~struct_name();
// Optional variadic parameters specify the default value for this struct
// member. They are passed through to the constructor for |type|.
#define IPC_STRUCT_MEMBER(type, name, ...) type name;
#define IPC_STRUCT_END() };


// Message macros collect specific numbers of arguments and funnel them into
// the common message generation macro.  These should never be redefined.
#define IPC_MESSAGE_CONTROL0(msg_class) \
	IPC_MESSAGE_DECL(EMPTY, CONTROL, msg_class, 0, 0, (), ())

#define IPC_MESSAGE_CONTROL1(msg_class, type1) \
	IPC_MESSAGE_DECL(ASYNC, CONTROL, msg_class, 1, 0, (type1), ())

#define IPC_MESSAGE_CONTROL2(msg_class, type1, type2) \
	IPC_MESSAGE_DECL(ASYNC, CONTROL, msg_class, 2, 0, (type1, type2), ())

#define IPC_MESSAGE_CONTROL3(msg_class, type1, type2, type3) \
	IPC_MESSAGE_DECL(ASYNC, CONTROL, msg_class, 3, 0, (type1, type2, type3), ())

#define IPC_MESSAGE_CONTROL4(msg_class, type1, type2, type3, type4) \
	IPC_MESSAGE_DECL(ASYNC, CONTROL, msg_class, 4, 0, (type1, type2, type3, type4), ())

#define IPC_MESSAGE_CONTROL5(msg_class, type1, type2, type3, type4, type5) \
	IPC_MESSAGE_DECL(ASYNC, CONTROL, msg_class, 5, 0, (type1, type2, type3, type4, type5), ())

#define IPC_MESSAGE_ROUTED0(msg_class) \
	IPC_MESSAGE_DECL(EMPTY, ROUTED, msg_class, 0, 0, (), ())

#define IPC_MESSAGE_ROUTED1(msg_class, type1) \
	IPC_MESSAGE_DECL(ASYNC, ROUTED, msg_class, 1, 0, (type1), ())

#define IPC_MESSAGE_ROUTED2(msg_class, type1, type2) \
	IPC_MESSAGE_DECL(ASYNC, ROUTED, msg_class, 2, 0, (type1, type2), ())

#define IPC_MESSAGE_ROUTED3(msg_class, type1, type2, type3) \
	IPC_MESSAGE_DECL(ASYNC, ROUTED, msg_class, 3, 0, (type1, type2, type3), ())

#define IPC_MESSAGE_ROUTED4(msg_class, type1, type2, type3, type4) \
	IPC_MESSAGE_DECL(ASYNC, ROUTED, msg_class, 4, 0, (type1, type2, type3, type4), ())

#define IPC_MESSAGE_ROUTED5(msg_class, type1, type2, type3, type4, type5) \
	IPC_MESSAGE_DECL(ASYNC, ROUTED, msg_class, 5, 0, (type1, type2, type3, type4, type5), ())


// The following macros define the common set of methods provided by ASYNC
// message classes.
// This macro is for all the async IPCs that don't pass an extra parameter using
// IPC_BEGIN_MESSAGE_MAP_WITH_PARAM.
#define IPC_ASYNC_MESSAGE_METHODS_GENERIC                                     \
  template<class T, class S, class P, class Method>                           \
  static bool Dispatch(const Message* msg, T* obj, S* sender, P* parameter,   \
	                   Method func) {                                         \
	Schema::Param p;                                                          \
	if (Read(msg, &p)) {                                                      \
	  IPC::DispatchToMethod(obj, func, p);                                    \
	  return true;                                                            \
	}                                                                         \
	return false;                                                             \
  }

// The following macros are for for async IPCs which have a dispatcher with an
// extra parameter specified using IPC_BEGIN_MESSAGE_MAP_WITH_PARAM.
#define IPC_ASYNC_MESSAGE_METHODS_1                                           \
  IPC_ASYNC_MESSAGE_METHODS_GENERIC                                           \
  template<class T, class S, class P, typename TA>                            \
  static bool Dispatch(const Message* msg, T* obj, S* sender, P* parameter,   \
	                   void (T::*func)(P*, TA)) {                             \
	Schema::Param p;                                                          \
	if (Read(msg, &p)) {                                                      \
	  (obj->*func)(parameter, std::get<0>(p));                                \
	  return true;                                                            \
	}                                                                         \
	return false;                                                             \
  }
#define IPC_ASYNC_MESSAGE_METHODS_2                                           \
  IPC_ASYNC_MESSAGE_METHODS_GENERIC                                           \
  template<class T, class S, class P, typename TA, typename TB>               \
  static bool Dispatch(const Message* msg, T* obj, S* sender, P* parameter,   \
	                   void (T::*func)(P*, TA, TB)) {                         \
	Schema::Param p;                                                          \
	if (Read(msg, &p)) {                                                      \
	  (obj->*func)(parameter, std::get<0>(p), std::get<1>(p));                \
	  return true;                                                            \
	}                                                                         \
	return false;                                                             \
  }
#define IPC_ASYNC_MESSAGE_METHODS_3                                           \
  IPC_ASYNC_MESSAGE_METHODS_GENERIC                                           \
  template<class T, class S, class P, typename TA, typename TB, typename TC>  \
  static bool Dispatch(const Message* msg, T* obj, S* sender, P* parameter,   \
	                   void (T::*func)(P*, TA, TB, TC)) {                     \
	Schema::Param p;                                                          \
	if (Read(msg, &p)) {                                                      \
	  (obj->*func)(parameter, std::get<0>(p), std::get<1>(p),                 \
	               std::get<2>(p));                                           \
	  return true;                                                            \
	}                                                                         \
	return false;                                                             \
  }
#define IPC_ASYNC_MESSAGE_METHODS_4                                           \
  IPC_ASYNC_MESSAGE_METHODS_GENERIC                                           \
  template<class T, class S, class P, typename TA, typename TB, typename TC,  \
	       typename TD>                                                       \
  static bool Dispatch(const Message* msg, T* obj, S* sender, P* parameter,   \
	                   void (T::*func)(P*, TA, TB, TC, TD)) {                 \
	Schema::Param p;                                                          \
	if (Read(msg, &p)) {                                                      \
	  (obj->*func)(parameter, std::get<0>(p), std::get<1>(p),                 \
	               std::get<2>(p), std::get<3>(p));                           \
	  return true;                                                            \
	}                                                                         \
	return false;                                                             \
  }
#define IPC_ASYNC_MESSAGE_METHODS_5                                           \
  IPC_ASYNC_MESSAGE_METHODS_GENERIC                                           \
  template<class T, class S, class P, typename TA, typename TB, typename TC,  \
	       typename TD, typename TE>                                          \
  static bool Dispatch(const Message* msg, T* obj, S* sender, P* parameter,   \
	                   void (T::*func)(P*, TA, TB, TC, TD, TE)) {             \
	Schema::Param p;                                                          \
	if (Read(msg, &p)) {                                                      \
	  (obj->*func)(parameter, std::get<0>(p), std::get<1>(p),                 \
	               std::get<2>(p), std::get<3>(p), std::get<4>(p));           \
	  return true;                                                            \
	}                                                                         \
	return false;                                                             \
  }


#define IPC_MESSAGE_DECL(sync, kind, msg_class,                               \
	                     in_cnt, out_cnt, in_list, out_list)                  \
  IPC_##sync##_##kind##_DECL(msg_class, in_cnt, out_cnt, in_list, out_list)   \
  IPC_MESSAGE_EXTRA(sync, kind, msg_class, in_cnt, out_cnt, in_list, out_list)

#define IPC_EMPTY_CONTROL_DECL(msg_class, in_cnt, out_cnt, in_list, out_list) \
  class IPC_MESSAGE_EXPORT msg_class : public IPC::Message {                  \
   public:                                                                    \
	typedef IPC::Message Schema;                                              \
	enum { ID = IPC_MESSAGE_ID() };                                           \
	msg_class() : IPC::Message(MSG_ROUTING_CONTROL, ID) {}                    \
};

#define IPC_EMPTY_ROUTED_DECL(msg_class, in_cnt, out_cnt, in_list, out_list)  \
  class IPC_MESSAGE_EXPORT msg_class : public IPC::Message {                  \
   public:                                                                    \
	typedef IPC::Message Schema;                                              \
	enum { ID = IPC_MESSAGE_ID() };                                           \
	msg_class(int32_t routing_id) : IPC::Message(routing_id, ID) {}           \
};

#define IPC_ASYNC_CONTROL_DECL(msg_class, in_cnt, out_cnt, in_list, out_list) \
  class IPC_MESSAGE_EXPORT msg_class : public IPC::Message {                  \
   public:                                                                    \
	typedef IPC::MessageSchema<IPC_TUPLE_IN_##in_cnt in_list> Schema;         \
	typedef Schema::Param Param;                                              \
	enum { ID = IPC_MESSAGE_ID() };                                           \
	msg_class(IPC_TYPE_IN_##in_cnt in_list);                                  \
	~msg_class();                                                             \
	static bool Read(const Message* msg, Schema::Param* p);                   \
	IPC_ASYNC_MESSAGE_METHODS_##in_cnt                                        \
};

#define IPC_ASYNC_ROUTED_DECL(msg_class, in_cnt, out_cnt, in_list, out_list)  \
  class IPC_MESSAGE_EXPORT msg_class : public IPC::Message {                  \
   public:                                                                    \
	typedef IPC::MessageSchema<IPC_TUPLE_IN_##in_cnt in_list> Schema;         \
	typedef Schema::Param Param;                                              \
	enum { ID = IPC_MESSAGE_ID() };                                           \
	msg_class(int32_t routing_id IPC_COMMA_##in_cnt                           \
	          IPC_TYPE_IN_##in_cnt in_list);                                  \
	~msg_class();                                                             \
	static bool Read(const Message* msg, Schema::Param* p);                   \
	IPC_ASYNC_MESSAGE_METHODS_##in_cnt                                        \
};


#if defined(IPC_MESSAGE_IMPL)

// "Implementation" inclusion produces constructors, destructors, and
// logging functions, except for the no-arg special cases, where the
// implementation occurs in the declaration, and there is no special
// logging function.
#define IPC_MESSAGE_EXTRA(sync, kind, msg_class,                              \
	                      in_cnt, out_cnt, in_list, out_list)                 \
  IPC_##sync##_##kind##_IMPL(msg_class, in_cnt, out_cnt, in_list, out_list)   

#define IPC_EMPTY_CONTROL_IMPL(msg_class, in_cnt, out_cnt, in_list, out_list)
#define IPC_EMPTY_ROUTED_IMPL(msg_class, in_cnt, out_cnt, in_list, out_list)

#define IPC_ASYNC_CONTROL_IMPL(msg_class, in_cnt, out_cnt, in_list, out_list) \
  msg_class::msg_class(IPC_TYPE_IN_##in_cnt in_list) :                        \
	  IPC::Message(MSG_ROUTING_CONTROL, ID) {				                  \
	    Schema::Write(this, IPC_NAME_IN_##in_cnt in_list);                    \
	  }                                                                       \
  msg_class::~msg_class() {}                                                  \
  bool msg_class::Read(const Message* msg, Schema::Param* p) {                \
	return Schema::Read(msg, p);                                              \
  }

#define IPC_ASYNC_ROUTED_IMPL(msg_class, in_cnt, out_cnt, in_list, out_list)  \
  msg_class::msg_class(int32_t routing_id IPC_COMMA_##in_cnt                  \
	                   IPC_TYPE_IN_##in_cnt in_list) :                        \
	  IPC::Message(routing_id, ID)                  {                         \
	    Schema::Write(this, IPC_NAME_IN_##in_cnt in_list);                    \
	  }                                                                       \
  msg_class::~msg_class() {}                                                  \
  bool msg_class::Read(const Message* msg, Schema::Param* p) {                \
	return Schema::Read(msg, p);                                              \
  }

#else

// Normal inclusion produces nothing extra.
#define IPC_MESSAGE_EXTRA(sync, kind, msg_class, in_cnt, out_cnt, in_list, out_list)

#endif // defined(IPC_MESSAGE_IMPL)


// Handle variable sized argument lists.  These are usually invoked by token
// pasting against the argument counts.
#define IPC_TYPE_IN_0()
#define IPC_TYPE_IN_1(t1)                   const t1& arg1
#define IPC_TYPE_IN_2(t1, t2)               const t1& arg1, const t2& arg2
#define IPC_TYPE_IN_3(t1, t2, t3)           const t1& arg1, const t2& arg2, const t3& arg3
#define IPC_TYPE_IN_4(t1, t2, t3, t4)       const t1& arg1, const t2& arg2, const t3& arg3, const t4& arg4
#define IPC_TYPE_IN_5(t1, t2, t3, t4, t5)   const t1& arg1, const t2& arg2, const t3& arg3, const t4& arg4, const t5& arg5

#define IPC_TYPE_OUT_0()
#define IPC_TYPE_OUT_1(t1)                  t1* arg6
#define IPC_TYPE_OUT_2(t1, t2)              t1* arg6, t2* arg7
#define IPC_TYPE_OUT_3(t1, t2, t3)          t1* arg6, t2* arg7, t3* arg8
#define IPC_TYPE_OUT_4(t1, t2, t3, t4)      t1* arg6, t2* arg7, t3* arg8, t4* arg9

#define IPC_TUPLE_IN_0()                    std::tuple<>
#define IPC_TUPLE_IN_1(t1)                  std::tuple<t1>
#define IPC_TUPLE_IN_2(t1, t2)              std::tuple<t1, t2>
#define IPC_TUPLE_IN_3(t1, t2, t3)          std::tuple<t1, t2, t3>
#define IPC_TUPLE_IN_4(t1, t2, t3, t4)      std::tuple<t1, t2, t3, t4>
#define IPC_TUPLE_IN_5(t1, t2, t3, t4, t5)  std::tuple<t1, t2, t3, t4, t5>

#define IPC_TUPLE_OUT_0()                   std::tuple<>
#define IPC_TUPLE_OUT_1(t1)                 std::tuple<t1&>
#define IPC_TUPLE_OUT_2(t1, t2)             std::tuple<t1&, t2&>
#define IPC_TUPLE_OUT_3(t1, t2, t3)         std::tuple<t1&, t2&, t3&>
#define IPC_TUPLE_OUT_4(t1, t2, t3, t4)     std::tuple<t1&, t2&, t3&, t4&>

#define IPC_NAME_IN_0()                     std::make_tuple()
#define IPC_NAME_IN_1(t1)                   std::make_tuple(arg1)
#define IPC_NAME_IN_2(t1, t2)               std::make_tuple(arg1, arg2)
#define IPC_NAME_IN_3(t1, t2, t3)           std::make_tuple(arg1, arg2, arg3)
#define IPC_NAME_IN_4(t1, t2, t3, t4)       std::make_tuple(arg1, arg2, arg3, arg4)
#define IPC_NAME_IN_5(t1, t2, t3, t4, t5)   std::make_tuple(arg1, arg2, arg3, arg4, arg5)

#define IPC_NAME_OUT_0()                    std::make_tuple()
#define IPC_NAME_OUT_1(t1)                  std::make_tuple(*arg6)
#define IPC_NAME_OUT_2(t1, t2)              std::make_tuple(*arg6, *arg7)
#define IPC_NAME_OUT_3(t1, t2, t3)          std::make_tuple(*arg6, *arg7, *arg8)
#define IPC_NAME_OUT_4(t1, t2, t3, t4)      std::make_tuple(*arg6, *arg7, *arg8, *arg9)

// There are places where the syntax requires a comma if there are input args,
// if there are input args and output args, or if there are input args or
// output args.  These macros allow generation of the comma as needed; invoke
// by token pasting against the argument counts.
#define IPC_COMMA_0
#define IPC_COMMA_1 ,
#define IPC_COMMA_2 ,
#define IPC_COMMA_3 ,
#define IPC_COMMA_4 ,
#define IPC_COMMA_5 ,

#define IPC_COMMA_AND_0(x)
#define IPC_COMMA_AND_1(x) x
#define IPC_COMMA_AND_2(x) x
#define IPC_COMMA_AND_3(x) x
#define IPC_COMMA_AND_4(x) x
#define IPC_COMMA_AND_5(x) x

#define IPC_COMMA_OR_0(x) x
#define IPC_COMMA_OR_1(x) ,
#define IPC_COMMA_OR_2(x) ,
#define IPC_COMMA_OR_3(x) ,
#define IPC_COMMA_OR_4(x) ,
#define IPC_COMMA_OR_5(x) ,


// Message IDs
// Note: we currently use __LINE__ to give unique IDs to messages within
// a file.  They're globally unique since each file defines its own
// IPC_MESSAGE_START.
#define IPC_MESSAGE_ID() ((IPC_MESSAGE_START << 16) + __LINE__)
#define IPC_MESSAGE_ID_CLASS(id) ((id) >> 16)
#define IPC_MESSAGE_ID_LINE(id) ((id) & 0xffff)


#define IPC_BEGIN_MESSAGE_MAP(class_name, msg) \
  { \
    typedef class_name _IpcMessageHandlerClass; \
    void* param__ = NULL; \
    const IPC::Message& ipc_message__ = msg; \
    switch (ipc_message__.type()) {

#define IPC_BEGIN_MESSAGE_MAP_WITH_PARAM(class_name, msg, param)  \
  {                                                               \
    typedef class_name _IpcMessageHandlerClass;                   \
    decltype(param) param__ = param;                              \
    const IPC::Message& ipc_message__ = msg;                      \
    switch (ipc_message__.type()) {

#define IPC_MESSAGE_FORWARD(msg_class, obj, member_func)                       \
    case msg_class::ID: {                                                      \
        msg_class::Dispatch(&ipc_message__, obj, this, param__,                \
                            &member_func);                                     \
	  }                                                                        \
	  break;

#define IPC_MESSAGE_HANDLER(msg_class, member_func) \
	IPC_MESSAGE_FORWARD(msg_class, this, _IpcMessageHandlerClass::member_func)


#define IPC_MESSAGE_UNHANDLED(code)                                            \
    default: {                                                                 \
        code;                                                                  \
	  }                                                                        \
	  break;

#define IPC_END_MESSAGE_MAP() \
  } \
}


// This corresponds to an enum value from IPCMessageStart.
#define IPC_MESSAGE_CLASS(message) \
	IPC_MESSAGE_ID_CLASS(message.type())


#undef IPC_MESSAGE_START

#endif  // IPC_MESSAGE_MACROS_H_