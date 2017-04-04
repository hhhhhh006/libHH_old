#include "base/win/scoped_handle.h"

namespace {

bool CloseHandleWrapper(HANDLE handle) 
{
	if (!::CloseHandle(handle))
	  return false;  // CloseHandle failed.
	return true;
}

}  // namespace


namespace base {
namespace win {

// Static.
bool HandleTraits::CloseHandle(HANDLE handle) 
{
	return CloseHandleWrapper(handle);
}

}  // namespace win
}  // namespace base