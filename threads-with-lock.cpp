#include <iostream>
#include <thread>

using namespace std;

// global mutex for cout operations by main and worker thread
mutex mtx; 

// function that worker thread will run
void workerTask(string threadName,int count)
{
    cout<<"Worker Thread execution\n";
    for(int i=0;i<count;i++)
    {
        // acquire lock over console
        lock_guard<mutex> lock(mtx);
        cout<<threadName<<" i: "<<i<<'\n';
    }
}
int main()
{
    cout<<"Main thread execution\n";
    // a background worker thread gets created immediately when this line is executed
    thread worker1(workerTask,"Worker Thread 1",5);
    thread worker2(workerTask,"Worker Thread 2",5);
    
    for(int i=0;i<6;i++)
    {
        lock_guard<mutex> lock(mtx);
        cout<<"MAIN THREAD i:"<<i<<'\n';
    }
    
    if(worker1.joinable())
    {
        // here main thread just stops and waits for the background thread to 
        // finish execution before main thread exits, this is a blocking call
        worker1.join();
    }
     if(worker2.joinable())
    {
        worker2.join();
    }
    return 0;
}
/*
OUTPUT:
Main thread execution
MAIN THREAD i:0
MAIN THREAD i:1
MAIN THREAD i:2
MAIN THREAD i:3
MAIN THREAD i:4
MAIN THREAD i:5
Worker Thread execution
Worker Thread 2 i: 0
Worker Thread 2 i: 1
Worker Thread 2 i: 2
Worker Thread 2 i: 3
Worker Thread 2 i: 4
Worker Thread execution
Worker Thread 1 i: 0
Worker Thread 1 i: 1
Worker Thread 1 i: 2
Worker Thread 1 i: 3
Worker Thread 1 i: 4
*/
