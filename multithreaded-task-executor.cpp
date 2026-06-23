/******************************************************************************
Question Link: https://enginebogie.com/interview/experience/rubrik-software-development-engineer/675
Gemini discussion: https://gemini.google.com/share/ee5a6a9ed07a

1283. Design a Multithreaded Task Execution System
Low-Level Design (LLD)Multi-Threaded ServicesMultithreadingConcurrencyThread SafetyThread SynchronizationTask Execution
Medium
Design a multi-threaded task execution system to enable efficient processing of tasks in parallel while ensuring a single-threaded client can synchronize when necessary. The system should implement the following interface:

class WorkerInterface {
    public void submitWork(); // Accepts a Task and returns immediately
    public void blockUntilComplete(); // Blocks the calling thread until all submitted tasks are finished
}
Requirements
Core Functionalities:

Task Submission: The submitWork method should allow tasks to be added for execution. The method must return immediately without waiting for the task to complete.
Task Completion Synchronization: The blockUntilComplete method should block the calling thread until all previously submitted tasks are fully executed.
Concurrency:

Multiple tasks should be executed concurrently using a thread pool.
Ensure thread-safe handling of task submission and synchronization.
Modifications Allowed:

You are allowed to modify the method signatures to include necessary parameters (e.g., task details or callback mechanisms).
Internal design changes, including the use of data structures or concurrency primitives, are permitted.
Performance Considerations:

Minimize overhead during task submission and ensure efficient utilization of threads.
Avoid unnecessary blocking or resource contention.
Edge Cases:

Handling scenarios where blockUntilComplete is called with no tasks submitted.
Ensure correct behavior if submitWork is called while the client thread is blocked on blockUntilComplete.
Support graceful shutdown of the system after all tasks are processed.

Example API Usage
Task Submission:

WorkerInterface worker = new WorkerImplementation();
worker.submitWork(() -> {
    System.out.println("Processing Task 1");
});
worker.submitWork(() -> {
    System.out.println("Processing Task 2");
});
Synchronization:

worker.blockUntilComplete(); // Blocks until all tasks are completed
System.out.println("All tasks finished!");
*******************************************************************************/
#include <iostream>
#include<bits/stdc++.h>
using namespace std;
class Task {
	int id;
public:
	Task(int id)
	{
		this->id=id;
	}
	void execute() {
		cout<<"TASK EXECUTION id: "<<id<<'\n';
	}
};
class WorkerInterface {
public:
	virtual void submitWork(Task task)=0;
	virtual void blockUntilComplete()=0;
	virtual ~WorkerInterface()=default;
};
class WorkerImplementation:public WorkerInterface {
	// shared resources
	queue<Task> taskQueue;
	// activeTasksCnt is the num of tasks either in queue or are currently executing
	atomic<int> activeTasksCnt{0};
	// like a done flag of multi-threaded web crawler question
	bool stopPool=false;
	vector<thread> workers;
	mutex queueMtx;
	condition_variable cvAllTasksDone;
	// client/main thread will wait on this cv and gets notify_all() when all
	// tasks are over in the queue and the pool stops accepting any further tasks

	condition_variable cvTaskAvailable;
	// the worker threads will wait on this cv and get notify_one()
	// when a new task gets submitted in the queue

	// workerTask func (similar to lambda func) that each worker task executes
	void workerTask() {
		// worker thread keeps executing in loop unless it needs to exit
		while(true)
		{
			Task currentTask(-1);
			// STEP1: wait/sleep if tasks queue is empty else grab a task
			{
				unique_lock<mutex> lock(queueMtx);
				while(taskQueue.empty() && !stopPool)
				{
					// worker thread sleeps when no tasks available
					cvTaskAvailable.wait(lock);
				}
				if(stopPool && taskQueue.empty())
				{
					return; //exits on stopPool
				}
				// thread safe queue modification due to unique_lock on queue mutex
				currentTask=taskQueue.front();
				taskQueue.pop();
				// activeTasksCnt--; can't decrement here as the task is yet not executed
			}
			// STEP2: execute the task, not under lock to have parallelism,
			// as task execution doesn't depend on any of shared resources
			currentTask.execute();

			// STEP3: task execution is done, now decrement activeTasksCnt
			{
				lock_guard<mutex> lock(queueMtx);
				activeTasksCnt--;
				if(activeTasksCnt==0)
				{
					// notify the main/client thread which is blocked on blockUntilComplete
					cvAllTasksDone.notify_all();
					// notify_all() instead of notify_one() so if there are multiple client threads
				}
			}
		}
	}
public:
	// spwan new workers in the constructor
	WorkerImplementation(int threadCount) {
		for(int i=0; i<threadCount; i++)
		{
			workers.push_back(thread(&WorkerImplementation::workerTask, this));
		}
	}

	void submitWork(Task task) override {
		{
			lock_guard<mutex> lock(queueMtx);
			taskQueue.push(task);
			activeTasksCnt++;
		}
		// let the notification be sent after the lock is released, so any new worker
		// which wakes up can grab the mutex
		cvTaskAvailable.notify_one();
		// so if any worker thread is sleeping it gets notified
		// that a new task arrived
	}

	// this is main thread which waits until all tasks are complete
	void blockUntilComplete() override {
		{
			unique_lock<mutex> lock(queueMtx);
			while(activeTasksCnt>0) {
				// main thread sleeps till all tasks are done
				cvAllTasksDone.wait(lock);
			}
		}
	}

	~WorkerImplementation()
	{
		{
			// cleanup , no worker thread shld be alive when main object is destroyed
			// since the threads spawning is done in the constructor, its destruction/join()
			// will be in the destructor
			lock_guard<mutex> lock(queueMtx);
			stopPool=true;
		} // release the lock HERE
		// else it'll lead to deadlock if on cvTaskAvailable.notify_all(); all threads
		// wake up but the mutex is being held by main thread in destructor and since
		// it'll go into a blocking call on worker.join() then it'll forever hold the lock
		// and other workers which just woken up to exit on stopPool=true won't be able
		// to get the lock and get blocked indefinetely - DEADLOCK!

		cvTaskAvailable.notify_all();
		// to wake up all worker threads so they can check on stopPool and exit the loop
		for(auto &worker:workers)
		{
			if(worker.joinable())
			{
				worker.join();
			}
		}
		cout<<"all threads joined\n";
	}
};

int main()
{
	cout<<"Hello World\n";

	WorkerInterface* pool=new WorkerImplementation(4);
	Task t1(1);
	Task t2(2);
	Task t3(3);

	pool->submitWork(t1);
	pool->submitWork(t2);

	pool->blockUntilComplete();

	pool->submitWork(t3);
	pool->blockUntilComplete();
	
	delete pool;

	return 0;
}

/*
Hello World
TASK EXECUTION id: 1
TASK EXECUTION id: 2
TASK EXECUTION id: 3
all threads joined
*/