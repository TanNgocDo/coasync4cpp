
#include "stdafx.h"
#include "TaskDispatcher.h"
#include "bind2thread.h"

class test_send2thread : public ::testing::Test {
protected:

	test_send2thread()  {
	}
	virtual void SetUp() {
		thread.reset(new ThreadWithTasks());
	}
	virtual void TearDown() {
		tearDown();
	}
	void awaitTasksDone() {
		thread->sync().get();
	}
	void tearDown()	{
		thread->stop();
		if (thread->joinable()) {
			thread->join();
		}
	}
	TaskDispatcherPtr secondThread() const _NOEXCEPT {
		return thread->dispatcher();
	}

	// DATA 

	std::shared_ptr<ThreadWithTasks> thread;

};


enum ResultOrigin {
	UndefinedOrigin,
	FromPost,
	FromSend 
};

TEST_F(test_send2thread, immediatelyExecution  ) {

	std::shared_ptr<TaskDispatcher4StdThread> mainThread(TaskDispatcher4StdThread::create());
	ResultOrigin originOfResult(UndefinedOrigin);
	int calls = 0;
	
	// sofort ausf�hren: 

	post2thread(mainThread, [&] {
		originOfResult = FromPost;
		++calls;
		mainThread->stop();
	});

	// send2current �berholt hier post2current, da es alles 
	// bereits im Zielthread abgefeuert wird. post2thread wird also zuletzt
	// ausgef�hrt und setzt damit originOfResult auf FromPost.
	// Das machen wir uns f�r den Test zunutze: 

	send2thread(mainThread, [&] {
		originOfResult = FromSend;
		++calls;
	});

	mainThread->run();

	EXPECT_EQ(originOfResult, FromPost);
	EXPECT_EQ( calls, 2);
	
}

TEST_F(test_send2thread, delayedExecution ) {

	std::shared_ptr<TaskDispatcher4StdThread> mainThread(TaskDispatcher4StdThread::create());
	ResultOrigin originOfResult(UndefinedOrigin);
	int calls = 0;

	post2thread( secondThread(), [&] {

		// sofort ausf�hren: 

		post2thread( mainThread, [&] {
			originOfResult = FromPost;
			++calls;
		});

		// send2current �berholt hier post2current NICHT, da es alles 
		// erst zur�ck in den hauptthread gepostet werden muss: 

		send2thread( mainThread, [&] {
			originOfResult = FromSend;
			++calls;
		});

		mainThread->stop();
	});

	mainThread->run();

	EXPECT_EQ(originOfResult, FromSend);
	EXPECT_EQ(calls, 2);

}

