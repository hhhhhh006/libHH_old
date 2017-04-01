#ifndef IPC_MESSAGE_H__
#define IPC_MESSAGE_H__

#include "base/pickle.h"
#include "ipc/ipc_export.h"

namespace IPC
{

class IPC_EXPORT Message : public base::Pickle
{
public:
	enum 
	{
		SYNC_BIT      = 0x01,
		REPLY_BIT     = 0x02,
	};

public:
	Message();

	Message(int32_t routing_id, uint32_t type);

	// Initializes a message from a const block of data.  The data is not copied;
	// instead the data is merely referenced by this message.  Only const methods
	// should be used on the message when initialized this way.
	Message(const char* data, int data_len);

	Message(const Message& other);
	Message& operator=(const Message& other);

	virtual ~Message();

	void set_sync() {
		header()->flags |= SYNC_BIT;
	}
	bool is_sync() const {
		return (header()->flags & SYNC_BIT) != 0;
	}

	void set_reply() {
		header()->flags |= REPLY_BIT;
	}
	bool is_reply() const {
		return (header()->flags * REPLY_BIT) != 0;
	}

	uint32_t type() const {
		return header()->type;
	}

	int32_t routing_id() const {
		return header()->routing;
	}
	void set_routing_id(int32_t new_id) {
		header()->routing = new_id;
	}

	uint32_t flags() const {
		return header()->flags;
	}

	// Sets all the given header values. The message should be empty at this call .
	void SetHeaderValues(int32_t routing, uint32_t type, uint32_t flags);

	template<class T, class S, class P>
	static bool Dispatch(const Message* msg, T* obj, S* sender, P* parameter, void (T::*func)()) 
	{
		(obj->*func)();
		return true;
	}

	template<class T, class S, class P>
	static bool Dispatch(const Message* msg, T* obj, S* sender, P* parameter, void (T::*func)(P*)) 
	{
		(obj->*func)(parameter);
		return true;
	}

	void set_sender_pid(int32_t id) { sender_pid_ = id; }
	int32_t get_sender_pid() const { return sender_pid_; }

	struct NextMessageInfo
	{
		NextMessageInfo();
		~NextMessageInfo();

		size_t message_size;

		bool message_found;

		const char* pickle_end;

		const char* message_end;
	};

	static void FindNext(const char* range_start, const char* range_end, NextMessageInfo* info);


#pragma pack(push, 4)
	struct Header : base::Pickle::Header 
	{
		int32_t routing;	//process id
		uint32_t type;		//message type
		uint32_t flags;
	};
#pragma pack(pop)

	Header* header(){
		return headerT<Header>();
	}
	const Header* header() const {
		return headerT<Header>();
	}

private:
	int32_t sender_pid_;

};

}

enum SpecialRoutingIDs {
	// indicates that we don't have a routing ID yet.
	MSG_ROUTING_NONE = -2,

	// indicates a general message
	MSG_ROUTING_CONTROL = INT32_MAX,
};

#define IPC_REPLY_ID 0xFFFFFFF0  // Special message id for replies


#endif // IPC_MESSAGE_H__
