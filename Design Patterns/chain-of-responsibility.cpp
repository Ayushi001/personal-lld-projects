/******************************************************************************
Chain of Responsibility Design Pattern
example: https://codewitharyan.com/tech-blogs/chain-of-responsibiliy-design-pattern
*******************************************************************************/
#include <bits/stdc++.h>
using namespace std;
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
			cout<<"[INFO] leaves handled by Supervisor";
			return;
		}
		// else pass the responsibility to next next handler
		nextHandler->processLeave(leaves);
	}
};
class Manager:public LeaveHandler {
public:
	void processLeave(int leaves) override {
		if(leaves<10) {
			cout<<"[INFO] leaves handled by Manager";
			return;
		}
		nextHandler->processLeave(leaves);
	}
};
// ultimate handler
class HR:public LeaveHandler {
public:
	void processLeave(int leaves)override {
		cout<<"[INFO] leaves handled by HR";
	}
};

int main()
{
	std::cout<<"Hello World";
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
Hello World[INFO] leaves handled by HR[INFO] leaves handled by Manager[INFO] leaves handled by Supervisor
*/