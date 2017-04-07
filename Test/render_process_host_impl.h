#ifndef RENDER_PROCESS_HOST_IMPL_H
#define RENDER_PROCESS_HOST_IMPL_H

#include <stdint.h>
#include <QObject>

#include "base/single_thread_task_runner.h"
#include "content/render_process_host.h"
#include "content/child_process_launcher.h"


namespace content {

class RenderProcessHostImpl : public QObject,
						      public RenderProcessHost,
							  public ChildProcessLauncher::Client
{
	Q_OBJECT

public:
	RenderProcessHostImpl(const QString& channel_name, 
        const scoped_refptr<base::SingleThreadTaskRunner>& ipc_task_runner,
        QObject *parent = 0);
	virtual ~RenderProcessHostImpl();

public:
	virtual void OnProcessLaunched(qint64 pid);

	void LaunchChildProcess(const QString& program, const QStringList& param);


private:
	ChildProcessLauncher* child_process_launcher_;

};

}



#endif // RENDER_PROCESS_HOST_IMPL_H
