#include "WorkerThread.hpp"
#include "App.hpp"



WorkerThread::WorkerThread (App& app0):
app(app0), running(false) {
}

void WorkerThread::submit (const Task& task) {
taskQueue.Post(task);
}

void WorkerThread::stop () {
running=false;
submit([](){});
}

wxThread::ExitCode WorkerThread::Entry () {
Task task;
running=true;
while(running) {
taskQueue.Receive(task);
task();
}
return nullptr;
}

