# CS528 Project- Trust Aware Scheduling of Online Tasks in FoG Nodes
Ridhiman Kaur Dhindsa, 210101088  
Aditya Gupta, 210101009  
Date: 16 April 2024  

## Files included
*scheduler.cpp, scheduler_approach2.cpp, Readme.md, report.docx*  

`scheduler.cpp` is the original file used as a benchmark for comparing with various testcases. It uses approach-1 scheduling as discussed in the report. To change any parameters, change the values of the global variables listed at the top of the cpp file. The comments are self-explanatory, indicating the purpose served by each variable.  

`scheduler_approach2.cpp` uses approach-2 scheduling policy as discussed in the report. It begins with the default values used in approach-1, case-1. To change any parameters, change the values of the global variables listed at the top of the cpp file. The comments are self-explanatory, indicating the purpose served by each variable.   

## Steps to run
Compile and run on an Ubuntu terminal:  
1) Open the terminal in the project directory.  
2) Compile using: `g++ scheduler.cpp -o scheduler`  
3) Run using: `./scheduler`  

## Output
The output consists of each task-node assignment being printed to the console, representing the task and its information such as execution time, user id etc, as well as the node assigned and completion information such as new DT value, new ST value, finish time etc. Mulitple tasks could arrive at the same time. The arrival and execution times are designed to simulate a natural and random manner through the Poisson distribution.

This is followed by the FOG system report which includes:  
* uptime- total time the system ran for  
* idle time- time for which no task is being scheduled  
* percentage utilization- amount of time when actual work was being done by system  
* total number of tasks
* number of tasks that missed deadline
* number of tasks that failed due to FP (i.e. miscellaneous reasons as described in report)  
* net cost of execution  

This is followed by final ST (shared trust) and DT (direct trust) values of all nodes and users.  