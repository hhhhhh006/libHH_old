#ifndef ipc_param_traits_h__
#define ipc_param_traits_h__


// Our IPC system uses the following partially specialized header to define how
// a data type is read, written and logged in the IPC system.

namespace IPC {

template <class P> struct ParamTraits {
};

template <class P>
struct SimilarTypeTraits {
	typedef P Type;
};

}  // namespace IPC

#endif // ipc_param_traits_h__
