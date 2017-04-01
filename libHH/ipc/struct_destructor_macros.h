#ifndef STRUCT_DESTRUCTOR_MACROS_H_
#define STRUCT_DESTRUCTOR_MACROS_H_

// Null out all the macros that need nulling.
#include "ipc_message_null_macros.h"

// Set up so next include will generate destructors.
#undef IPC_STRUCT_BEGIN_WITH_PARENT
#define IPC_STRUCT_BEGIN_WITH_PARENT(struct_name, parent) \
	struct_name::~struct_name() {}

#endif  // STRUCT_DESTRUCTOR_MACROS_H_