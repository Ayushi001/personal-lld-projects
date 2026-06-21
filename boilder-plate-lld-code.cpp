/******************************************************************************

Welcome to GDB Online.
GDB online is an online compiler and debugger tool for C, C++, Python, Java, PHP, Ruby, Perl,
C#, OCaml, VB, Swift, Pascal, Fortran, Haskell, Objective-C, Assembly, HTML, CSS, JS, SQLite, Prolog.
Code, Compile, Run and Debug online from anywhere in world.

*******************************************************************************/
#include <iostream>
#include<bits/stdc++.h>
using namespace std;

class StrategyInterface {
public:
	virtual void assign()=0;
};

class OptimalStrategy:public StrategyInterface {
public:
	OptimalStrategy() {}
	void assign() override {
		cout<<"OptimalStrategy\n";
	}
};

class RandomStrategy:public StrategyInterface {
public:
	RandomStrategy() {}
	void assign() override {
		cout<<"RandomStrategy\n";
	}
};

class ThreadSafeStrategy: public StrategyInterface {
	int assignmentCnt;
	// mutex is specific to this class
	mutex m;
public:
	ThreadSafeStrategy() {
		assignmentCnt=0;
	}
	void assign() override {
		// lock_guard locks the mutex when assign() is called and releases lock automatically
		// when scope of assign() over
		lock_guard<mutex> lock(m);
		assignmentCnt++;
		cout<<"Thread-Safe:"<<assignmentCnt<<'\n';
	}
};

class SharedMutexStrategy:public StrategyInterface {
	int assignmentCnt;
	shared_mutex m;
public:
	SharedMutexStrategy() {
		assignmentCnt=0;
	}
	//READ operation
	void logAssignmentCnt() {
		// shared_lock occupied on shared_mutex m so multiple threads can
		// read on the assignmentCnt value and enter this func
		shared_lock<shared_mutex> readLock(m);
		cout<<assignmentCnt<<'\n';
	}
	// WRITE operation
	void assign() override
	{
		// a thread entering this func will occupy a write lock and to get it
		// no other thread should be reading the value/ no other thread should
		// have acquired shared_lock in logAssignmentCnt()
		unique_lock<shared_mutex> writeLock(m);
		assignmentCnt++;
	}
};

int main()
{
	StrategyInterface* ostrategy= new OptimalStrategy();
	StrategyInterface* rstrategy=new RandomStrategy();

	ostrategy->assign();
	rstrategy->assign();

	StrategyInterface *tsstrategy= new ThreadSafeStrategy();
	tsstrategy->assign();

	SharedMutexStrategy* rwstrategy=new SharedMutexStrategy();
	rwstrategy->logAssignmentCnt();
	rwstrategy->assign();
	rwstrategy->logAssignmentCnt();

	return 0;
}