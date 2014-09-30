#pragma once 

// PROJECT 
#include "qtimer.h"

// LOCAL 
#include "TaskDispatcher.h"

/*
	 In QtThread und die Loop dort einh�ngen ... 
	 http://de.slideshare.net/neeramital/qt-framework-events-signals-threads 
	 http://qt-project.org/doc/qt-4.8/qcoreapplication.html#sendEvent
	 
	 Zu Qt Threads: 
	 http://mayaposch.wordpress.com/2011/11/01/how-to-really-truly-use-qthreads-the-full-explanation/ 

	 Anmerkung: Eine Implementierung von ThreadWithTasks f�r QtThread w�re ebenfalls leicht m�glich, 
	 wird aktuell aber nicht ben�tigt. 

	 Diesen Header im Zielprojekt dem Qt Metacompiler bekannt machen. Alternativ k�nnte man auch sigwd f�r das connect
	 nutzen, dann h�tten wir aber eine Abh�ngigkeit mehr im Projekt. 

*/
class TaskDispatcher4QtThread : public QObject , public TaskDispatcherBase {
	
	Q_OBJECT 
	typedef TaskDispatcherBase super;

public:

	// Erzeugt einen Taskdispatcher im aktuellen Qt-QCoreApplication-Thread. Geeignet um i aktuellen thread Tasks zu verarbeiten.
	static std::shared_ptr<TaskDispatcher4QtThread> create() {
		std::shared_ptr<TaskDispatcher> dispatcher(new TaskDispatcher4QtThread());
		::initCurrentThread(dispatcher);
		return std::static_pointer_cast<TaskDispatcher4QtThread> (dispatcher);
	}

private:

	// onActivation und onAbort besser sezifizieren:
	// onActivation - Abarbeitung von executeQueue veranlassen. TaskDispatcherBase sch�tzt aber bereits vor unn�tigen Aktivierungen, da� ist hier nicht
	// mehr n�tig. 
	// onAbort - Execution-Loop verlassen und TLS-Instanz austragen ; Parent k�mmert sich TaskDispatcherBase aber um Schutz vor neuen Posts.. 

	TaskDispatcher4QtThread() : super(std::bind(&TaskDispatcher4QtThread::onActivation, this),
									  std::bind(&TaskDispatcher4QtThread::onAbort, this)) {
	}

	inline void onActivation() { 
		QTimer::singleShot(0, this, SLOT(triggerExecuteQueue()));
	}

	inline void onAbort()  {
		this->post([] {
			::resetCurrentThread();
		});
	}

private Q_SLOTS:

	inline void triggerExecuteQueue() {
		this->executeQueue();
	}

};




