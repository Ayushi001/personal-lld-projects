/******************************************************************************
Rubrik System Coding, question link: 
https://enginebogie.com/interview/experience/rubrik-software-development-engineer-2/220?srsltid=AfmBOor8SLx-XZDQoRA1CGu1PcJRJL41Xku4mKEVPvyhogFSxzyMz7K_
GEMINI discussion link: https://gemini.google.com/share/01283dd0ff24

1280. Multithreaded Web Crawler
Implement a multi-threaded web crawler. The crawler starts from a given URL and uses multiple threads to explore all pages accessible from this starting point.
Each URL points to a webpage, and the crawling process retrieves all URLs present on a page to visit them recursively.

The system will call the crawl with startUrl and you will have to block the calling thread until all the URLs are parsed and then return those URLs.

Signature:

class WebCrawler {
    List<String> crawl(String startUrl) {
        // Your code here
    }
}
Requirements:

The crawling process should:
Avoid revisiting the same URL more than once.
Crawl only pages belonging to the same hostname as the starting URL.

Multithreading:
Utilize multiple threads to speed up the crawling process.
Ensure thread safety to handle shared data structures (e.g., visited URLs).

Constraints:
startUrl is guaranteed to belong to the same hostname for the crawling process.
URL comparisons should be case-sensitive and exact.

Key Considerations:
Thread Safety: Design the solution to avoid race conditions while accessing shared resources like the visited URLs set.
Hostname Validation: Use the hostname of the startUrl to filter URLs that do not belong to the same domain.
Concurrency: Efficiently distribute the crawling workload across threads to optimize performance.
*******************************************************************************/
#include <iostream>
#include <bits/stdc++.h>
using namespace std;

class WebCrawler
{
    // Shared rescources among all threads:
    queue<string> urlQueue; // queue of URLs to crawl from
    int activeWorkers = 0;
    unordered_set<string> visited; // keeps track of visited urls
    bool done = false;             // when no longer urls are left to be crawled

    mutex mtx; // to acquire lock on shared resources
    condition_variable cv;
    // to make a thread sleep when waiting for some operation to finish, eg:
    // when waiting for some new url to be pushed into the queue, instead of
    // continously checking using while(true){ if( queue.empty()? }  and wasting
    // cpu cycles, just put the thread to sleep and when some other thread pushes
    // a new url into the shared queue, it notifies one sleeping thread to wake up
    // using cv.notifyOne() or cv.notifyAll() -> if all sleeping threads need to be
    // woken up eg when no longer urls are left to be crawled, but cv.notifyAll()
    // should be used in critical conditions only else waking all sleeping threads
    // unnecessarily when only 1 thread was needed could lead to a thundering herd problem

    // helper function to let a thread safely grab a url from the shared urlQueue
    // WRITE THIS AT END OF INTERVIEW, START WITH RAW {} CODE BLOCKS FIRST - faster
    // 	bool fetchUrlSafe(&string currentUrl)
    // 	{
    // 		unique_lock<mutex> lock(mtx);
    // 		while(urlQueue.empty() && !done)
    //      ...
    // 	}

    // the common func that each background worker thread will execute
    void workerTask(unordered_map<string, vector<string>> web)
    {
        string currentUrl = "";
        while (true)
        {

            // STEP 1: grab the URL from queue
            {
                // unique_lock is needed instead of lock_guard as cv.wait internally
                // releases the lock using .unlock() func which only unique_lock offers
                // while lock_guard can't manually release lock mid-scope
                unique_lock<mutex> lock(mtx);
                while (urlQueue.empty() && !done)
                {
                    // for any thread if url queue is empty and the crawling process
                    // isn;t completed then just sleep
                    cv.wait(lock);
                }
                if (done)
                {
                    // just exit gracefully as no more urls left to crawl, all are visited
                    return;
                }
                // grab the url
                currentUrl = urlQueue.front();
                urlQueue.pop();
                activeWorkers++;
            }

            // STEP 2: iterate over web to get its unvisted child urls which can be crawled
            {
                // lock_guard suffices here as we don't need cv.wait(lock)
                lock_guard<mutex> lock(mtx);
                string targetHost = getHostName(currentUrl);
                for (auto &url : web[currentUrl])
                {
                    string domain = getHostName(url);
                    if (domain != targetHost)
                        continue;

                    if (visited.find(url) == visited.end())
                    {
                        visited.insert(url);
                        urlQueue.push(url);
                    }
                }
                activeWorkers--;

                // check for exit steps
                if (urlQueue.empty() && activeWorkers == 0)
                {
                    // all urls are now crawled
                    done = true;
                    cv.notify_all(); // wake up all sleeping threads so they can exit gracefully
                    return;
                }
                if (!urlQueue.empty())
                {
                    cv.notify_one();
                    // u got some unprocessed urls in the queue, wake up ONE sleeping thread
                }
            } // unlocks the lock
        }
    }

    // helper function to get domain name
    string getHostName(string url)
    {
        // for input: https://https://abc.com/xyz , return https://abc.com
        string host;
        int slashes = 0;
        bool colonFound = false;
        for (auto ch : url)
        {
            if (!colonFound && ch != ':')
                continue;
            if (ch == ':')
            {
                colonFound = true;
                continue;
            }
            if (ch == '/')
            {
                slashes++;
                if (slashes == 3)
                    return host;
                continue;
            }
            host += ch;
        }
        return host;
    }

public:
    WebCrawler() {}
    vector<string> crawl(string startUrl, unordered_map<string, vector<string>> web)
    {
        // main thread
        // do a precautionary clean-up
        while (!urlQueue.empty())
        {
            urlQueue.pop();
        }
        visited.clear();
        activeWorkers = 0;
        done = false;

        // single-threaded till now only main thread is running
        urlQueue.push(startUrl);
        visited.insert(startUrl);

        // spawn up 4 workers
        vector<thread> workers;
        // create 4 background worker threads to crawl the web
        for (int i = 0; i < 4; i++)
        {
            // thread worker(&WebCrawler::workerTask, this, web);
            // the above line immediately creates 1 worker that starts running
            // the func passed in its input, since its a private member func and
            // not a global func, it needs an object instance this to run this func on
            workers.push_back(thread(&WebCrawler::workerTask, this, web));
        }

        // make the main thread sleep
        {
            // use unique_lock when using condition_variable
            unique_lock<mutex> lock(mtx);
            while (!done)
            {
                cv.wait(lock);
            }
        }

        // once the done=true, main thread wakes up
        for (int i = 0; i < 4; i++)
        {
            if (workers[i].joinable())
            {
                // wait for all background worker threads to finish and join back
                // to main thread, following is a blocking call will wait
                // till the thread finish
                workers[i].join();
            }
        }

        return vector<string>(visited.begin(), visited.end());
    }
};

int main()
{
    cout << "Hello World\n";

    WebCrawler *crawler = new WebCrawler();
    unordered_map<string, vector<string>> web;
    web["https://abc.com"] = {"https://abc.com/home", "https://abc.com/about"};
    web["https://abc.com/about"] = {"https://abc.com/about/contact", "https://xyz.com/home"};

    vector<string> urls = crawler->crawl("https://abc.com", web);

    for (auto url : urls)
    {
        cout << url << '\n';
    }
    return 0;
}
/*
OUTPUT:
Hello World
https://abc.com/about/contact
https://abc.com/about
https://abc.com/home
https://abc.com
*/