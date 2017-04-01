#ifndef IPC_MESSAGE_UTILS_IMPL_H_
#define IPC_MESSAGE_UTILS_IMPL_H_


namespace IPC {

template <class ParamType>
void MessageSchema<ParamType>::Write(Message* msg, const Param& p) 
{
	WriteParam(msg, p);
}

template <class ParamType>
bool MessageSchema<ParamType>::Read(const Message* msg, Param* p) 
{
	base::PickleIterator iter(*msg);
	if (ReadParam(msg, &iter, p))
		return true;
	return false;
}

}  // namespace IPC


#endif  // IPC_MESSAGE_UTILS_IMPL_H_