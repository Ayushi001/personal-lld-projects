/******************************************************************************
Chain of Responsibility Design Pattern
example: https://codewitharyan.com/tech-blogs/chain-of-responsibiliy-design-pattern
*******************************************************************************/
#include <bits/stdc++.h>
using namespace std;

// Singleton Logger class to log info and error messages
class Logger {
	static Logger* instance;
	static mutex m;
	Logger() {}
public:
	static Logger* getLoggerInstance() {
		if(instance==NULL) {
			lock_guard<mutex> lock(m);
			if(instance==NULL) {
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
// initialise memory at global space
Logger* Logger::instance=NULL;
mutex Logger::m;

// leave approver interface/ abstract class
class LeaveHandler {
	// protected so only its derived classes can use it
protected:
	LeaveHandler* nextHandler;

public:
	LeaveHandler() {
		nextHandler=NULL;
	}
	~LeaveHandler()=default;
	virtual void processLeave(int leaves)=0;

	void setNextHandler(LeaveHandler* nextHandler) {
		this->nextHandler=nextHandler;
	}
};
class Supervisor:public LeaveHandler {
public:
	void processLeave(int leaves) override {
		if(leaves<3)
		{
			Logger::getLoggerInstance()->logInfo("leaves handled by Supervisor");
			return;
		}
		// else pass the responsibility to next next handler
		if(nextHandler)
			nextHandler->processLeave(leaves);
		else
			Logger::getLoggerInstance()->logError("Request can't be handled");
	}
};
class Manager:public LeaveHandler {
public:
	void processLeave(int leaves) override {
		if(leaves<10) {
			Logger::getLoggerInstance()->logInfo("leaves handled by manager");
			return;
		}
		if(nextHandler)
			nextHandler->processLeave(leaves);
		else
			Logger::getLoggerInstance()->logError("Request can't be handled");
	}
};
// ultimate handler
class HR:public LeaveHandler {
public:
	void processLeave(int leaves)override {
		Logger::getLoggerInstance()->logInfo("leaves handled by HR");
	}
};

int main()
{
	LeaveHandler* supervisor=new Supervisor();
	LeaveHandler* manager=new Manager();
	LeaveHandler* hr=new HR();

	supervisor->setNextHandler(manager);
	manager->setNextHandler(hr);

	supervisor->processLeave(20);
	supervisor->processLeave(5);
	supervisor->processLeave(1);
	return 0;
}
/*
[INFO]leaves handled by HR[INFO]leaves handled by manager[INFO]leaves handled by Supervisor
*/