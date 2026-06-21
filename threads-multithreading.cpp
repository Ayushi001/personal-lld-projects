#include <iostream>
#include <thread>

using namespace std;

// function that worker thread will run
void workerTask(string threadName,int count)
{
    cout<<"Worker Thread execution\n";
    for(int i=0;i<count;i++)
    {
        cout<<threadName<<" i: "<<i<<'\n';
    }
}
int main()
{
    cout<<"Main thread execution\n";
    // a background worker thread gets created immediately when this line is executed
    thread worker(workerTask,"Worker Thread",5);
    
    for(int i=0;i<6;i++)
    {
        cout<<"MAIN THREAD i:"<<i<<'\n';
    }
    
    if(worker.joinable())
    {
        // here main thread just stops and waits for the background thread to 
        // finish execution before main thread exits, this is a blocking call
        worker.join();
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
Worker Thread i: 0
Worker Thread i: 1
Worker Thread i: 2
Worker Thread i: 3
Worker Thread i: 4
*/