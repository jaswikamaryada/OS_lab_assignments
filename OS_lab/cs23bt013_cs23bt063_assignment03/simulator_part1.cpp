#include <bits/stdc++.h>
#include <chrono>
using namespace std;

struct Process {
    int id;
    int time_of_arrival;
    vector<int> bursts;            
    int current_burst_index = 0;  
    int completion_time = 0;
};
bool compareByArrival(const Process &a, const Process &b) {
    return a.time_of_arrival < b.time_of_arrival;
}
int main(int argc, char* argv[]) {
    if (argc!= 3) {
        cerr<< "Usage:" << argv[0] << "<algorithm> <workload-file>\n";
        return 1;
    }
    string algo = argv[1];
    string filename = argv[2];
    if (algo!="FIFO" && algo!="SJF" && algo != "PSJF" && algo!="RR") {
        cerr << "The entered Algorithm Not found";
        return 1;
    }
    ifstream fin(filename);
     if (!fin) {
        cerr<<"Enter Valid filename.Cananot open file"<<filename<<"\n";
        return 1;
    }
    vector<Process> processes;
    string entry;
    int pid=1;
    while (getline(fin, entry)) {
        if (entry.empty()) continue;
        stringstream ss(entry);
        Process p;
        p.id = pid++;
        if (!(ss>>p.time_of_arrival)) continue;
        int burst;
        while (ss>>burst && burst != -1) {
            p.bursts.push_back(burst);
        }
        if (!p.bursts.empty()) processes.push_back(p);
    }
    sort(processes.begin(), processes.end(),compareByArrival);
    cout<<"CPU0\n";
    int current_time=0;
    int process_index=0;
    vector<int> turnaround_times;
    if (algo=="FIFO" || algo=="SJF") {
        auto simulator_startingtime = chrono::high_resolution_clock::now();
        vector<Process*>ready_queue;
        using IOEvent = pair<int, Process*>;
        priority_queue<IOEvent,vector<IOEvent>,greater<IOEvent>>io_waiting;
        while (process_index < processes.size() || !ready_queue.empty() || !io_waiting.empty()) {
            while (process_index < processes.size() &&
                   processes[process_index].time_of_arrival <= current_time) {
                ready_queue.push_back(&processes[process_index]);
                process_index++;
            }
            while (!io_waiting.empty() && io_waiting.top().first <= current_time) {
                ready_queue.push_back(io_waiting.top().second);
                io_waiting.pop();
            }
            if (!ready_queue.empty()) {
                Process* present_process = nullptr;
                if (algo=="FIFO") {
                    present_process = ready_queue.front();
                    ready_queue.erase(ready_queue.begin());
                } else { 
                    auto it=min_element(ready_queue.begin(), ready_queue.end(),
                        [](Process* a, Process* b) {
                            int bursttimeA=(a->current_burst_index < a->bursts.size())
                                             ? a->bursts[a->current_burst_index]
                                             : INT_MAX;
                            int bursttimeB=(b->current_burst_index < b->bursts.size())
                                             ? b->bursts[b->current_burst_index]
                                             : INT_MAX;
                            return bursttimeA<bursttimeB;
                        });
                    present_process=*it;
                    ready_queue.erase(it);
                }
                if (present_process->current_burst_index < present_process->bursts.size()) {
                    int cpu_burst = present_process->bursts[present_process->current_burst_index];
                    int start_time=current_time;
                    current_time += cpu_burst;
                    cout << "P"<< present_process->id << ","
                         << (present_process->current_burst_index / 2 + 1)
                         << "\t" << start_time << "\t" << current_time - 1 << endl;

                    present_process->current_burst_index++;
                    if (present_process->current_burst_index < present_process->bursts.size()) {
                        int io_burst = present_process->bursts[present_process->current_burst_index];
                        int io_completion = current_time + io_burst;
                        present_process->current_burst_index++;
                        io_waiting.push({io_completion, present_process});
                    } else {
                        present_process->completion_time = current_time;
                        turnaround_times.push_back(
                            present_process->completion_time - present_process->time_of_arrival);
                    }
                }
            } else {
                int next_event_time = INT_MAX;
                if (process_index < processes.size())
                    next_event_time = min(next_event_time, processes[process_index].time_of_arrival);
                if (!io_waiting.empty())
                    next_event_time = min(next_event_time, io_waiting.top().first);
                if (next_event_time != INT_MAX) current_time = next_event_time;
            }
        }

        auto simulator_endingtime = chrono::high_resolution_clock::now();
        chrono::duration<double> elapsed = simulator_endingtime - simulator_startingtime;
        cout << "\n RunTime of Simulator is " << elapsed.count() << " seconds\n";
    }

    else if (algo == "PSJF") {
        auto simulator_startingtime = chrono::high_resolution_clock::now();
        struct Job {
            Process* proc;
            int remaining;
        };
        vector<Job> ready;
        using IOEvent = pair<int, Process*>;
        priority_queue<IOEvent, vector<IOEvent>, greater<IOEvent>> io_waiting;

        int n=processes.size();
        int completed=0;
        int prev_pid=-1,prev_burst_num =-1, interval_start = -1;

        while (completed<n) {
            while (process_index < n && processes[process_index].time_of_arrival <= current_time) {
                ready.push_back({&processes[process_index],
                                 processes[processes[process_index].id - 1].bursts[0]});
                process_index++;
            }
            while (!io_waiting.empty() && io_waiting.top().first <= current_time) {
                Process* p = io_waiting.top().second;
                io_waiting.pop();
                if (p->current_burst_index < p->bursts.size()) {
                    ready.push_back({p, p->bursts[p->current_burst_index]});
                }
            }

            if (!ready.empty()) {
                auto it = min_element(ready.begin(), ready.end(),
                    [](const Job& a, const Job& b) { return a.remaining < b.remaining; });
                Job present = *it;
                ready.erase(it);

                if (prev_pid != present.proc->id || prev_burst_num != present.proc->current_burst_index/2+1) {
                    if (prev_pid != -1) {
                        cout << "P" << prev_pid << "," << prev_burst_num
                             << "\t" << interval_start << "\t" << current_time - 1 << endl;
                    }
                    interval_start = current_time;
                    prev_pid = present.proc->id;
                    prev_burst_num = present.proc->current_burst_index/2+1;
                }

                present.remaining--;
                present.proc->bursts[present.proc->current_burst_index]--;
                current_time++;

                if (present.remaining > 0) {
                    ready.push_back(present);
                } else {
                    present.proc->current_burst_index++;
                    if (present.proc->current_burst_index < present.proc->bursts.size()) {
                        int io_burst = present.proc->bursts[present.proc->current_burst_index];
                        int io_completion = current_time + io_burst;
                        present.proc->current_burst_index++;
                        io_waiting.push({io_completion, present.proc});
                    } else {
                        present.proc->completion_time = current_time;
                        turnaround_times.push_back(
                            present.proc->completion_time - present.proc->time_of_arrival);
                        completed++;
                    }
                }
            } else {
                if (prev_pid != -1) {
                    cout << "P" << prev_pid << "," << prev_burst_num
                         << "\t" << interval_start << "\t" << current_time - 1 << endl;
                    prev_pid = -1;
                }
                current_time++;
            }
        }
        if (prev_pid!=-1) {
            cout << "P" << prev_pid << "," << prev_burst_num
                 << "\t" << interval_start << "\t" << current_time - 1 << endl;
        }

        auto simulator_endingtime = chrono::high_resolution_clock::now();
        chrono::duration<double> elapsed = simulator_endingtime - simulator_startingtime;
        cout << "\nRunTime of Simulator for PSJF is " << elapsed.count() << " seconds\n";
    }

    else if (algo == "RR") {
        auto simulator_startingtime=chrono::high_resolution_clock::now();
        int slice;
        cout<<"Enter The time sLice value:";
        cin>>slice;
        queue<Process*> ready_queue;
        int n=processes.size();
        while (process_index < n || !ready_queue.empty()) {
            while (process_index < n && processes[process_index].time_of_arrival <= current_time) {
                ready_queue.push(&processes[process_index]);
                process_index++;
            }

            if (!ready_queue.empty()) {
                Process* present = ready_queue.front();
                ready_queue.pop();

                if (present->current_burst_index < present->bursts.size()) {
                    int cpu_burst = present->bursts[present->current_burst_index];
                    int executiontime = min(cpu_burst, slice);
                    int start_time = current_time;
                    current_time += executiontime;
                    cout << "P" << present->id << ","
                         << (present->current_burst_index/2 + 1)
                         << "\t" << start_time << "\t" << current_time - 1 << endl;
                    present->bursts[present->current_burst_index] -= executiontime;
                    if (present->bursts[present->current_burst_index] == 0) {
                        present->current_burst_index++;
                        if (present->current_burst_index < present->bursts.size()) {
                            int io_burst = present->bursts[present->current_burst_index];
                            current_time += io_burst;
                            present->current_burst_index++;
                            if (present->current_burst_index < present->bursts.size()) {
                                ready_queue.push(present);
                            } else {
                                present->completion_time = current_time;
                                turnaround_times.push_back(
                                    present->completion_time - present->time_of_arrival);
                            }
                        } else {
                            present->completion_time = current_time;
                            turnaround_times.push_back(
                                present->completion_time - present->time_of_arrival);
                        }
                    } else {
                        ready_queue.push(present);
                    }
                }
            } else {
                if (process_index<n) current_time=processes[process_index].time_of_arrival;
            }
        }

        auto simulator_endingtime=chrono::high_resolution_clock::now();   
        chrono::duration<double> elapsed=simulator_endingtime-simulator_startingtime;
        cout<<"\nrunTime of simulator is " <<elapsed.count()<<" seconds\n";
    }

    if (!turnaround_times.empty()) {
         double average_turnaround_time =accumulate(turnaround_times.begin(),turnaround_times.end(), 0.0)/turnaround_times.size();
        int maximum_turnaround_time=*max_element(turnaround_times.begin(),turnaround_times.end());
        cout<<"\naverage Turnaround Time = "<<average_turnaround_time<<"\n";
        cout<<"maximum Turnaround Time = " <<maximum_turnaround_time<<"\n";
    }

    return 0;
}