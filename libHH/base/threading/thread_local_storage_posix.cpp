#include "base/threading/thread_local_storage.h"


namespace base {

namespace internal {

bool PlatformThreadLocalStorage::AllocTLS(TLSKey* key) {
  return !pthread_key_create(key,
      base::internal::PlatformThreadLocalStorage::OnThreadExit);
}

void PlatformThreadLocalStorage::FreeTLS(TLSKey key) {
  int ret = pthread_key_delete(key);
}

void* PlatformThreadLocalStorage::GetTLSValue(TLSKey key) {
  return pthread_getspecific(key);
}

void PlatformThreadLocalStorage::SetTLSValue(TLSKey key, void* value) {
  int ret = pthread_setspecific(key, value);
}

}  // namespace internal

}  // namespace base
