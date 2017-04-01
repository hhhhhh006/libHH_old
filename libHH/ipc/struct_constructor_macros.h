#ifndef STRUCT_CONSTRUCTOR_MACROS_H_
#define STRUCT_CONSTRUCTOR_MACROS_H_

// Null out all the macros that need nulling.
#include "ipc_message_null_macros.h"

// Set up so next include will generate constructors.
#undef IPC_STRUCT_BEGIN_WITH_PARENT
#undef IPC_STRUCT_MEMBER
#undef IPC_STRUCT_END
#define IPC_STRUCT_BEGIN_WITH_PARENT(struct_name, parent) \
	struct_name::struct_name() : parent()
#define IPC_STRUCT_MEMBER(type, name, ...) , name(__VA_ARGS__)
#define IPC_STRUCT_END() {}

#endif  // STRUCT_CONSTRUCTOR_MACROS_H_