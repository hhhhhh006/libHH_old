#ifndef MESSAGE_PUMP_UI_H
#define MESSAGE_PUMP_UI_H

#include "base/base_export.h"
#include "base/message_loop/message_pump.h"

#include <QObject>
#include <QEvent>

namespace base {

class MessagePumpQt : public QObject, public MessagePump
{
public:
	MessagePumpQt();
	virtual ~MessagePumpQt();

protected:
	int GetCurrentDelay() const;

protected:
	const int kMsgHaveWork_;

	TimeTicks delayed_work_time_;

	Delegate* delegate_;

	bool should_quit_;
};

class BASE_EXPORT MessagePumpForUI : public MessagePumpQt
{
public:
	MessagePumpForUI();
	virtual ~MessagePumpForUI();

public:
	virtual void Run(Delegate* delegate);

	virtual void Quit();

	virtual void ScheduleWork();

	virtual void ScheduleDelayedWork(const TimeTicks& delayed_work_time);


protected:
	virtual bool event(QEvent *e);

private:
	void HandleWorkMessage();
	void HandleTimerMessage();
	void RescheduleTimer();

private:
	int timer_id_;
	
};


}

#endif // MESSAGE_PUMP_UI_H
