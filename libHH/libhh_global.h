#ifndef LIBHH_GLOBAL_H
#define LIBHH_GLOBAL_H

#include <QtCore/qglobal.h>

#ifdef LIBHH_LIB
# define LIBHH_EXPORT Q_DECL_EXPORT
#else
# define LIBHH_EXPORT Q_DECL_IMPORT
#endif

#endif // LIBHH_GLOBAL_H
