#include <iostream>
#include <thread>
#include <vector>
#include "httplib.h"

using namespace std;

void download(vector<string> & data, int id, int worker_cnt, int N)
{
  unsigned int BUFFSIZE = 2048;
  char buffer[BUFFSIZE];

  httplib::Client cli("http://name-service.appspot.com");
  for (int i = id; i < N; i += worker_cnt)
  {
    snprintf(buffer, BUFFSIZE, "/api/v1/names/%i.xml", i);
    
    auto res = cli.Get(buffer);

    // DO SOME PARSING MAGIC ... 

    data[id] = res->body;
  }
}

int main() 
{
  int worker_cnt = thread::hardware_concurrency();
  cout << worker_cnt << endl;

  unsigned int N = 16;

  vector<string> data;
  vector<thread> workers;
  
  data.resize(N);
  
  for (int id = 0; id < worker_cnt; ++id)
  {
     workers.push_back(thread(download, std::ref(data), id, worker_cnt, N));
  }
 
  for (thread& worker : workers)
  {
    worker.join();
  }

  for (string s : data)
  {
     cout << s << endl;
  }

  return 0;
}