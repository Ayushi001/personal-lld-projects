/******************************************************************************
Ques link: https://enginebogie.com/interview/experience/rubrik-software-development-engineer-2/1801
Gemini discussion link: https://share.gemini.google/LIwuyZdVhJAu
631. Design Bounded Blocking Queue
ConcurrencyMultithreadingMulti-Threaded ServicesBackendJavaSystem DesignLow-Level Design (LLD)
Medium
Implement a bounded blocking queue with the following operations:

enqueue(value): Add a value to the queue. If the queue is full, wait until there is space available.
dequeue(): Remove and return the front element from the queue. If the queue is empty, wait until there is an element available.
size(): Return the current size of the queue.
The goal is to design a thread-safe, bounded blocking queue that ensures proper synchronization and efficient use of resources.

Requirements:

The queue has a maximum capacity 'capacity' specified during initialization.
The enqueue operation should block if the queue is full, and it should resume when space becomes available.
The dequeue operation should block if the queue is empty, and it should resume when an element becomes available.
Ensure thread safety to prevent race conditions and maintain the integrity of the queue.
*******************************************************************************/
#include <iostream>
#include<bits/stdc++.h>

using namespace std;
class BoundedBlockingQueue {
	// shared resources
	queue<int> q;
	int size;
	bool isShutDown=false;

	// concurrency resources
	mutex mtx; // mutex over the shared resources
	condition_variable cv_full;
	// producer thread waits on it, if q is full, cv_full.wait(lock) till there's space to enqueue
	condition_variable cv_empty;
	// consumer thread waits on it, if q is empty, cv_empty.wait(lock) till there's an element to consume/dequeu
public:
	BoundedBlockingQueue(int capacity)
	{
		size=capacity;
	}
	// called by producer thread
	void enqueue(int value) {
		{
			// acquire lock over shared queue access
			unique_lock<mutex> lock(mtx);
			while(q.size()==size && !isShutDown)
			{
				cv_full.wait(lock);
			}
			if(isShutDown) return;
			q.push(value);
			cv_empty.notify_one(); // notify any sleeping consumer thread
			// can notify within lock scope as the producer/consumer threads when they wake up
			// won't fight for this mtx, but will be fighting for coutMtx which has nothing to do with
			// BoundedBlockingQueue
		}
	}
	// called by consumer thread
	int dequeue()
	{
		{
			unique_lock<mutex> lock(mtx);
			while(q.empty() && !isShutDown)
			{
				// sleep the consumer thread as nothing to dequeue
				cv_empty.wait(lock);
			}
			if(isShutDown) return -1;
			int top=q.front();
			q.pop();
			cv_full.notify_one();
			return top;
		}
	}
	int sizeQ()
	{
		{
			// acquire lock over queue so its size is not under update when reading it
			lock_guard<mutex> lock(mtx);
			return q.size();
		}
	}
	// wakes up all consumer/producer flags sleeping on condition_variable
	void shutdown()
	{
		{
			// since isShutDown is a shared resource btwn multiple threads, its
			// value must be changed after acquiring a lock
			lock_guard<mutex> lock(mtx);
			isShutDown=true;
			cv_full.notify_all();
			cv_empty.notify_all();
		}
		// notify outside lock scope as the main thread then release the mutex
		// even though all producers/consumers threads waking up on these cv
		// would be fighting over coutMtx and not on queue Mtx
		// 	 cv_full.notify_all();
		// 	 cv_empty.notify_all();
		// here the notifications must be sent within the lock scope, check gemini chats
	}
};
// shared mutex btwn consumer and producer threads over cout operation
// static and global to make them synchronised
static mutex coutMtx;
// consumer class spawns consumer thread that access the shared queue and also cleans
// them in destructor
class Consumer {
	// reference to queue as both consumer and producer work on same queue
	BoundedBlockingQueue &sharedQ;
	vector<thread> consumers;
	bool stopSignal=false;

	void consumerTask(int id)
	{
		while(!stopSignal)
		{
			int itemConsumed=sharedQ.dequeue();
			if(itemConsumed==-1) return;
			{
				lock_guard<mutex> lock(coutMtx);
				cout<<"Item consumed by thread id: "<<id<<" is = "<<itemConsumed<<'\n';
			}
		}
	}
public:
	Consumer(BoundedBlockingQueue& qRef, int threadNum) :sharedQ(qRef) {
		// 		sharedQ=qRef;
		for(int i=0; i<threadNum; i++)
		{
			consumers.push_back(thread(&Consumer::consumerTask, this, i));
		}
	}
	~Consumer() {
		stopSignal=true;
		for(int i=0; i<consumers.size(); i++)
		{
			if(consumers[i].joinable())
			{
				consumers[i].join();
			}
		}
	}
};

// consumer class spawns consumer thread that access the shared queue and also cleans
// them in destructor
class Producer {
	// reference to queue as both consumer and producer work on same queue
	BoundedBlockingQueue &sharedQ;
	vector<thread> producers;
	bool stopSignal=false;

	void producerTask(int id)
	{
		while(!stopSignal)
		{
			int itemProduced=rand()%100;
			sharedQ.enqueue(itemProduced);
			{
				lock_guard<mutex> lock(coutMtx);
				cout<<"Item produced by thread id: "<<id<<" is = "<<itemProduced<<'\n';
			}
		}
	}
public:
	Producer(BoundedBlockingQueue& qRef, int threadNum) :sharedQ(qRef) {
		// 		sharedQ=qRef;
		for(int i=0; i<threadNum; i++)
		{
			producers.push_back(thread(&Producer::producerTask, this, i));
		}
	}
	~Producer() {
		stopSignal=true;
		for(int i=0; i<producers.size(); i++)
		{
			if(producers[i].joinable())
			{
				producers[i].join();
			}
		}
	}
};
int main()
{
	BoundedBlockingQueue sharedQ(10);
	Consumer consumer(sharedQ,5);
	Producer producer(sharedQ,5);
	
	// let the threads work for 1s
	this_thread::sleep_for(chrono::seconds(1));
	sharedQ.shutdown();
	return 0;
}

/*
OUTPUT:
Item produced by thread id: 0 is = 12
Item produced by thread id: 0 is = 90
Item produced by thread id: 0 is = 12
Item produced by thread id: 2 is = 80
Item consumed by thread id: 0 is = 16
Item consumed by thread id: 0 is = 93
Item consumed by thread id: 1 is = 70
Item consumed by thread id: 1 is = 57
Item produced by thread id: 4 is = 46
Item consumed by thread id: 2 is = 31
Item consumed by thread id: 2 is = 12
Item consumed by thread id: 2 is = 86
Item consumed by thread id: 2 is = 14
Item consumed by thread id: 2 is = 55
Item consumed by thread id: 2 is = 12
Item consumed by thread id: 2 is = 90
Item consumed by thread id: 2 is = 12
Item consumed by thread id: 2 is = 79
Item consumed by thread id: 2 is = 10
Item produced by thread id: 3 is = 51
Item produced by thread id: 3 is = 69
Item produced by thread id: 3 is = 89
Item produced by thread id: 3 is = 55
Item produced by thread id: 3 is = 41
Item produced by thread id: 3 is = 20
Item produced by thread id: 3 is = 33
Item produced by thread id: 3 is = 87
Item produced by thread id: 3 is = 88
Item produced by thread id: 3 is = 38
Item produced by thread id: 3 is = 66
Item produced by thread id: 3 is = 70
Item produced by thread id: 2 is = 10
Item consumed by thread id: 0 is = 27
Item consumed by thread id: 0 is = 74
Item consumed by thread id: 0 is = 55
Item consumed by thread id: 0 is = 41
Item consumed by thread id: 0 is = 20
Item consumed by thread id: 0 is = 33
Item consumed by thread id: 0 is = 87
Item consumed by thread id: 0 is = 88
Item consumed by thread id: 0 is = 38
Item consumed by thread id: 0 is = 66
Item consumed by thread id: 0 is = 70
Item consumed by thread id: 3 is = 51
Item produced by thread id: 2 is = 56
Item produced by thread id: 2 is = 17
Item produced by thread id: 2 is = 6
Item produced by thread id: 2 is = 60
Item produced by thread id: 2 is = 49
Item produced by thread id: 2 is = 37
Item produced by thread id: 2 is = 5
Item produced by thread id: 2 is = 59
Item produced by thread id: 2 is = 17
Item produced by thread id: 2 is = 18
Item produced by thread id: 2 is = 45
Item produced by thread id: 0 is = 79
Item consumed by thread id: 2 is = 89
Item consumed by thread id: 1 is = 69
Item consumed by thread id: 1 is = 6
Item produced by thread id: 0 is = 73
Item consumed by thread id: 2 is = 49
Item consumed by thread id: 2 is = 84
Item consumed by thread id: 0 is = 17
Item consumed by thread id: 0 is = 37
Item consumed by thread id: 0 is = 5
Item consumed by thread id: 0 is = 59
Item consumed by thread id: 0 is = 17
Item consumed by thread id: 0 is = 18
Item consumed by thread id: 0 is = 45
Item consumed by thread id: 0 is = 73
Item consumed by thread id: 0 is = 83
Item consumed by thread id: 0 is = 58
Item consumed by thread id: 4 is = 46
Item consumed by thread id: 1 is = 60
Item produced by thread id: 3 is = 84
Item produced by thread id: 3 is = 73
Item produced by thread id: 3 is = 37
Item produced by thread id: 0 is = 58
Item consumed by thread id: 2 is = 37
Item produced by thread id: 0 is = 83
Item consumed by thread id: 2 is = 89
Item produced by thread id: 0 is = 7
Item consumed by thread id: 2 is = 83
Item produced by thread id: 4 is = 74
Item produced by thread id: 4 is = 57
Item produced by thread id: 4 is = 14
Item produced by thread id: 4 is = 71
Item produced by thread id: 4 is = 29
Item produced by thread id: 4 is = 0
Item produced by thread id: 4 is = 59
Item produced by thread id: 4 is = 18
Item produced by thread id: 4 is = 38
Item produced by thread id: 4 is = 25
Item produced by thread id: 4 is = 88
Item produced by thread id: 4 is = 74
Item consumed by thread id: 4 is = 7
Item consumed by thread id: 4 is = 14
Item consumed by thread id: 4 is = 71
Item consumed by thread id: 4 is = 29
Item consumed by thread id: 4 is = 0
Item consumed by thread id: 4 is = 59
Item consumed by thread id: 4 is = 18
Item consumed by thread id: 4 is = 38
Item consumed by thread id: 4 is = 25
Item consumed by thread id: 4 is = 88
Item consumed by thread id: 4 is = 74
Item consumed by thread id: 4 is = 33
Item produced by thread id: 2 is = 83
Item produced by thread id: 2 is = 57
Item produced by thread id: 2 is = 81
Item produced by thread id: 2 is = 93
Item produced by thread id: 2 is = 58
Item produced by thread id: 2 is = 70
Item produced by thread id: 2 is = 99
Item produced by thread id: 2 is = 17
Item produced by thread id: 2 is = 39
Item produced by thread id: 2 is = 69
Item produced by thread id: 2 is = 63
Item produced by thread id: 3 is = 89
Item produced by thread id: 0 is = 78
Item produced by thread id: 1 is = 29
Item consumed by thread id: 1 is = 73
Item consumed by thread id: 1 is = 57
Item consumed by thread id: 1 is = 81
Item consumed by thread id: 1 is = 93
Item consumed by thread id: 1 is = 58
Item consumed by thread id: 1 is = 70
Item consumed by thread id: 1 is = 99
Item consumed by thread id: 1 is = 17
Item consumed by thread id: 1 is = 39
Item consumed by thread id: 1 is = 69
Item consumed by thread id: 1 is = 63
Item consumed by thread id: 0 is = 78
Item produced by thread id: 3 is = 94
Item produced by thread id: 3 is = 31
Item produced by thread id: 3 is = 62
Item produced by thread id: 3 is = 82
Item produced by thread id: 3 is = 90
Item produced by thread id: 3 is = 92
Item produced by thread id: 3 is = 91
Item produced by thread id: 3 is = 57
Item produced by thread id: 3 is = 15
Item produced by thread id: 3 is = 21
Item consumed by thread id: 0 is = 94
Item consumed by thread id: 0 is = 31
Item consumed by thread id: 0 is = 62
Item consumed by thread id: 0 is = 82
Item consumed by thread id: 0 is = 90
Item consumed by thread id: 0 is = 92
Item consumed by thread id: 0 is = 57
Item consumed by thread id: 2 is = 57
Item consumed by thread id: 2 is = 15
Item consumed by thread id: 2 is = 21
Item consumed by thread id: 2 is = 57
Item consumed by thread id: 2 is = 73
Item consumed by thread id: 2 is = 47
Item produced by thread id: 0 is = 73
Item consumed by thread id: 1 is = 91
Item produced by thread id: 4 is = 33
Item produced by thread id: 1 is = 47
Item produced by thread id: 1 is = 91
Item produced by thread id: 1 is = 47
Item produced by thread id: 1 is = 51
Item produced by thread id: 1 is = 31
Item produced by thread id: 1 is = 21
Item produced by thread id: 1 is = 37
Item produced by thread id: 1 is = 40
Item produced by thread id: 1 is = 54
Item produced by thread id: 1 is = 30
Item produced by thread id: 1 is = 98
Item produced by thread id: 2 is = 22
Item consumed by thread id: 1 is = 91
Item consumed by thread id: 1 is = 47
Item consumed by thread id: 1 is = 51
Item consumed by thread id: 1 is = 31
Item produced by thread id: 3 is = 57
Item consumed by thread id: 2 is = 74
Item consumed by thread id: 2 is = 37
Item consumed by thread id: 2 is = 54
Item produced by thread id: 2 is = 81
Item produced by thread id: 2 is = 2
Item produced by thread id: 2 is = 31
Item produced by thread id: 2 is = 39
Item produced by thread id: 2 is = 96
Item consumed by thread id: 2 is = 30
Item consumed by thread id: 0 is = 22
Item consumed by thread id: 0 is = 98
Item consumed by thread id: 0 is = 81
Item consumed by thread id: 0 is = 25
Item consumed by thread id: 0 is = 16
Item consumed by thread id: 0 is = 2
Item consumed by thread id: 0 is = 31
Item consumed by thread id: 0 is = 39
Item consumed by thread id: 1 is = 21
Item consumed by thread id: 1 is = 96
Item produced by thread id: 2 is = 4
Item produced by thread id: 1 is = 25
Item produced by thread id: 1 is = 38
Item produced by thread id: 1 is = 80
Item produced by thread id: 1 is = 18
Item produced by thread id: 1 is = 21
Item produced by thread id: 1 is = 70
Item produced by thread id: 1 is = 62
Item produced by thread id: 1 is = 12
Item produced by thread id: 1 is = 79
Item produced by thread id: 1 is = 77
Item produced by thread id: 1 is = 85
Item consumed by thread id: 2 is = 16
Item consumed by thread id: 2 is = 80
Item produced by thread id: 3 is = 16
Item produced by thread id: 0 is = 74
Item consumed by thread id: 1 is = 38
Item consumed by thread id: 1 is = 21
Item consumed by thread id: 1 is = 70
Item consumed by thread id: 2 is = 18
Item consumed by thread id: 2 is = 62
Item consumed by thread id: 2 is = 12
Item produced by thread id: 1 is = 36
Item produced by thread id: 1 is = 7
Item produced by thread id: 1 is = 59
Item produced by thread id: 1 is = 57
Item consumed by thread id: 0 is = 4
Item consumed by thread id: 0 is = 85
Item consumed by thread id: 0 is = 4
Item produced by thread id: 0 is = 83
Item produced by thread id: 1 is = 44
Item produced by thread id: 0 is = 99
Item produced by thread id: 0 is = 27
Item produced by thread id: 0 is = 50
Item consumed by thread id: 4 is = 40
Item consumed by thread id: 4 is = 76
Item consumed by thread id: 4 is = 83
Item consumed by thread id: 4 is = 7
Item consumed by thread id: 4 is = 59
Item consumed by thread id: 4 is = 57
Item consumed by thread id: 4 is = 44
Item consumed by thread id: 4 is = 99
Item consumed by thread id: 4 is = 11
Item consumed by thread id: 4 is = 27
Item consumed by thread id: 4 is = 50
Item consumed by thread id: 3 is = 56
Item produced by thread id: 0 is = 36
Item produced by thread id: 0 is = 60
Item produced by thread id: 0 is = 18
Item produced by thread id: 0 is = 5
Item produced by thread id: 0 is = 63
Item produced by thread id: 0 is = 49
Item produced by thread id: 0 is = 44
Item produced by thread id: 0 is = 11
Item produced by thread id: 0 is = 5
Item produced by thread id: 0 is = 34
Item produced by thread id: 2 is = 4
Item consumed by thread id: 3 is = 36
Item consumed by thread id: 3 is = 60
Item consumed by thread id: 3 is = 18
Item consumed by thread id: 3 is = 5
Item consumed by thread id: 3 is = 63
Item consumed by thread id: 3 is = 49
Item consumed by thread id: 3 is = 44
Item consumed by thread id: 3 is = 11
Item consumed by thread id: 3 is = 5
Item consumed by thread id: 3 is = 34
Item produced by thread id: 4 is = 16
Item produced by thread id: 4 is = 55
Item produced by thread id: 4 is = 14
Item produced by thread id: 4 is = 89
Item produced by thread id: 4 is = 68
Item produced by thread id: 4 is = 93
Item produced by thread id: 4 is = 18
Item produced by thread id: 4 is = 5
Item produced by thread id: 4 is = 82
Item produced by thread id: 4 is = 22
Item produced by thread id: 4 is = 82
Item consumed by thread id: 4 is = 55
Item consumed by thread id: 4 is = 14
Item consumed by thread id: 4 is = 89
Item consumed by thread id: 4 is = 68
Item consumed by thread id: 4 is = 93
Item consumed by thread id: 4 is = 18
Item consumed by thread id: 4 is = 5
Item consumed by thread id: 4 is = 82
Item consumed by thread id: 4 is = 22
Item consumed by thread id: 4 is = 82
Item produced by thread id: 3 is = 76
Item produced by thread id: 3 is = 30
Item produced by thread id: 3 is = 93
Item produced by thread id: 3 is = 74
Item produced by thread id: 3 is = 26
Item produced by thread id: 3 is = 93
Item produced by thread id: 3 is = 86
Item produced by thread id: 3 is = 53
Item produced by thread id: 3 is = 43
Item produced by thread id: 3 is = 74
Item produced by thread id: 3 is = 14
Item consumed by thread id: 2 is = 79
Item consumed by thread id: 3 is = 30
Item consumed by thread id: 2 is = 93
Item consumed by thread id: 3 is = 74
Item consumed by thread id: 3 is = 26
Item consumed by thread id: 3 is = 93
Item consumed by thread id: 3 is = 86
Item consumed by thread id: 3 is = 53
Item consumed by thread id: 3 is = 43
Item consumed by thread id: 3 is = 74
Item consumed by thread id: 3 is = 14
Item consumed by thread id: 3 is = 75
Item produced by thread id: 0 is = 91
Item produced by thread id: 0 is = 79
Item produced by thread id: 0 is = 77
Item produced by thread id: 0 is = 62
Item produced by thread id: 0 is = 75
Item produced by thread id: 0 is = 88
Item produced by thread id: 0 is = 19
Item produced by thread id: 0 is = 10
Item produced by thread id: 0 is = 32
Item produced by thread id: 0 is = 94
Item produced by thread id: 2 is = 75
Item produced by thread id: 1 is = 11
Item consumed by thread id: 1 is = 77
Item consumed by thread id: 0 is = 36
Item consumed by thread id: 4 is = 91
Item consumed by thread id: 2 is = 79
Item produced by thread id: 4 is = 17
Item produced by thread id: 4 is = 37
Item produced by thread id: 4 is = 91
Item produced by thread id: 2 is = 46
Item produced by thread id: 3 is = 13
Item produced by thread id: 2 is = 43
Item consumed by thread id: 2 is = 75
Item consumed by thread id: 3 is = 88
Item consumed by thread id: 4 is = 62
Item consumed by thread id: 1 is = 77
Item produced by thread id: 0 is = 17
Item consumed by thread id: 2 is = 10
Item consumed by thread id: 2 is = 32
Item consumed by thread id: 3 is = 19
Item produced by thread id: 1 is = 35
Item produced by thread id: 1 is = 91
Item produced by thread id: 4 is = 53
Item produced by thread id: 2 is = 28
Item produced by thread id: 2 is = 17
Item consumed by thread id: 1 is = 17
Item consumed by thread id: 4 is = 37
Item consumed by thread id: 1 is = 13
Item consumed by thread id: 1 is = 43
Item consumed by thread id: 1 is = 17
Item consumed by thread id: 1 is = 35
Item consumed by thread id: 1 is = 91
Item consumed by thread id: 1 is = 53
Item consumed by thread id: 1 is = 28
Item consumed by thread id: 1 is = 17
Item consumed by thread id: 1 is = 10
Item consumed by thread id: 1 is = 73
Item produced by thread id: 0 is = 25
Item produced by thread id: 0 is = 63
Item produced by thread id: 0 is = 55
Item produced by thread id: 0 is = 90
Item produced by thread id: 0 is = 58
Item produced by thread id: 0 is = 30
Item produced by thread id: 0 is = 4
Item produced by thread id: 4 is = 18
Item produced by thread id: 2 is = 36
Item produced by thread id: 2 is = 71
Item produced by thread id: 2 is = 61
Item produced by thread id: 1 is = 10
Item consumed by thread id: 2 is = 46
Item produced by thread id: 3 is = 73
Item produced by thread id: 3 is = 4
Item consumed by thread id: 3 is = 91
Item consumed by thread id: 4 is = 25
Item produced by thread id: 4 is = 33
Item produced by thread id: 4 is = 5
Item produced by thread id: 4 is = 50
Item consumed by thread id: 1 is = 63
Item consumed by thread id: 1 is = 36
Item consumed by thread id: 1 is = 58
Item consumed by thread id: 1 is = 30
Item consumed by thread id: 1 is = 4
Item consumed by thread id: 1 is = 71
Item consumed by thread id: 1 is = 61
Item consumed by thread id: 1 is = 33
Item consumed by thread id: 1 is = 4
Item consumed by thread id: 1 is = 5
Item consumed by thread id: 1 is = 50
Item produced by thread id: 4 is = 68
Item produced by thread id: 4 is = 3
Item produced by thread id: 4 is = 85
Item produced by thread id: 4 is = 6
Item produced by thread id: 4 is = 95
Item produced by thread id: 2 is = 89
Item produced by thread id: 0 is = 73
Item produced by thread id: 3 is = 51
Item consumed by thread id: 4 is = 18
Item consumed by thread id: 3 is = 90
Item consumed by thread id: 3 is = 3
Item consumed by thread id: 0 is = 94
Item produced by thread id: 1 is = 85
Item consumed by thread id: 1 is = 68
Item consumed by thread id: 1 is = 85
Item consumed by thread id: 2 is = 55
Item consumed by thread id: 2 is = 51
Item consumed by thread id: 2 is = 6
Item consumed by thread id: 2 is = 95
Item consumed by thread id: 2 is = 39
Item consumed by thread id: 2 is = 20
Item consumed by thread id: 2 is = 67
Item consumed by thread id: 2 is = 26
Item consumed by thread id: 0 is = 73
Item produced by thread id: 1 is = 26
Item produced by thread id: 4 is = 39
Item consumed by thread id: 4 is = 85
Item produced by thread id: 4 is = 77
Item produced by thread id: 4 is = 96
Item produced by thread id: 4 is = 81
Item produced by thread id: 4 is = 65
Item produced by thread id: 4 is = 60
Item produced by thread id: 4 is = 36
Item produced by thread id: 4 is = 55
Item produced by thread id: 4 is = 70
Item produced by thread id: 4 is = 18
Item produced by thread id: 4 is = 11
Item produced by thread id: 4 is = 42
Item produced by thread id: 4 is = 32
Item produced by thread id: 2 is = 67
Item consumed by thread id: 3 is = 89
Item consumed by thread id: 3 is = 81
Item consumed by thread id: 3 is = 65
Item consumed by thread id: 3 is = 60
Item consumed by thread id: 1 is = 49
Item consumed by thread id: 0 is = 77
Item produced by thread id: 0 is = 49
Item produced by thread id: 0 is = 21
Item produced by thread id: 3 is = 20
Item produced by thread id: 1 is = 63
Item produced by thread id: 1 is = 84
Item produced by thread id: 1 is = 72
Item consumed by thread id: 2 is = 63
Item consumed by thread id: 3 is = 36
Item consumed by thread id: 3 is = 18
Item consumed by thread id: 3 is = 11
Item consumed by thread id: 3 is = 42
Item consumed by thread id: 3 is = 32
Item consumed by thread id: 3 is = 79
Item consumed by thread id: 3 is = 96
Item consumed by thread id: 3 is = 21
Item consumed by thread id: 3 is = 70
Item consumed by thread id: 3 is = 84
Item consumed by thread id: 0 is = 70
Item consumed by thread id: 4 is = 96
Item produced by thread id: 0 is = 70
Item produced by thread id: 2 is = 79
Item produced by thread id: 2 is = 83
Item consumed by thread id: 2 is = 72
Item consumed by thread id: 2 is = 83
Item produced by thread id: 3 is = 27
Item produced by thread id: 3 is = 72
Item produced by thread id: 3 is = 98
Item consumed by thread id: 0 is = 34
Item consumed by thread id: 0 is = 98
Item consumed by thread id: 0 is = 30
Item consumed by thread id: 0 is = 63
Item produced by thread id: 0 is = 40
Item produced by thread id: 0 is = 47
Item produced by thread id: 0 is = 50
Item produced by thread id: 0 is = 30
Item produced by thread id: 0 is = 73
Item consumed by thread id: 1 is = 55
Item produced by thread id: 1 is = 34
Item produced by thread id: 1 is = 14
Item produced by thread id: 1 is = 59
Item produced by thread id: 1 is = 22
Item produced by thread id: 1 is = 47
Item produced by thread id: 1 is = 24
Item produced by thread id: 1 is = 82
Item produced by thread id: 1 is = 35
Item consumed by thread id: 1 is = 50
Item consumed by thread id: 1 is = 30
Item consumed by thread id: 1 is = 73
Item consumed by thread id: 1 is = 14
Item produced by thread id: 3 is = 63
Item consumed by thread id: 0 is = 47
Item produced by thread id: 2 is = 30
Item produced by thread id: 2 is = 43
Item produced by thread id: 2 is = 98
Item consumed by thread id: 3 is = 27
Item consumed by thread id: 3 is = 47
Item consumed by thread id: 3 is = 24
Item consumed by thread id: 3 is = 82
Item consumed by thread id: 1 is = 59
Item consumed by thread id: 1 is = 32
Item consumed by thread id: 1 is = 4
Item consumed by thread id: 1 is = 54
Item consumed by thread id: 1 is = 43
Item consumed by thread id: 1 is = 98
Item consumed by thread id: 1 is = 86
Item produced by thread id: 3 is = 54
Item produced by thread id: 3 is = 40
Item produced by thread id: 3 is = 78
Item produced by thread id: 3 is = 59
Item consumed by thread id: 4 is = 40
Item consumed by thread id: 4 is = 78
Item produced by thread id: 0 is = 4
Item produced by thread id: 0 is = 62
Item produced by thread id: 0 is = 83
Item consumed by thread id: 3 is = 35
Item consumed by thread id: 0 is = 22
Item consumed by thread id: 0 is = 62
Item consumed by thread id: 0 is = 62
Item consumed by thread id: 0 is = 83
Item consumed by thread id: 0 is = 41
Item consumed by thread id: 1 is = 40
Item produced by thread id: 1 is = 32
Item produced by thread id: 1 is = 48
Item produced by thread id: 1 is = 23
Item produced by thread id: 1 is = 24
Item produced by thread id: 1 is = 72
Item produced by thread id: 1 is = 22
Item produced by thread id: 1 is = 54
Item produced by thread id: 1 is = 35
Item produced by thread id: 1 is = 21
Item produced by thread id: 1 is = 57
Item produced by thread id: 1 is = 65
Item produced by thread id: 3 is = 62
Item consumed by thread id: 3 is = 48
Item produced by thread id: 2 is = 86
Item produced by thread id: 2 is = 71
Item produced by thread id: 1 is = 47
Item consumed by thread id: 0 is = 72
Item consumed by thread id: 0 is = 22
Item consumed by thread id: 0 is = 54
Item consumed by thread id: 0 is = 35
Item consumed by thread id: 0 is = 21
Item consumed by thread id: 0 is = 57
Item consumed by thread id: 0 is = 65
Item consumed by thread id: 0 is = 47
Item consumed by thread id: 0 is = 71
Item produced by thread id: 2 is = 76
Item consumed by thread id: 2 is = 72
Item produced by thread id: 0 is = 41
Item produced by thread id: 0 is = 3
Item produced by thread id: 0 is = 53
Item produced by thread id: 0 is = 33
Item produced by thread id: 0 is = 7
Item produced by thread id: 0 is = 59
Item produced by thread id: 0 is = 28
Item produced by thread id: 0 is = 6
Item produced by thread id: 0 is = 97
Item consumed by thread id: 0 is = 18
Item consumed by thread id: 0 is = 69
Item produced by thread id: 0 is = 20
Item produced by thread id: 0 is = 84
Item produced by thread id: 2 is = 1
Item produced by thread id: 4 is = 96
Item produced by thread id: 1 is = 18
Item consumed by thread id: 3 is = 23
Item consumed by thread id: 3 is = 3
Item produced by thread id: 0 is = 8
Item consumed by thread id: 3 is = 53
Item consumed by thread id: 3 is = 33
Item consumed by thread id: 3 is = 7
Item consumed by thread id: 3 is = 59
Item consumed by thread id: 3 is = 28
Item consumed by thread id: 3 is = 6
Item consumed by thread id: 3 is = 97
Item consumed by thread id: 3 is = 20
Item consumed by thread id: 3 is = 84
Item consumed by thread id: 3 is = 8
Item consumed by thread id: 3 is = 76
Item consumed by thread id: 3 is = 98
Item consumed by thread id: 3 is = 91
Item consumed by thread id: 1 is = 24
Item produced by thread id: 2 is = 34
Item produced by thread id: 2 is = 98
Item produced by thread id: 0 is = 76
Item consumed by thread id: 1 is = 34
Item consumed by thread id: 1 is = 98
Item consumed by thread id: 1 is = 15
Item consumed by thread id: 1 is = 52
Item produced by thread id: 4 is = 98
Item produced by thread id: 4 is = 71
Item produced by thread id: 4 is = 89
Item produced by thread id: 4 is = 59
Item consumed by thread id: 4 is = 59
Item consumed by thread id: 3 is = 89
Item consumed by thread id: 3 is = 59
Item produced by thread id: 0 is = 52
Item produced by thread id: 0 is = 6
Item produced by thread id: 0 is = 10
Item produced by thread id: 0 is = 16
Item produced by thread id: 0 is = 24
Item produced by thread id: 0 is = 9
Item produced by thread id: 0 is = 39
Item produced by thread id: 0 is = 0
Item consumed by thread id: 1 is = 71
Item consumed by thread id: 1 is = 16
Item consumed by thread id: 1 is = 24
Item consumed by thread id: 1 is = 9
Item consumed by thread id: 1 is = 39
Item consumed by thread id: 1 is = 0
Item consumed by thread id: 1 is = 78
Item consumed by thread id: 1 is = 9
Item produced by thread id: 1 is = 91
Item produced by thread id: 1 is = 53
Item produced by thread id: 1 is = 81
Item produced by thread id: 1 is = 14
Item produced by thread id: 1 is = 38
Item produced by thread id: 1 is = 89
Item produced by thread id: 1 is = 26
Item produced by thread id: 1 is = 67
Item produced by thread id: 1 is = 47
Item produced by thread id: 1 is = 23
Item produced by thread id: 1 is = 87
Item produced by thread id: 1 is = 31
Item produced by thread id: 0 is = 78
Item consumed by thread id: 2 is = 76
Item consumed by thread id: 2 is = 81
Item consumed by thread id: 2 is = 14
Item produced by thread id: 0 is = 22
Item consumed by thread id: 0 is = 1
Item consumed by thread id: 0 is = 89
Item consumed by thread id: 0 is = 26
Item consumed by thread id: 0 is = 67
Item consumed by thread id: 0 is = 47
Item consumed by thread id: 0 is = 23
Item produced by thread id: 2 is = 15
Item produced by thread id: 2 is = 75
Item produced by thread id: 2 is = 50
Item produced by thread id: 1 is = 32
Item produced by thread id: 3 is = 69
Item produced by thread id: 3 is = 54
Item produced by thread id: 3 is = 50
Item produced by thread id: 4 is = 9
Item consumed by thread id: 1 is = 53
Item consumed by thread id: 1 is = 31
Item consumed by thread id: 0 is = 87
Item consumed by thread id: 1 is = 32
Item consumed by thread id: 1 is = 81
Item consumed by thread id: 1 is = 75
Item consumed by thread id: 2 is = 38
Item consumed by thread id: 1 is = 50
Item consumed by thread id: 1 is = 90
Item consumed by thread id: 1 is = 54
Item consumed by thread id: 1 is = 50
Item produced by thread id: 3 is = 31
Item produced by thread id: 3 is = 57
Item produced by thread id: 3 is = 94
Item produced by thread id: 3 is = 81
Item produced by thread id: 3 is = 81
Item produced by thread id: 3 is = 3
Item produced by thread id: 3 is = 20
Item produced by thread id: 3 is = 33
Item produced by thread id: 3 is = 82
Item produced by thread id: 3 is = 81
Item produced by thread id: 3 is = 87
Item produced by thread id: 1 is = 90
Item consumed by thread id: 4 is = 6
Item consumed by thread id: 2 is = 79
Item consumed by thread id: 4 is = 57
Item consumed by thread id: 4 is = 81
Item consumed by thread id: 4 is = 81
Item consumed by thread id: 4 is = 3
Item consumed by thread id: 4 is = 20
Item consumed by thread id: 4 is = 33
Item consumed by thread id: 4 is = 82
Item consumed by thread id: 4 is = 81
Item consumed by thread id: 4 is = 87
Item consumed by thread id: 4 is = 96
Item consumed by thread id: 0 is = 22
Item produced by thread id: 4 is = 13
Item produced by thread id: 1 is = 96
Item produced by thread id: 0 is = 81
Item consumed by thread id: 3 is = 10
Item produced by thread id: 0 is = 4
Item produced by thread id: 4 is = 25
Item produced by thread id: 0 is = 22
Item produced by thread id: 1 is = 51
Item produced by thread id: 2 is = 79
Item produced by thread id: 0 is = 97
Item consumed by thread id: 0 is = 13
Item produced by thread id: 2 is = 32
Item consumed by thread id: 3 is = 25
Item produced by thread id: 0 is = 81
Item consumed by thread id: 2 is = 94
Item consumed by thread id: 0 is = 4
Item produced by thread id: 3 is = 15
Item produced by thread id: 1 is = 15
Item produced by thread id: 3 is = 57
Item produced by thread id: 3 is = 8
Item consumed by thread id: 0 is = 51
Item consumed by thread id: 4 is = 97
Item consumed by thread id: 4 is = 32
Item consumed by thread id: 0 is = 34
Item consumed by thread id: 1 is = 31
Item produced by thread id: 2 is = 34
Item consumed by thread id: 3 is = 22
Item produced by thread id: 4 is = 92
Item consumed by thread id: 4 is = 15
Item consumed by thread id: 4 is = 15
Item consumed by thread id: 4 is = 57
Item consumed by thread id: 2 is = 92
Item consumed by thread id: 1 is = 81
Item consumed by thread id: 1 is = 8
Item consumed by thread id: 1 is = 99
Item produced by thread id: 1 is = 99
Item produced by thread id: 0 is = 6
Item produced by thread id: 1 is = 83
Item consumed by thread id: 2 is = 95
Item produced by thread id: 1 is = 54
Item produced by thread id: 3 is = 95
Item consumed by thread id: 0 is = 83
Item produced by thread id: 2 is = 97
Item produced by thread id: 2 is = 87
Item produced by thread id: 2 is = 32
Item produced by thread id: 2 is = 82
Item produced by thread id: 2 is = 21
Item produced by thread id: 2 is = 66
Item produced by thread id: 2 is = 63
Item produced by thread id: 2 is = 60
Item consumed by thread id: 2 is = 97
Item produced by thread id: 1 is = 77
Item produced by thread id: 1 is = 11
Item consumed by thread id: 4 is = 76
Item consumed by thread id: 4 is = 9
Item consumed by thread id: 4 is = 87
Item consumed by thread id: 4 is = 32
Item produced by thread id: 1 is = 85
Item produced by thread id: 1 is = 86
Item produced by thread id: 1 is = 85
Item produced by thread id: 1 is = 30
Item consumed by thread id: 3 is = 6
Item consumed by thread id: 3 is = 21
Item consumed by thread id: 3 is = 66
Item consumed by thread id: 3 is = 63
Item consumed by thread id: 2 is = 77
Item consumed by thread id: 2 is = 82
Item consumed by thread id: 2 is = 11
Item consumed by thread id: 2 is = 85
Item consumed by thread id: 2 is = 86
Item consumed by thread id: 2 is = 85
Item consumed by thread id: 2 is = 30
Item consumed by thread id: 2 is = 90
Item produced by thread id: 0 is = 76
Item produced by thread id: 0 is = 83
Item produced by thread id: 0 is = 14
Item produced by thread id: 0 is = 76
Item produced by thread id: 0 is = 16
Item produced by thread id: 0 is = 20
Item produced by thread id: 0 is = 92
Item produced by thread id: 0 is = 25
Item produced by thread id: 0 is = 28
Item produced by thread id: 0 is = 39
Item produced by thread id: 0 is = 25
Item produced by thread id: 0 is = 90
Item produced by thread id: 1 is = 90
Item consumed by thread id: 3 is = 60
Item consumed by thread id: 3 is = 14
Item consumed by thread id: 3 is = 76
Item consumed by thread id: 3 is = 16
Item consumed by thread id: 3 is = 20
Item consumed by thread id: 3 is = 92
Item produced by thread id: 1 is = 60
Item produced by thread id: 1 is = 18
Item produced by thread id: 1 is = 43
Item produced by thread id: 1 is = 37
Item produced by thread id: 1 is = 28
Item consumed by thread id: 0 is = 54
Item consumed by thread id: 0 is = 28
Item consumed by thread id: 0 is = 39
Item consumed by thread id: 0 is = 25
Item produced by thread id: 4 is = 62
Item produced by thread id: 4 is = 21
Item produced by thread id: 4 is = 10
Item produced by thread id: 0 is = 36
Item produced by thread id: 0 is = 55
Item produced by thread id: 3 is = 9
Item consumed by thread id: 4 is = 82
Item consumed by thread id: 4 is = 36
Item consumed by thread id: 4 is = 60
Item produced by thread id: 0 is = 88
Item consumed by thread id: 4 is = 18
Item consumed by thread id: 4 is = 43
Item consumed by thread id: 4 is = 37
Item consumed by thread id: 4 is = 28
Item produced by thread id: 2 is = 82
Item consumed by thread id: 3 is = 25
Item consumed by thread id: 3 is = 21
Item consumed by thread id: 3 is = 10
Item consumed by thread id: 3 is = 55
Item consumed by thread id: 3 is = 88
Item consumed by thread id: 3 is = 70
Item consumed by thread id: 3 is = 25
Item produced by thread id: 4 is = 25
Item consumed by thread id: 4 is = 82
Item produced by thread id: 4 is = 53
Item produced by thread id: 4 is = 8
Item produced by thread id: 4 is = 22
Item produced by thread id: 4 is = 83
Item produced by thread id: 4 is = 50
Item produced by thread id: 4 is = 57
Item produced by thread id: 4 is = 97
Item produced by thread id: 4 is = 27
Item produced by thread id: 4 is = 26
Item produced by thread id: 4 is = 69
Item consumed by thread id: 0 is = 90
Item consumed by thread id: 0 is = 53
Item consumed by thread id: 0 is = 8
Item consumed by thread id: 0 is = 22
Item consumed by thread id: 0 is = 83
Item consumed by thread id: 0 is = 50
Item consumed by thread id: 0 is = 57
Item consumed by thread id: 3 is = 37
Item consumed by thread id: 0 is = 97
Item consumed by thread id: 0 is = 27
Item consumed by thread id: 3 is = 26
Item consumed by thread id: 4 is = 15
Item produced by thread id: 0 is = 70
Item produced by thread id: 0 is = 51
Item produced by thread id: 0 is = 49
Item produced by thread id: 0 is = 10
Item produced by thread id: 0 is = 28
Item produced by thread id: 0 is = 39
Item produced by thread id: 0 is = 98
Item produced by thread id: 0 is = 88
Item produced by thread id: 0 is = 10
Item produced by thread id: 0 is = 93
Item produced by thread id: 0 is = 77
Item produced by thread id: 0 is = 90
Item consumed by thread id: 2 is = 83
Item consumed by thread id: 2 is = 49
Item consumed by thread id: 2 is = 10
Item consumed by thread id: 2 is = 28
Item consumed by thread id: 2 is = 39
Item consumed by thread id: 2 is = 98
Item produced by thread id: 0 is = 76
Item produced by thread id: 0 is = 99
Item produced by thread id: 0 is = 52
Item produced by thread id: 0 is = 31
Item produced by thread id: 0 is = 87
Item produced by thread id: 0 is = 77
Item produced by thread id: 3 is = 15
Item consumed by thread id: 1 is = 62
Item consumed by thread id: 1 is = 10
Item consumed by thread id: 4 is = 51
Item produced by thread id: 0 is = 99
Item produced by thread id: 2 is = 37
Item consumed by thread id: 0 is = 69
Item consumed by thread id: 0 is = 90
Item consumed by thread id: 0 is = 76
Item consumed by thread id: 0 is = 99
Item consumed by thread id: 0 is = 52
Item consumed by thread id: 0 is = 31
Item consumed by thread id: 0 is = 87
Item consumed by thread id: 0 is = 77
Item consumed by thread id: 0 is = 99
Item consumed by thread id: 0 is = 66
Item consumed by thread id: 0 is = 57
Item consumed by thread id: 2 is = 88
Item consumed by thread id: 3 is = 71
Item consumed by thread id: 3 is = 52
Item produced by thread id: 1 is = 82
Item produced by thread id: 1 is = 17
Item consumed by thread id: 2 is = 17
Item consumed by thread id: 4 is = 77
Item produced by thread id: 1 is = 41
Item produced by thread id: 0 is = 66
Item produced by thread id: 1 is = 35
Item consumed by thread id: 0 is = 35
Item consumed by thread id: 3 is = 98
Item consumed by thread id: 1 is = 93
Item consumed by thread id: 4 is = 41
Item produced by thread id: 0 is = 68
Item produced by thread id: 4 is = 71
Item consumed by thread id: 0 is = 84
Item produced by thread id: 4 is = 95
Item consumed by thread id: 0 is = 95
Item consumed by thread id: 3 is = 76
Item produced by thread id: 1 is = 98
Item produced by thread id: 1 is = 5
Item produced by thread id: 1 is = 66
Item produced by thread id: 1 is = 28
Item consumed by thread id: 1 is = 66
Item produced by thread id: 2 is = 52
Item produced by thread id: 2 is = 28
Item produced by thread id: 2 is = 8
Item consumed by thread id: 3 is = 54
Item consumed by thread id: 2 is = 68
Item consumed by thread id: 2 is = 93
Item produced by thread id: 0 is = 84
Item produced by thread id: 0 is = 78
Item produced by thread id: 0 is = 97
Item produced by thread id: 0 is = 55
Item produced by thread id: 0 is = 72
Item produced by thread id: 0 is = 74
Item produced by thread id: 0 is = 45
Item produced by thread id: 0 is = 0
Item produced by thread id: 0 is = 25
Item produced by thread id: 0 is = 97
Item produced by thread id: 0 is = 83
Item produced by thread id: 2 is = 93
Item consumed by thread id: 0 is = 28
Item consumed by thread id: 0 is = 97
Item consumed by thread id: 0 is = 55
Item consumed by thread id: 0 is = 72
Item consumed by thread id: 0 is = 74
Item consumed by thread id: 0 is = 45
Item consumed by thread id: 0 is = 0
Item produced by thread id: 1 is = 54
Item consumed by thread id: 0 is = 25
Item consumed by thread id: 2 is = 78
Item consumed by thread id: 2 is = 83
Item produced by thread id: 4 is = 76
Item consumed by thread id: 2 is = 12
Item consumed by thread id: 2 is = 27
Item produced by thread id: 2 is = 27
Item consumed by thread id: 3 is = 8
Item consumed by thread id: 3 is = 21
Item consumed by thread id: 3 is = 93
Item produced by thread id: 1 is = 82
Item produced by thread id: 1 is = 34
Item consumed by thread id: 3 is = 34
Item produced by thread id: 1 is = 39
Item produced by thread id: 3 is = 57
Item produced by thread id: 1 is = 34
Item produced by thread id: 1 is = 21
Item produced by thread id: 1 is = 85
Item produced by thread id: 1 is = 57
Item produced by thread id: 1 is = 54
Item produced by thread id: 1 is = 61
Item consumed by thread id: 4 is = 5
Item consumed by thread id: 4 is = 34
Item consumed by thread id: 4 is = 21
Item consumed by thread id: 4 is = 59
Item consumed by thread id: 4 is = 85
Item consumed by thread id: 4 is = 57
Item consumed by thread id: 4 is = 54
Item consumed by thread id: 4 is = 61
Item consumed by thread id: 4 is = 62
Item consumed by thread id: 3 is = 39
Item produced by thread id: 4 is = 21
Item produced by thread id: 4 is = 72
Item produced by thread id: 4 is = 41
Item produced by thread id: 4 is = 16
Item produced by thread id: 4 is = 52
Item produced by thread id: 4 is = 50
Item produced by thread id: 4 is = 62
Item produced by thread id: 4 is = 82
Item produced by thread id: 4 is = 99
Item produced by thread id: 4 is = 17
Item produced by thread id: 4 is = 54
Item produced by thread id: 4 is = 73
Item consumed by thread id: 4 is = 41
Item consumed by thread id: 4 is = 16
Item consumed by thread id: 4 is = 52
Item consumed by thread id: 4 is = 50
Item consumed by thread id: 4 is = 62
Item consumed by thread id: 4 is = 82
Item consumed by thread id: 4 is = 99
Item consumed by thread id: 4 is = 17
Item consumed by thread id: 4 is = 54
Item consumed by thread id: 4 is = 73
Item produced by thread id: 2 is = 93
Item produced by thread id: 2 is = 6
Item produced by thread id: 1 is = 62
Item produced by thread id: 1 is = 64
Item produced by thread id: 4 is = 15
Item produced by thread id: 4 is = 63
Item produced by thread id: 3 is = 59
Item produced by thread id: 3 is = 72
Item produced by thread id: 3 is = 37
Item produced by thread id: 3 is = 37
Item produced by thread id: 3 is = 59
Item consumed by thread id: 2 is = 82
Item consumed by thread id: 2 is = 51
Item consumed by thread id: 2 is = 64
Item consumed by thread id: 2 is = 15
Item consumed by thread id: 2 is = 90
Item consumed by thread id: 2 is = 63
Item consumed by thread id: 2 is = 91
Item produced by thread id: 2 is = 51
Item consumed by thread id: 1 is = 28
Item consumed by thread id: 1 is = 37
Item consumed by thread id: 1 is = 37
Item consumed by thread id: 1 is = 59
Item consumed by thread id: 0 is = 97
Item consumed by thread id: 0 is = 71
Item produced by thread id: 1 is = 90
Item consumed by thread id: 3 is = 72
Item produced by thread id: 0 is = 12
Item produced by thread id: 0 is = 87
Item produced by thread id: 0 is = 56
Item produced by thread id: 0 is = 90
Item produced by thread id: 0 is = 41
Item produced by thread id: 0 is = 70
Item produced by thread id: 0 is = 52
Item produced by thread id: 0 is = 65
Item produced by thread id: 0 is = 11
Item produced by thread id: 0 is = 69
Item produced by thread id: 0 is = 17
Item consumed by thread id: 0 is = 87
Item consumed by thread id: 0 is = 56
Item consumed by thread id: 0 is = 90
Item consumed by thread id: 0 is = 41
Item consumed by thread id: 0 is = 70
Item consumed by thread id: 0 is = 52
Item consumed by thread id: 0 is = 65
Item consumed by thread id: 0 is = 11
Item consumed by thread id: 0 is = 69
Item consumed by thread id: 0 is = 17
Item produced by thread id: 3 is = 28
Item consumed by thread id: 2 is = 72
Item produced by thread id: 3 is = 83
Item consumed by thread id: 2 is = 83
Item consumed by thread id: 2 is = 51
Item consumed by thread id: 3 is = 80
Item produced by thread id: 1 is = 80
Item produced by thread id: 1 is = 12
Item produced by thread id: 1 is = 0
Item produced by thread id: 1 is = 6
Item consumed by thread id: 0 is = 61
Item produced by thread id: 3 is = 51
Item produced by thread id: 2 is = 71
Item consumed by thread id: 4 is = 6
Item produced by thread id: 4 is = 91
Item consumed by thread id: 1 is = 28
Item consumed by thread id: 2 is = 12
Item consumed by thread id: 3 is = 0
Item produced by thread id: 0 is = 61
*/