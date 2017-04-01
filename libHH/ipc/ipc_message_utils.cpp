#include "ipc_message_utils.h"


namespace IPC {

void ParamTraits<LogData>::Write(Message* m, const param_type& p) 
{
	WriteParam(m, p.channel);
	WriteParam(m, p.routing_id);
	WriteParam(m, p.type);
	WriteParam(m, p.message_name);
	WriteParam(m, p.params);
}

bool ParamTraits<LogData>::Read(const Message* m,
								base::PickleIterator* iter,
								param_type* r) 
{
	return ReadParam(m, iter, &r->channel) &&
		   ReadParam(m, iter, &r->routing_id) &&
		   ReadParam(m, iter, &r->type) &&
		   ReadParam(m, iter, &r->message_name) &&
		   ReadParam(m, iter, &r->params);
}

void IPC::ParamTraits<signed char>::Write(Message* m, const param_type& p)
{
	m->WriteBytes(&p, sizeof(param_type));
}

bool ParamTraits<signed char>::Read(const Message* m,
									base::PickleIterator* iter,
									param_type* r) 
{
	const char* data;
	if (!iter->ReadBytes(&data, sizeof(param_type)))
		return false;
	memcpy(r, data, sizeof(param_type));
	return true;
}

void ParamTraits<unsigned char>::Write(Message* m, const param_type& p) {
	m->WriteBytes(&p, sizeof(param_type));
}

bool ParamTraits<unsigned char>::Read(const Message* m,
									  base::PickleIterator* iter,
									  param_type* r) 
{
	const char* data;
	if (!iter->ReadBytes(&data, sizeof(param_type)))
		return false;
	memcpy(r, data, sizeof(param_type));
	return true;
}

void ParamTraits<double>::Write(Message* m, const param_type& p) {
	m->WriteBytes(reinterpret_cast<const char*>(&p), sizeof(param_type));
}

bool ParamTraits<double>::Read(const Message* m,
							   base::PickleIterator* iter,
							   param_type* r) 
{
	const char *data;
	if (!iter->ReadBytes(&data, sizeof(*r))) {
		return false;
	}
	memcpy(r, data, sizeof(param_type));
	return true;
}

void ParamTraits<Message>::Write(Message* m, const Message& p) {
	m->WriteUInt32(static_cast<uint32_t>(p.routing_id()));
	m->WriteUInt32(p.type());
	m->WriteUInt32(p.flags());
	m->WriteData(p.payload(), static_cast<uint32_t>(p.payload_size()));
}

bool ParamTraits<Message>::Read(const Message* m,
								base::PickleIterator* iter,
								Message* r) 
{
	uint32_t routing_id, type, flags;
	if (!iter->ReadUInt32(&routing_id) ||
		!iter->ReadUInt32(&type) ||
		!iter->ReadUInt32(&flags))
		return false;

	int payload_size;
	const char* payload;
	if (!iter->ReadData(&payload, &payload_size))
		return false;

	r->SetHeaderValues(static_cast<int32_t>(routing_id), type, flags);
	return r->WriteBytes(payload, payload_size);
}

void ParamTraits<std::vector<char> >::Write(Message* m, const param_type& p) {
	if (p.empty()) {
		m->WriteData(NULL, 0);
	} else {
		m->WriteData(&p.front(), static_cast<int>(p.size()));
	}
}

bool ParamTraits<std::vector<char>>::Read(const Message* m,
										  base::PickleIterator* iter,
										  param_type* r) 
{
	const char *data;
	int data_size = 0;
	if (!iter->ReadData(&data, &data_size) || data_size < 0)
		return false;
	r->resize(data_size);
	if (data_size)
		memcpy(&r->front(), data, data_size);
	return true;
}

void ParamTraits<std::vector<unsigned char> >::Write(Message* m,
													 const param_type& p) 
{
	if (p.empty()) {
		m->WriteData(NULL, 0);
	} else {
		m->WriteData(reinterpret_cast<const char*>(&p.front()),
			static_cast<int>(p.size()));
	}
}

bool ParamTraits<std::vector<unsigned char>>::Read(const Message* m,
												   base::PickleIterator* iter,
												   param_type* r) 
{
	const char *data;
	int data_size = 0;
	if (!iter->ReadData(&data, &data_size) || data_size < 0)
		return false;
	r->resize(data_size);
	if (data_size)
		memcpy(&r->front(), data, data_size);
	return true;
}

void ParamTraits<std::vector<bool> >::Write(Message* m, const param_type& p) {
	WriteParam(m, static_cast<int>(p.size()));
	for (size_t i = 0; i < p.size(); i++)
		WriteParam(m, static_cast<bool>(p[i]));
}

bool ParamTraits<std::vector<bool>>::Read(const Message* m,
										  base::PickleIterator* iter,
										  param_type* r) 
{
	int size;
	// ReadLength() checks for < 0 itself.
	if (!iter->ReadLength(&size))
		return false;
	r->resize(size);
	for (int i = 0; i < size; i++) {
		bool value;
		if (!ReadParam(m, iter, &value))
			return false;
		(*r)[i] = value;
	}
	return true;
}


} // namespace IPC
