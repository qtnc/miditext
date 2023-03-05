#ifndef _____WORKER_THREAD_1
#define _____WORKER_THREAD_1
#include "../common/wxUtils.hpp"
#include <wx/thread.h>
#include <wx/msgqueue.h>
#include<functional>
#include<memory>

struct WorkerThread: wxThread {
typedef std::function<void()> Task;
struct App& app;
wxMessageQueue<Task> taskQueue;
bool running;

WorkerThread (App& app0);
void submit (const Task&);
void stop ();
virtual wxThread::ExitCode Entry () override;
};


#endif
