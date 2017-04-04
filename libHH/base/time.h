#ifndef BASE_TIME_H__
#define BASE_TIME_H__

#include <stdint.h>
#include <QDateTime>

namespace base
{

typedef int64_t TimeTicks;
typedef int64_t TimeDelta;
typedef int64_t Time;

#define TimeTicksNow QDateTime::currentMSecsSinceEpoch()

}

#endif // BASE_TIME_H__
