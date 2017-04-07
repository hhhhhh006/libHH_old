#ifndef MESSAGE_FILTER_H_
#define MESSAGE_FILTER_H_

#include <stdint.h>
#include <vector>

#include "base/memory/ref_counted.h"
#include "ipc/ipc_export.h"

namespace IPC {

class Sender;
class Message;


class IPC_EXPORT MessageFilter
	: public base::RefCountedThreadSafe<MessageFilter>
{

public:
	MessageFilter();

    virtual void OnFilterAdded(Sender* sender);

    virtual void OnFilterRemoved();

    // Return true to indicate that the message was handled, or false to let
    // the message be handled in the default way.
    virtual bool OnMessageReceived(const Message& message);

    virtual void OnChannelConnected(int32_t peer_pid);

    // Called to query the Message classes supported by the filter.  Return
    // false to indicate that all message types should reach the filter, or true
    // if the resulting contents of |supported_message_classes| may be used to
    // selectively offer messages of a particular class to the filter.
    virtual bool GetSupportedMessageClasses(std::vector<uint32_t>* supported_message_classes) const;

protected:
  virtual ~MessageFilter();

private:
	friend class base::RefCountedThreadSafe<MessageFilter>;

};

}  // namespace IPC

#endif  // MESSAGE_FILTER_H_
