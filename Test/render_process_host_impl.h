#ifndef RENDER_PROCESS_HOST_IMPL_H
#define RENDER_PROCESS_HOST_IMPL_H

#include <stdint.h>
#include <QObject>

#include "Content/render_process_host.h"
#include "Content/child_process_launcher.h"


namespace content {

class RenderProcessHostImpl : public QObject,
						      public RenderProcessHost,
							  public ChildProcessLauncher::Client
{
	Q_OBJECT

public:
	RenderProcessHostImpl(const QString& channel_name, QObject *parent = 0);
	virtual ~RenderProcessHostImpl();

public:
	virtual void OnProcessLaunched(qint64 pid);

	void LaunchChildProcess(const QString& program, const QStringList& param);


private:
	ChildProcessLauncher* child_process_launcher_;

};

}



#endif // RENDER_PROCESS_HOST_IMPL_H
