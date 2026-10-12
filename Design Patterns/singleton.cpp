/******************************************************************************
SingleTon Design Pattern
*******************************************************************************/
// #include <bits/stdc++.h>
#include <iostream>
#include <string>
#include <mutex>
using namespace std;

class Logger {
	// single logger instance that should be constant throughout application
	// even in multi-threaded program
	static Logger* instance; // static vars persist for entire application lifecycle
	// in global scope

	static mutex m; // since only 1 global var mutex shared across all threads
	// SingleTon class has private constructor
	Logger() {
		cout<<"Logger Initialised\n";
	}
	~Logger() {
		cout<<"logger destructed\n";
	}
public:
	static Logger* getLoggerInstance() {
		if(instance==nullptr)
		{
			// initialisation not yet done
			lock_guard<mutex> lock(m);
			if(instance==nullptr)
			{
				// re-check is needed if multiple threads are in queue of acquiring
				// lock , the 2nd and rest threads must not create a new instance after
				// 1st thread which enters here has already created logger instance
				instance=new Logger();
				return instance;
			}
		}
		return instance;
	}
	void logInfo(string info) {
		cout<<"[INFO]"<<info;
	}
	void logError(string err) {
		cout<<"[ERROR]"<<err;
	}
};
Logger* Logger::instance=nullptr;
mutex Logger::m;
int main()
{
	cout<<"Hello World";

	Logger* logger1=Logger::getLoggerInstance();
	logger1->logInfo("logger1 Info");
	Logger* logger2=Logger::getLoggerInstance();
	logger2->logError("logger2 Error");
	
	if(logger1==logger2)
	{
	    cout<<"\ncuz of SingleTon, both loggers point to same memory location as expected\n";
	}

	return 0;
}
/*
Hello WorldLogger Initialised
[INFO]logger1 Info[ERROR]logger2 Error
cuz of SingleTon, both loggers point to same memory location as expected
*/