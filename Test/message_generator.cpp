

#define IPC_MESSAGE_IMPL
#include "message_define.h"

// Generate constructors.
#include "IPC/struct_constructor_macros.h"
#undef message_define_h__
#include "message_define.h"

// Generate destructors.
#include "IPC/struct_destructor_macros.h"
#undef message_define_h__
#include "message_define.h"

// Generate param traits write methods.
#include "IPC/param_traits_write_macros.h"
namespace IPC {
#undef message_define_h__
#include "message_define.h"
}

// Generate param traits read methods.
#include "IPC/param_traits_read_macros.h"
namespace IPC {
#undef message_define_h__
#include "message_define.h"
}

