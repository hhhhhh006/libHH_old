

#define IPC_MESSAGE_IMPL
#include "message_traits.h"

// Generate constructors.
#include "IPC/struct_constructor_macros.h"
#undef MESSAGE_TRAITS_H
#include "message_traits.h"

// Generate destructors.
#include "IPC/struct_destructor_macros.h"
#undef MESSAGE_TRAITS_H
#include "message_traits.h"

// Generate param traits write methods.
#include "IPC/param_traits_write_macros.h"
namespace IPC {
	#undef MESSAGE_TRAITS_H
#include "message_traits.h"
}  // namespace IPC

// Generate param traits read methods.
#include "IPC/param_traits_read_macros.h"
namespace IPC {
	#undef MESSAGE_TRAITS_H
#include "message_traits.h"
}  // namespace IPC



