#include "IPC/ipc_message_macros.h"
#include "IPC/ipc_message_start.h"
#include "IPC/param_traits_macros.h"

#include "logdata.h"
#include "message_traits.h"

#define IPC_MESSAGE_START TestMsgStart

IPC_STRUCT_TRAITS_BEGIN(Logdata)
	IPC_STRUCT_TRAITS_MEMBER(id)
	IPC_STRUCT_TRAITS_MEMBER(name)
	IPC_STRUCT_TRAITS_MEMBER(time)
IPC_STRUCT_TRAITS_END()

IPC_STRUCT_TRAITS_BEGIN(Loglive)
	IPC_STRUCT_TRAITS_PARENT(Logdata)
	IPC_STRUCT_TRAITS_MEMBER(flows)
	IPC_STRUCT_TRAITS_MEMBER(num)
IPC_STRUCT_TRAITS_END()


IPC_MESSAGE_CONTROL1(TestMsg_Text, std::string)

IPC_MESSAGE_CONTROL2(TestMsg_Hello, int, int)

IPC_MESSAGE_CONTROL1(TestMsg_Log, Logdata)

IPC_MESSAGE_CONTROL1(TestMsg_Live, Loglive)