#include "ref_counted.h"

namespace base {

namespace subtle {


RefCountedThreadSafeBase::RefCountedThreadSafeBase() : ref_count_(0)
{

}

RefCountedThreadSafeBase::~RefCountedThreadSafeBase()
{

}

bool RefCountedThreadSafeBase::HasOneRef() const
{
    return AtomicRefCountIsOne(&const_cast<RefCountedThreadSafeBase*>(this)->ref_count_);
}

void RefCountedThreadSafeBase::AddRef() const
{
    AtomicRefCountInc(&ref_count_);
}

bool RefCountedThreadSafeBase::Release() const
{
    if (!AtomicRefCountDec(&ref_count_))
        return true;

    return false;
}

} // namespace subtle

} // namespace base
