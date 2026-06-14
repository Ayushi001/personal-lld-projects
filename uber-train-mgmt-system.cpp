/*
Design a Train-Platform Management System with functionalities:

Assign trains to platforms based on input.

Query which train is at a given platform at a specific time.

Query which platform a train is at, at a specific time.
*/

/*
- Train : trainId
- platform : platformId
- Schedule: trainId, arrivalTime,departureTime,platformId
- ScheduleManager: has ds for current schedules, and assigns an incoming req, and has query APIs
   For query which train is at a given platform at a specific time:
   - unordered_map<int,vector<Schedule>> : <platformId,sorted vector of schedules assigned to
                                          that platform based on arrivalTime>
   For query which platform a train is at, at a specific time:
   - unordered_map<int,vector<Schedule>> : <trainId, sorted vector of schedules the tain is having
                                            of which platforms to be at based on arrivalTime
- AssignmentStrategy :
  - optimal platform asisgnment strategy for a trainId request based on platform which gets
    free the earliest - maintain minHeap {endTime,platformId} to get earliest free platform
  - random strategy - assigns a random platform which is free at the requested time
*/
#include <iostream>
#include<bits/stdc++.h>

using namespace std;

class Train {
public:
	int trainId;
	Train(int id) {
		trainId=id;
	}
};
class Platform {
public:
	int platformId;
	Platform(int id) {
		platformId=id;
	}
};
class Schedule {
public:
	int arrivalTime,departureTime,trainId,platformId;
	Schedule(int a,int d,int t,int p)
	{
		arrivalTime=a,departureTime=d,trainId=t,platformId=p;
	}
	// strict ordering is required as we are storing set<Schedule> so we need to
	// make it clear how to distinguish each schedule, as set silenty drops 2 schedules
	// of same value
	bool operator<(const Schedule &b) const
	{
		// strict ordering based on each variable
		if(this->arrivalTime!=b.arrivalTime)
			return this->arrivalTime<b.arrivalTime;
		if(this->departureTime!=b.departureTime)
			return this->departureTime<b.departureTime;
		if(this->trainId!=b.trainId)
			return this->trainId<b.trainId;
		return this->platformId<b.platformId;
	}
};
class AssignmentStrategyI {
public:
	// returns the assigned platform ID for given train request
	virtual int assign(int trainId,int startTime,int departureTime,
	                   unordered_map<int,set<Schedule>> &platformSchedules, int totalPlatforms)=0;
};
class OptimisedAssignment:public AssignmentStrategyI {
public:
	// return the earliest free platform
	int assign(int trainId,int startTime,int departureTime,
	           unordered_map<int,set<Schedule>> &platformSchedules, int totalPlatforms) override {
		//pq of (endTime,platformID)
		priority_queue<pair<int,int>, vector<pair<int,int>>, greater<pair<int,int>> >minHeap;
		for(int i=0; i<totalPlatforms; i++)
		{
			if(platformSchedules.find(i)==platformSchedules.end())
			{
				minHeap.push({0,i});
				continue;
			}
			// rbegin -> reversed begin for back() /pointer to last element in set
			set<Schedule> &schedules=platformSchedules[i];
			if(schedules.empty())
			{
				// empty set
				minHeap.push({0,i});
				continue;
			}
			auto schedule=schedules.rbegin();
			// 			if(schedule==schedules.rend())
			// 			{
			// 			    // empty set
			// 			    minHeap.push({0,i});
			// 				continue;
			// 			}
			//since vector<schedule> for each platformId is sorted, only last schedule has the chance
			// that its still not departed
			minHeap.push({schedule->departureTime,schedule->platformId});
		}

		// get the earliest free platform
		auto p=minHeap.top();
		if(p.first>startTime)
		{
			return -1; // no platform free
		}
		return p.second;
	}
};

class ScheduleManager {
public:
	vector<Schedule> schedules;
	unordered_map<int,set<Schedule>> platformSchedules; // platformId, list of sorted schedules
	unordered_map<int,set<Schedule>> trainSchedules;
	AssignmentStrategyI* strategy;
	int totalPlatforms;
	mutex scheduleMutex;

	ScheduleManager( vector<Schedule> schedules,AssignmentStrategyI*strategy, int totalPlatforms)
	{
		this->schedules=schedules;
		for(auto schedule:schedules)
		{
			platformSchedules[schedule.platformId].insert(schedule);
			trainSchedules[schedule.trainId].insert(schedule);
		}
		// either make it set<Schedule> instead of vector<Schedule> to keep it sorted on arrivalTime
		this->strategy=strategy;
		this->totalPlatforms=totalPlatforms;
	}
	// prone to race condition
	void assignTrain(int trainId,int startTime, int departureTime)
	{
		lock_guard<mutex> lock(scheduleMutex);

		int platformId=strategy->assign(trainId,startTime,departureTime,platformSchedules,totalPlatforms);
		if(platformId==-1)
		{
			cout<<"Not possible";
			return;
		}
		// create a new schedule for the platform for given train req and update the maps
		Schedule newSchedule= Schedule(startTime,departureTime,trainId,platformId);

		//1.
		// instead of inserting at end, insest such that the vectors in unordered_map remain sorted
		// platformSchedules[platformId].push_back(newSchedule);
		// trainSchedules[trainId].push_back(newSchedule);

		//2.
		// 		vector<Schedule> &v=platformSchedules[platformId];
		// 		auto it=upper_bound(v.begin(),v.end(),newSchedule);
		// 		// it points to location where newSchedule is JUST greater than last value
		// 		v.insert(it, newSchedule);

		// 		vector<Schedule> &p=trainSchedules[trainId];
		// 		it=upper_bound(p.begin(),p.end(),newSchedule);
		// 		// it points to location where newSchedule is JUST greater than last value
		// 		p.insert(it, newSchedule);

		platformSchedules[platformId].insert(newSchedule);
		trainSchedules[trainId].insert(newSchedule);
	}
	// Query which train is at a given platform at a specific time.
    // queries can be further optimised using a shared_mutex with shared_lock for queries and unique_lock for updates,
    // as queries are more frequent than updates
	int queryPlatform(int platformId, int timestamp)
	{
		// assuminng vector<Schedule> sorted on arrivalTime, return 1st schedule which has
		// arrivalTime JUST smaller than timestamp -> upper_bound - 1
		set<Schedule> &s=platformSchedules[platformId];
		Schedule target(timestamp, INT_MAX,INT_MAX,INT_MAX);
		// 		auto it=upper_bound(v.begin(),v.end(), target );
		auto it= s.upper_bound(target);
		if(it==s.begin())
			return -1; // no train at platform at given timestamp
		it--;
		if(it->departureTime<timestamp)
			return -1; // the last train with arrival time JUST smaller than target timestamp just departed
		return it->trainId;
	}
};
int main()
{
	AssignmentStrategyI *strategy=new OptimisedAssignment();
	ScheduleManager testManager({},strategy,3);
	testManager.assignTrain(0,1,3);
	testManager.assignTrain(1,1,3);
	testManager.assignTrain(2,2,4);

	cout<<testManager.queryPlatform(0, 2)<<'\n';
	cout<<testManager.queryPlatform(1, 2)<<'\n';
	cout<<testManager.queryPlatform(2, 5)<<'\n';

	return 0;
}