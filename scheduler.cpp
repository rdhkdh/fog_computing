#include <bits/stdc++.h>
#include <unistd.h>
using namespace std;

const int K = 5;            // relative deadline
const int U = 5;            // no of users
const int M = 5;            // no of FOG nodes
const double BR = 10;       // brand constant, decides the cost
double failure_rate = 0.2;  // determines FP of a task
const int TOTAL_TIME = 200; // total duration of execution
int GLOBAL_CLOCK = 0; 
//attributes to output:
int idle_time = 0, late_tasks = 0, total_no_of_tasks = 0, failed_due_to_FP = 0;
double total_cost = 0, executiontime_mean = 2.0;

vector<vector<double>> DT(U, vector<double>(M, 0.5)); // direct trust
vector<double> ST(M, 0.5);                            // shared trust

struct User
{
    int id;
};

struct Task
{
    int arrival;
    int execution_time;
    int deadline;
    double failure_probability;
    User *owner;
};

struct Node
{
    int id;
    bool busy;
    int finish_time;
    Task *assigned_task;
};

std::queue<Task> Central_queue; // central queue for task requests

void update_ST(int Machine_index)
{
    double sum = 0;
    for (int i = 0; i < U; i++)
    {
        sum += DT[i][Machine_index];
    }
    ST[Machine_index] = sum / double(U);
}

// lower the ratio, more optimum is the node
double calculate_cost_ratio(int m)
{
    double Base = 0.3 * BR;
    double C = Base + (BR * ST[m] * ST[m]); // cost of execution
    double cost_ratio = C / ST[m];
    return cost_ratio;
}

// sorts Tasks in decreasing order of failure probability
bool compareByFailureProbability(const Task &a, const Task &b)
{
    return a.failure_probability > b.failure_probability;
}

int getRandomInt(int min, int max)
{
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_int_distribution<int> distribution(min, max);
    return distribution(gen);
}

// Function to generate 0 with probability f and 1 with probability 1-f
int generateWithProbability(double f)
{
    // Initialize random number generator
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<double> dis(0.0, 1.0);

    // Generate a random number between 0 and 1
    double randomValue = dis(gen);

    // Compare with probability f
    if (randomValue < f)
    {
        return 0; // Return 0 with probability f
    }
    else
    {
        return 1; // Return 1 with probability 1-f
    }
}

void generate_random_tasks(vector<User> &users)
{
    default_random_engine generator(time(nullptr));
    poisson_distribution<int> arrival_distribution(3.0);
    poisson_distribution<int> execution_distribution(executiontime_mean);

    int current_time = 0;

    while (current_time < TOTAL_TIME)
    {
        int freq = getRandomInt(1, U); // no of tasks having same arrival time
        int arrival_time = current_time + arrival_distribution(generator);

        // Generate 'freq' no of new tasks
        for (int i = 1; i <= freq; i++)
        {
            Task new_task;

            int execution_time = execution_distribution(generator);
            new_task.arrival = arrival_time;
            new_task.execution_time = execution_time;
            new_task.deadline = arrival_time + K;
            new_task.failure_probability = 0.1 * exp(-failure_rate * double(new_task.execution_time)); // exp(-f*t), where f is the failure rate

            // Assign the task to a random user
            int index = rand() % users.size();
            User &u = users[index];
            new_task.owner = &u;

            // push the created task into central queue
            Central_queue.push(new_task);
        }

        // Update current time
        current_time = arrival_time;
    }
}

void assign_nodes(vector<Node> &nodes, const vector<User> &users)
{
    Task curr;
    int a = 0; // arrival time of head of queue
    vector<Task> curr_tasks, pending_tasks, new_tasks;
    curr_tasks.clear();
    pending_tasks.clear();

    while (GLOBAL_CLOCK < TOTAL_TIME + (5 * executiontime_mean))
    {
        if (!Central_queue.empty())
        {
            // Update free/busy status of all nodes
            for (int i = 0; i < M; i++)
            {
                if (GLOBAL_CLOCK >= nodes[i].finish_time)
                {
                    nodes[i].busy = false;
                }
            }

            // Put all pending tasks at front
            curr_tasks = pending_tasks;
            pending_tasks.clear();

            // Pop from queue, all tasks of a particular arrival time (if reached)
            curr = Central_queue.front();
            a = curr.arrival;
            if (a <= GLOBAL_CLOCK) // if task has actually arrived
            {
                while (curr.arrival == a)
                {
                    Central_queue.pop();       // pop out current task
                    new_tasks.push_back(curr); // store in vector of new tasks
                    if (Central_queue.empty())
                        break;                    // Check if queue is empty to avoid invalid access
                    curr = Central_queue.front(); // next task in queue
                }
            }
            else
            {
                idle_time++;
                GLOBAL_CLOCK++;
                continue;
            }

            // Order the tasks by shortest processing time, i.e., highest FP
            sort(new_tasks.begin(), new_tasks.end(), compareByFailureProbability);

            // curr_tasks = pending_tasks + new_tasks
            for (auto u : new_tasks)
            {
                curr_tasks.push_back(u);
            }
            new_tasks.clear();

            // Iterate over available nodes to calculate cost ratio
            vector<pair<double, int>> cost_ratios; // cost_ratio, node index
            for (int i = 0; i < min(M, U); i++)    // Only consider first U nodes
            {
                if (!nodes[i].busy) // if not busy, node can be allocated
                {
                    double cr = calculate_cost_ratio(i);
                    cost_ratios.push_back(make_pair(cr, i));
                }
            }
            sort(cost_ratios.begin(), cost_ratios.end()); // sort in increasing order of cost_ratio

            //------------------------Assign tasks to nodes---------------------------
            int tasks_assigned = 0;                                              // Track how many tasks have been assigned
            for (int j = 0; j < min(curr_tasks.size(), cost_ratios.size()); j++) // Ensure we don't access invalid indices
            {
                int node_id = cost_ratios[j].second;
                int user_id = curr_tasks[j].owner->id;
                nodes[node_id].assigned_task = &curr_tasks[j];                            // match task to node
                nodes[node_id].finish_time = GLOBAL_CLOCK + curr_tasks[j].execution_time; // set finish time of node
                nodes[node_id].busy = true;                                               // mark node as busy

                // Output assignment information
                cout << "Task: ";
                cout << "Arrival=" << curr_tasks[j].arrival << " ";
                cout << "Execution=" << curr_tasks[j].execution_time << " ";
                cout << "Deadline=" << curr_tasks[j].deadline << " ";
                cout << "FP=" << fixed << setprecision(2) << curr_tasks[j].failure_probability << " "; // round to 2 decimal places
                cout << "User=" << user_id << "\n";

                cout << "Assigned to: ";
                cout << "Node=" << node_id << " ";
                cout << "Shared Trust=" << ST[node_id] << " ";
                cout << "Finish Time=" << nodes[node_id].finish_time << "\n";

                // calculate cost of assignment and add to net cost
                double C = (0.3 * BR) + (BR * ST[node_id] * ST[node_id]); // cost of execution
                total_cost += (C * curr_tasks[j].execution_time);

                cout << "Old DT= " << DT[user_id][node_id] << " Old ST= " << ST[node_id] << endl;
                // Check if the task finishes before its deadline
                if (nodes[node_id].finish_time <= curr_tasks[j].deadline)
                {
                    // Generate a random number to simulate success or failure
                    int flag = generateWithProbability(curr_tasks[j].failure_probability);

                    if (flag == 1)
                    {
                        // Task successful execution
                        cout << "Task successfully executed.\n";
                        // Update trust (increase) as the task executed successfully
                        DT[user_id][node_id] += (1 - DT[user_id][node_id]) * 0.1; // Increment based on current value
                        // Ensure DT doesn't exceed 1
                        DT[user_id][node_id] = min(DT[user_id][node_id], 1.0);
                    }
                    else
                    {
                        // Task failed due to miscellaneous reasons
                        cout << "Task failed due to miscellaneous reasons.\n";
                        // Update trust (decrease) as the task failed
                        DT[user_id][node_id] -= DT[user_id][node_id] * 0.1; // Decrement based on current value
                        // Ensure DT doesn't go below 0
                        DT[user_id][node_id] = max(DT[user_id][node_id], 0.0);
                        failed_due_to_FP++;
                    }
                }
                else
                {
                    // Task missed its deadline
                    cout << "Task missed its deadline.\n";
                    // Update trust (decrease) as the task missed its deadline
                    DT[user_id][node_id] -= DT[user_id][node_id] * 0.2; // Decrement based on current value
                    // Ensure DT doesn't go below 0
                    DT[user_id][node_id] = max(DT[user_id][node_id], 0.0);
                    late_tasks++;
                }
                update_ST(node_id);
                cout << "New DT= " << DT[user_id][node_id] << " New ST= " << ST[node_id] << endl;
                cout << endl;

                tasks_assigned++; // Increment the count of tasks assigned
            }

            // If there are tasks remaining after all nodes are assigned, push them back to pending_tasks
            if (tasks_assigned < curr_tasks.size())
            {
                for (int k = tasks_assigned; k < curr_tasks.size(); k++)
                {
                    pending_tasks.push_back(curr_tasks[k]);
                }
            }

            curr_tasks.clear();
        }

        GLOBAL_CLOCK++;
    }
}

int main()
{
    srand(time(nullptr)); // Seed the random number generator

    // create and intialize users vector
    vector<User> users(U);
    for (int i = 0; i < U; ++i)
    {
        users[i].id = i;
    }

    // create and intialize nodes vector
    vector<Node> nodes(M);
    for (int i = 0; i < M; i++)
    {
        nodes[i].id = i;
        nodes[i].busy = false;
        nodes[i].finish_time = 0;
    }

    generate_random_tasks(users);
    total_no_of_tasks = Central_queue.size();

    assign_nodes(nodes, users);

    cout << "------------------FoG System Report--------------------\n";
    cout << "Total uptime= " << TOTAL_TIME + (5 * executiontime_mean) << "s\n";
    cout << "Idle time= " << idle_time << "s" << endl;
    cout << "System utilization= " << double((2 * TOTAL_TIME - idle_time) * 100) / double(2 * TOTAL_TIME) << "%" << endl;
    cout << "Total no. of tasks= " << total_no_of_tasks << endl;
    cout << "No. of tasks that missed deadline= " << late_tasks << endl;
    cout << "No. of tasks that failed due to FP= " << failed_due_to_FP << endl;
    cout << "Net cost of executing all tasks= " << total_cost << endl;

    cout << "\n---------------------Final DT and ST values-------------------------------\n";
    cout << "ST: ";
    for (auto u : ST)
    {
        cout << u << " ";
    }
    cout << "\n\n";

    cout << "DT:\n";
    for (int i1 = 0; i1 < U; i1++)
    {
        for (int j1 = 0; j1 < M; j1++)
        {
            cout << DT[i1][j1] << " ";
        }
        cout << "\n";
    }
    cout << "\n";

    return 0;
}
