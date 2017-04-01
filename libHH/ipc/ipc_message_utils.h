#ifndef IPC_MESSAGE_UTILS_H__
#define IPC_MESSAGE_UTILS_H__

#include <vector>
#include <map>
#include <set>
#include <tuple>

#include "ipc_param_traits.h"
#include "ipc_message.h"

namespace IPC {


// -----------------------------------------------------------------------------
// How we send IPC message logs across channels.
struct LogData {
	LogData();
	~LogData();

	std::string channel;
	int32_t routing_id;
	uint32_t type;
	std::string message_name;
	std::string params;
};

template <>
struct ParamTraits<LogData> {
	typedef LogData param_type;
	static void Write(Message* m, const param_type& p);
	static bool Read(const Message* m,
					 base::PickleIterator* iter,
					 param_type* r);
};


// A dummy struct to place first just to allow leading commas for all
// members in the macro-generated constructor initializer lists.
struct NoParams {
};

template <class P>
static inline void WriteParam(Message* m, const P& p) {
	typedef typename SimilarTypeTraits<P>::Type Type;
	ParamTraits<Type>::Write(m, static_cast<const Type& >(p));
}

template <class P>
static inline bool ReadParam(const Message* m, base::PickleIterator* iter, P* p) {
	typedef typename SimilarTypeTraits<P>::Type Type;
	return ParamTraits<Type>::Read(m, iter, reinterpret_cast<Type* >(p));
}


// Primitive ParamTraits -------------------------------------------------------

template <>
struct IPC_EXPORT ParamTraits<bool> {
	typedef bool param_type;
	static void Write(Message* m, const param_type& p) {
		m->WriteBool(p);
	}
	static bool Read(const Message* m,
					 base::PickleIterator* iter,
					 param_type* r) {
		return iter->ReadBool(r);
	}
};

template <>
struct IPC_EXPORT ParamTraits<int> {
	typedef int param_type;
	static void Write(Message* m, const param_type& p) {
		m->WriteInt(p);
	}
	static bool Read(const Message* m,
					 base::PickleIterator* iter,
					 param_type* r) {
		return iter->ReadInt(r);
	}
};

template <>
struct IPC_EXPORT ParamTraits<unsigned int> {
	typedef unsigned int param_type;
	static void Write(Message* m, const param_type& p) {
		m->WriteInt(p);
	}
	static bool Read(const Message* m,
					 base::PickleIterator* iter,
					 param_type* r) {
		return iter->ReadInt(reinterpret_cast<int*>(r));
	}
};

template <>
struct IPC_EXPORT ParamTraits<signed char> {
	typedef signed char param_type;
	static void Write(Message* m, const param_type& p);
	static bool Read(const Message* m, base::PickleIterator* iter, param_type* r);
};

template <>
struct IPC_EXPORT ParamTraits<unsigned char> {
	typedef unsigned char param_type;
	static void Write(Message* m, const param_type& p);
	static bool Read(const Message* m, base::PickleIterator* iter, param_type* r);
};

template <>
struct IPC_EXPORT ParamTraits<long long> {
	typedef long long param_type;
	static void Write(Message* m, const param_type& p) {
		m->WriteInt64(static_cast<int64_t>(p));
	}
	static bool Read(const Message* m,
					 base::PickleIterator* iter,
					 param_type* r) {
		return iter->ReadInt64(reinterpret_cast<int64_t*>(r));
	}
};

template <>
struct IPC_EXPORT ParamTraits<unsigned long long> {
	typedef unsigned long long param_type;
	static void Write(Message* m, const param_type& p) {
		m->WriteInt64(p);
	}
	static bool Read(const Message* m,
					 base::PickleIterator* iter,
					 param_type* r) {
		return iter->ReadInt64(reinterpret_cast<int64_t*>(r));
	}
};

// Note that the IPC layer doesn't sanitize NaNs and +/- INF values.  Clients
// should be sure to check the sanity of these values after receiving them over
// IPC.
template <>
struct IPC_EXPORT ParamTraits<float> {
	typedef float param_type;
	static void Write(Message* m, const param_type& p) {
		m->WriteFloat(p);
	}
	static bool Read(const Message* m,
					 base::PickleIterator* iter,
					 param_type* r) {
		return iter->ReadFloat(r);
	}
};

template <>
struct IPC_EXPORT ParamTraits<double> {
	typedef double param_type;
	static void Write(Message* m, const param_type& p);
	static bool Read(const Message* m,
					 base::PickleIterator* iter,
					 param_type* r);
};


// IPC types ParamTraits -------------------------------------------------------

template <>
struct IPC_EXPORT ParamTraits<Message> {
	static void Write(Message* m, const Message& p);
	static bool Read(const Message* m,
					 base::PickleIterator* iter,
					 Message* r);
};

// STL ParamTraits -------------------------------------------------------------

template <>
struct IPC_EXPORT ParamTraits<std::string> {
	typedef std::string param_type;
	static void Write(Message* m, const param_type& p) {
		m->WriteString(p);
	}
	static bool Read(const Message* m,
					 base::PickleIterator* iter,
					 param_type* r) {
		return iter->ReadString(r);
	}
};

template <>
struct IPC_EXPORT ParamTraits<std::wstring> {
	typedef std::wstring param_type;
	static void Write(Message* m, const param_type& p) {
		m->WriteString16(p);
	}
	static bool Read(const Message* m,
					 base::PickleIterator* iter,
					 param_type* r) {
		return iter->ReadString16(r);
	}
};

template <>
struct IPC_EXPORT ParamTraits<std::vector<char> > {
	typedef std::vector<char> param_type;
	static void Write(Message* m, const param_type& p);
	static bool Read(const Message*,
					 base::PickleIterator* iter,
					 param_type* r);
};

template <>
struct IPC_EXPORT ParamTraits<std::vector<unsigned char> > {
	typedef std::vector<unsigned char> param_type;
	static void Write(Message* m, const param_type& p);
	static bool Read(const Message* m,
					 base::PickleIterator* iter,
					 param_type* r);
};

template <>
struct IPC_EXPORT ParamTraits<std::vector<bool> > {
	typedef std::vector<bool> param_type;
	static void Write(Message* m, const param_type& p);
	static bool Read(const Message* m,
					 base::PickleIterator* iter,
					 param_type* r);
};

template <class P>
struct IPC_EXPORT ParamTraits<std::vector<P> > {
	typedef std::vector<P> param_type;
	static void Write(Message* m, const param_type& p) {
		WriteParam(m, static_cast<int>(p.size()));
		for (size_t i = 0; i < p.size(); i++)
			WriteParam(m, p[i]);
	}
	static bool Read(const Message* m,
					 base::PickleIterator* iter,
					 param_type* r) {
		int size;
		// ReadLength() checks for < 0 itself.
		if (!iter->ReadLength(&size))
			return false;
		// Resizing beforehand is not safe, see BUG 1006367 for details.
		if (INT_MAX / sizeof(P) <= static_cast<size_t>(size))
			return false;
		r->resize(size);
		for (int i = 0; i < size; i++) {
			if (!ReadParam(m, iter, &(*r)[i]))
				return false;
		}
		return true;
	}
};

template <class P>
struct IPC_EXPORT ParamTraits<std::set<P> > {
	typedef std::set<P> param_type;
	static void Write(Message* m, const param_type& p) {
		WriteParam(m, static_cast<int>(p.size()));
		typename param_type::const_iterator iter;
		for (iter = p.begin(); iter != p.end(); ++iter)
			WriteParam(m, *iter);
	}
	static bool Read(const Message* m,
					 base::PickleIterator* iter,
					 param_type* r) {
		int size;
		if (!iter->ReadLength(&size))
			return false;
		for (int i = 0; i < size; ++i) {
			P item;
			if (!ReadParam(m, iter, &item))
				return false;
			r->insert(item);
		}
		return true;
	}
};

template <class K, class V, class C, class A>
struct IPC_EXPORT ParamTraits<std::map<K, V, C, A> > {
	typedef std::map<K, V, C, A> param_type;
	static void Write(Message* m, const param_type& p) {
		WriteParam(m, static_cast<int>(p.size()));
		typename param_type::const_iterator iter;
		for (iter = p.begin(); iter != p.end(); ++iter) {
			WriteParam(m, iter->first);
			WriteParam(m, iter->second);
		}
	}
	static bool Read(const Message* m,
					 base::PickleIterator* iter,
					 param_type* r) {
		int size;
		if (!ReadParam(m, iter, &size) || size < 0)
			return false;
		for (int i = 0; i < size; ++i) {
			K k;
			if (!ReadParam(m, iter, &k))
				return false;
			V& value = (*r)[k];
			if (!ReadParam(m, iter, &value))
				return false;
		}
		return true;
	}

};

template <class A, class B>
struct IPC_EXPORT ParamTraits<std::pair<A, B> > {
	typedef std::pair<A, B> param_type;
	static void Write(Message* m, const param_type& p) {
		WriteParam(m, p.first);
		WriteParam(m, p.second);
	}
	static bool Read(const Message* m,
					 base::PickleIterator* iter,
					 param_type* r) {
		return ReadParam(m, iter, &r->first) && ReadParam(m, iter, &r->second);
	}
};

template <>
struct ParamTraits<std::tuple<>> {
	typedef std::tuple<> param_type;
	static void Write(Message* m, const param_type& p) {
	}
	static bool Read(const Message* m,
					 base::PickleIterator* iter,
		             param_type* r) {
		return true;
	}
};

template <class A>
struct ParamTraits<std::tuple<A>> {
	typedef std::tuple<A> param_type;
	static void Write(Message* m, const param_type& p) {
		WriteParam(m, std::get<0>(p));
	}
	static bool Read(const Message* m,
					 base::PickleIterator* iter,
					 param_type* r) {
		return ReadParam(m, iter, &std::get<0>(*r));
	}
};

template <class A, class B>
struct ParamTraits<std::tuple<A, B>> {
	typedef std::tuple<A, B> param_type;
	static void Write(Message* m, const param_type& p) {
		WriteParam(m, std::get<0>(p));
		WriteParam(m, std::get<1>(p));
	}
	static bool Read(const Message* m,
					 base::PickleIterator* iter,
					 param_type* r) {
		return (ReadParam(m, iter, &std::get<0>(*r)) &&
				ReadParam(m, iter, &std::get<1>(*r)));
	}
};

template <class A, class B, class C>
struct ParamTraits<std::tuple<A, B, C>> {
	typedef std::tuple<A, B, C> param_type;
	static void Write(Message* m, const param_type& p) {
		WriteParam(m, std::get<0>(p));
		WriteParam(m, std::get<1>(p));
		WriteParam(m, std::get<2>(p));
	}
	static bool Read(const Message* m,
					 base::PickleIterator* iter,
					 param_type* r) {
		return (ReadParam(m, iter, &std::get<0>(*r)) &&
				ReadParam(m, iter, &std::get<1>(*r)) &&
				ReadParam(m, iter, &std::get<2>(*r)));
	}
};

template <class A, class B, class C, class D>
struct ParamTraits<std::tuple<A, B, C, D>> {
	typedef std::tuple<A, B, C, D> param_type;
	static void Write(Message* m, const param_type& p) {
		WriteParam(m, std::get<0>(p));
		WriteParam(m, std::get<1>(p));
		WriteParam(m, std::get<2>(p));
		WriteParam(m, std::get<3>(p));
	}
	static bool Read(const Message* m,
					 base::PickleIterator* iter,
					 param_type* r) {
		return (ReadParam(m, iter, &std::get<0>(*r)) &&
				ReadParam(m, iter, &std::get<1>(*r)) &&
				ReadParam(m, iter, &std::get<2>(*r)) &&
				ReadParam(m, iter, &std::get<3>(*r)));
	}
};

template <class A, class B, class C, class D, class E>
struct ParamTraits<std::tuple<A, B, C, D, E>> {
	typedef std::tuple<A, B, C, D, E> param_type;
	static void Write(Message* m, const param_type& p) {
		WriteParam(m, std::get<0>(p));
		WriteParam(m, std::get<1>(p));
		WriteParam(m, std::get<2>(p));
		WriteParam(m, std::get<3>(p));
		WriteParam(m, std::get<4>(p));
	}
	static bool Read(const Message* m,
					 base::PickleIterator* iter,
					 param_type* r) {
		return (ReadParam(m, iter, &std::get<0>(*r)) &&
				ReadParam(m, iter, &std::get<1>(*r)) &&
				ReadParam(m, iter, &std::get<2>(*r)) &&
				ReadParam(m, iter, &std::get<3>(*r)) &&
				ReadParam(m, iter, &std::get<4>(*r)));
	}
};



//-----------------------------------------------------------------------------
// Generic message subclasses

// Used for asynchronous messages.
template <class ParamType>
class MessageSchema {
  public:
	typedef ParamType Param;

	static void Write(Message* msg, const Param& p);
	static bool Read(const Message* msg, Param* p);
};


} // namespace IPC

#endif // IPC_MESSAGE_UTILS_H__
