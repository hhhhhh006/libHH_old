#include "base/at_exit.h"

#include <stddef.h>
#include <ostream>
#include <functional>

#include "base/callback.h"

namespace base {

// Keep a stack of registered AtExitManagers.  We always operate on the most
// recent, and we should never have more than one outside of testing (for a
// statically linked version of this library).  Testing may use the shadow
// version of the constructor, and if we are building a dynamic library we may
// end up with multiple AtExitManagers on the same process.  We don't protect
// this for thread-safe access, since it will only be modified in testing.
static AtExitManager* g_top_manager = nullptr;

AtExitManager::AtExitManager() : next_manager_(g_top_manager) {
// If multiple modules instantiate AtExitManagers they'll end up living in this
// module... they have to coexist.
  g_top_manager = this;
}

AtExitManager::~AtExitManager() {
  if (!g_top_manager) {
    return;
  }

  ProcessCallbacksNow();
  g_top_manager = next_manager_;
}

// static
void AtExitManager::RegisterCallback(AtExitCallbackType func, void* param) {
  base::Closure task = std::bind(func, param);
  RegisterTask(task);
}

// static
void AtExitManager::RegisterTask(base::Closure task) {
  if (!g_top_manager) {
    return;
  }

//  AutoLock lock(g_top_manager->lock_);
  g_top_manager->stack_.push(task);
}

// static
void AtExitManager::ProcessCallbacksNow() {
  if (!g_top_manager) {
    return;
  }

//  AutoLock lock(g_top_manager->lock_);

  while (!g_top_manager->stack_.empty()) {
    base::Closure task = g_top_manager->stack_.top();
    task.Run();
    g_top_manager->stack_.pop();
  }
}

AtExitManager::AtExitManager(bool shadow) : next_manager_(g_top_manager) {
  g_top_manager = this;
}

}  // namespace base
