#include <iostream>
#include <string>
#include <list>
#include <vector>
#include <chrono>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <unistd.h>
#include "configreader.h"
#include "process.h"
#include "semaphore.h"

// Shared data for all cores
typedef struct SchedulerData {
    std::mutex mutex;
    std::condition_variable condition;
    Semaphore *semaphore;
    ScheduleAlgorithm algorithm;
    uint32_t context_switch;
    uint32_t time_slice;
    std::list<Process*> ready_queue;
    bool all_terminated;
} SchedulerData;

void coreRunProcesses(uint8_t core_id, SchedulerData *data);
int printProcessOutput(std::vector<Process*>& processes, std::mutex& mutex);
void clearOutput(int num_lines);
uint32_t currentTime();
std::string processStateToString(Process::State state);

int main(int argc, char **argv)
{
    // ensure user entered a command line parameter for configuration file name
    if (argc < 2)
    {
        std::cerr << "Error: must specify configuration file" << std::endl;
        exit(1);
    }

    // declare variables used throughout main
    int i;
    SchedulerData *shared_data;
    std::vector<Process*> processes;
    bool done = false;
    
    // read configuration file for scheduling simulation
    SchedulerConfig *config = readConfigFile(argv[1]);

    // store configuration parameters in shared data object
    uint8_t num_cores = config->cores;
    shared_data = new SchedulerData();
    shared_data->algorithm = config->algorithm;
    shared_data->context_switch = config->context_switch;
    shared_data->time_slice = config->time_slice;
    shared_data->all_terminated = false;
    shared_data->semaphore = new Semaphore(0);
    // create processes
    uint32_t start = currentTime();
    for (i = 0; i < config->num_processes; i++)
    {
        Process *p = new Process(config->processes[i], start);
        processes.push_back(p);
        if (p->getState() == Process::State::Ready)
        {
            shared_data->ready_queue.push_back(p);
	    shared_data->semaphore->signal();
        }
    }

    // free configuration data from memory
    deleteConfig(config);

    // launch 1 scheduling thread per cpu core
    std::thread *schedule_threads = new std::thread[num_cores];
    for (i = 0; i < num_cores; i++)
    {
        schedule_threads[i] = std::thread(coreRunProcesses, i, shared_data);
    }
    // main thread work goes here:
    int num_lines = 0;
    while (!(shared_data->all_terminated))
    {
        // clear output from previous iteration
        clearOutput(num_lines);

        // start new processes at their appropriate start time

	//@@Check through the NotStarted Processes
	//@@See if time
	{
	  
	  std::lock_guard<std::mutex> lock(shared_data->mutex);
	  for(i = 0 ; i < processes.size(); i++){
	    //**Lauch a process
	    if((currentTime() - start) >= processes[i]->getStartTime() && processes[i]->getState() == Process::State::NotStarted){
	      processes[i]->setState(Process::State::Ready, currentTime());
	      //Add process to end of the queue
	      shared_data->ready_queue.push_back(processes[i]);
	      shared_data->semaphore->signal();
	    }//if not started
	  }//For every process
	}//lock shared data
	
	
	
	
        // determine when an I/O burst finishes and put the process back in the ready queue
		//@@Check through the NotStarted Processes
	//@@See if time
	{
	  
	  std::lock_guard<std::mutex> lock(shared_data->mutex);
	  for(i = 0 ; i < processes.size(); i++){
	   //*Lauch a process
	    uint16_t burst_counter = processes[i]->getCurrentBurst();
	    processes[i]->updateProcess(currentTime());
	    if(burst_counter != processes[i]->getCurrentBurst() && processes[i]->getState() == Process::State::IO){
	      processes[i]->setState(Process::State::Ready, currentTime());
	      //Add process to end of the queue
	      shared_data->ready_queue.push_back(processes[i]);
	      shared_data->semaphore->signal();
	    }//if IO//*/
	  }//For every process
	}//lock shared data
	
        // sort the ready queue (if needed - based on scheduling algorithm)
	
        // determine if all processes are in the terminated state
	{
	  done = true;
	  int count = 0;
	  for(i = 0; i < processes.size() && done == true; i++){
	    if(processes[i]->getState() != Process::State::Terminated){
	      done = false;
	    }
	  }
	  shared_data->all_terminated = done;
	  //	  std::cout<<"Checking end state: "<<done<<std::endl;
	}//*/

	
        // output process status table
	
        num_lines = printProcessOutput(processes, shared_data->mutex);

        // sleep 1/60th of a second
        usleep(16667);
    }

    //Wait up the cores once they finished
    for(i = 0; i < num_cores; i++){
      shared_data->semaphore->signal();
    }

    std::cout<<"All done in main thread"<<std::endl;
    // wait for threads to finish
    for (i = 0; i < num_cores; i++)
    {
        schedule_threads[i].join();
    }
    
    // print final statistics
    //  - CPU utilization
    //  - Throughput
    //     - Average for first 50% of processes finished
    //     - Average for second 50% of processes finished
    //     - Overall average
    //  - Average turnaround time
    //  - Average waiting time


    // Clean up before quitting program
    processes.clear();

    return 0;
}//main()

void coreRunProcesses(uint8_t core_id, SchedulerData *shared_data)
{
  uint32_t context_switch;
  uint32_t time_slice;
  ScheduleAlgorithm algorithm;
  uint32_t cpu_burst_time;
  uint32_t start_cpu_time;
  Process *runningProcess;
  uint8_t burst_counter;
  
  //* CRITICAL SECTION: GET INITIAL CONFIGURATION
  context_switch = shared_data->context_switch;
  time_slice = shared_data->time_slice;
  algorithm = shared_data->algorithm;
  
  
  //@@ Use Mutex to gain access to shared_data
  while(!shared_data->all_terminated){
    //      printf("Begin Core run\n");
    //while not done
    shared_data->semaphore->wait();
    if(!shared_data->all_terminated){
      {//* CRITICAL SECTION GET FROM READ QUEUE
	
	// Work to be done by each core idependent of the other cores
	//  - Get process at front of ready queue
	std::lock_guard<std::mutex> lock(shared_data->mutex);
	if(shared_data->ready_queue.size() == 0);
	runningProcess = shared_data->ready_queue.front();
	//  - Remove the entry from the queue
	shared_data->ready_queue.pop_front();
	// printf("Grabbing from queue\n");
	//*Update process core and state
	runningProcess->setCpuCore(core_id);
	runningProcess->updateProcess(currentTime());
	runningProcess->setState(Process::State::Running, currentTime());
	cpu_burst_time = runningProcess->getBurstTime();
	burst_counter = runningProcess->getCurrentBurst();
      }
      
      start_cpu_time = currentTime();
      
      //  - Simulate the processes running until one of the following:
      //     - CPU burst time has elapsed
      //     - RR time slice has elapsed
      //     - Process preempted by higher priority process
      
      if(algorithm == ScheduleAlgorithm::FCFS){
	while(currentTime() - start_cpu_time < cpu_burst_time);
	//std::cout << "Finished something"<<std::endl;
      }    
      else if(algorithm == ScheduleAlgorithm::RR){
	while(currentTime() - start_cpu_time < cpu_burst_time);
      }
      else if(algorithm == ScheduleAlgorithm::SJF){
	while(currentTime() - start_cpu_time < cpu_burst_time);
      }
      else if(algorithm == ScheduleAlgorithm::PP){
	while(currentTime() - start_cpu_time < cpu_burst_time);
      }
      

      //  - Place the process back in the appropriate queue
      //     - I/O queue if CPU burst finished (and process not finished)
      //     - Terminated if CPU burst finished and no more bursts remain
      //     - Ready queue if time slice elapsed or process was preempted
      {//* CRITICAL SECTION GET FROM READ QUEUE
	//*Update process core and state
	std::lock_guard<std::mutex> lock(shared_data->mutex);
	runningProcess->setCpuCore(-1);
	runningProcess->updateProcess(currentTime());
	//Terminate
	if(runningProcess->getCurrentBurst() >= runningProcess->getNumBursts()){
	  runningProcess->setState(Process::State::Terminated, currentTime());
	}
	//Finished CPU Burst
	//*
	else if(burst_counter != runningProcess->getCurrentBurst()){
	  runningProcess->setState(Process::State::IO, currentTime());
	}//*/
	//If preempted
	else{
	   runningProcess->setState(Process::State::Ready, currentTime());
	   shared_data->ready_queue.push_back(runningProcess);
	   shared_data->semaphore->signal();
	}
      }
      
      //      printf("done deal\n");    
      //  - Wait context switching time
      usleep(context_switch);
      //  * Repeat until all processes in terminated state
    }//If not terminated (checked after wait)
  }//while ! done//*/
}

int printProcessOutput(std::vector<Process*>& processes, std::mutex& mutex)
{
    int i;
    int num_lines = 2;
    std::lock_guard<std::mutex> lock(mutex);
    printf("|   PID | Priority |      State | Core | Turn Time | Wait Time | CPU Time | Remain Time |\n");
    printf("+-------+----------+------------+------+-----------+-----------+----------+-------------+\n");
    for (i = 0; i < processes.size(); i++)
    {
        if (processes[i]->getState() != Process::State::NotStarted)
        {
            uint16_t pid = processes[i]->getPid();
            uint8_t priority = processes[i]->getPriority();
            std::string process_state = processStateToString(processes[i]->getState());
            int8_t core = processes[i]->getCpuCore();
            std::string cpu_core = (core >= 0) ? std::to_string(core) : "--";
            double turn_time = processes[i]->getTurnaroundTime();
            double wait_time = processes[i]->getWaitTime();
            double cpu_time = processes[i]->getCpuTime();
            double remain_time = processes[i]->getRemainingTime();
            printf("| %5u | %8u | %10s | %4s | %9.1lf | %9.1lf | %8.1lf | %11.1lf |\n", 
                   pid, priority, process_state.c_str(), cpu_core.c_str(), turn_time, 
                   wait_time, cpu_time, remain_time);
            num_lines++;
        }
    }
    return num_lines;
}

void clearOutput(int num_lines)
{
    int i;
    for (i = 0; i < num_lines; i++)
    {
        fputs("\033[A\033[2K", stdout);
    }
    rewind(stdout);
    fflush(stdout);
}

uint32_t currentTime()
{
    uint32_t ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                  std::chrono::system_clock::now().time_since_epoch()).count();
    return ms;
}

std::string processStateToString(Process::State state)
{
    std::string str;
    switch (state)
    {
        case Process::State::NotStarted:
            str = "not started";
            break;
        case Process::State::Ready:
            str = "ready";
            break;
        case Process::State::Running:
            str = "running";
            break;
        case Process::State::IO:
            str = "i/o";
            break;
        case Process::State::Terminated:
            str = "terminated";
            break;
        default:
            str = "unknown";
            break;
    }
    return str;
}
