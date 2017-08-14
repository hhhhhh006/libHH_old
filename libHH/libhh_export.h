#ifndef LIBHH_EXPORT_H_
#define LIBHH_EXPORT_H_

// ���Ե��� std ģ����ľ������
__pragma(warning(disable:4251))

#if defined(COMPONENT_BUILD)

#if defined(LIBHH_IMPLEMENTATION)
#define LIBHH_EXPORT __declspec(dllexport)
#else
#define LIBHH_EXPORT __declspec(dllimport)
#endif  // defined(LIBHH_IMPLEMENTATION)

#else  // defined(COMPONENT_BUILD)
#define LIBHH_EXPORT
#endif

#endif  // LIBHH_EXPORT_H_
