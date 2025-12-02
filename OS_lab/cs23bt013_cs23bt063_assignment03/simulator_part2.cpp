#include <bits/stdc++.h>
#include <chrono>
using namespace std;

struct Process {
    int id;
    int time_of_arrival;
    vector<int> bursts;
    int current_burst_index = 0;
    int time_of_completion = 0;
};
bool compareByArrival(const Process &a, const Process &b) {
    return a.time_of_arrival < b.time_of_arrival;
}
struct CPU {
    Process* running=nullptr;
    int remaining = 0;      
    int start_time = 0;     
};

int main(int argc, char* argv[]) {
    if (argc != 3) {
        cerr << "Usage: " << argv[0] << " <algorithm> <workload-file>\n";
        return 1;
    }

    string algo = argv[1];
    string filename = argv[2];
    if (algo != "FIFO" && algo != "SJF" && algo != "PSJF" && algo != "RR") {
        cerr << "Supported algorithms: FIFO, SJF, PSJF, RR\n";
        return 1;
    }

    ifstream fin(filename);
    if (!fin) {
        cerr << "Error: Cannot open file " << filename << "\n";
        return 1;
    }

    vector<Process> processes;
    string line;
    int pid = 1;
    while (getline(fin, line)) {
        if (line.empty()) continue;
        stringstream ss(line);
        Process p;
        p.id = pid++;
        if (!(ss >> p.time_of_arrival)) continue;
        int burst;
        while (ss >> burst && burst != -1) p.bursts.push_back(burst);
        if (!p.bursts.empty()) processes.push_back(p);
    }

     sort(processes.begin(),processes.end(),compareByArrival);
    CPU cpus[2];
    deque<Process*>ready_queue;
    using IOEvent = pair<int, Process*>;
    priority_queue<IOEvent, vector<IOEvent>, greater<IOEvent>> io_waiting;
    int slice;
    int process_index = 0, current_time = 0;
    vector<int> turnaround_times;
    if (algo=="RR") {
        cout <<"Enter the time slice value: "; cin>>slice;
    }
    cout << "CPU0 and CPU1 simulation\n";
    auto simulator_startingtime = chrono::high_resolution_clock::now();

    while (process_index<processes.size() || !ready_queue.empty() || !io_waiting.empty() ||
           cpus[0].running || cpus[1].running) {

        while (process_index<processes.size() && processes[process_index].time_of_arrival<=current_time) {
            ready_queue.push_back(&processes[process_index]);
            process_index++;
        }

        while (!io_waiting.empty() && io_waiting.top().first<=current_time) {
            ready_queue.push_back(io_waiting.top().second);
            io_waiting.pop();
        }

        if (algo == "PSJF") {
            for (int i=0;i<2;i++) {
                if (cpus[i].running && !ready_queue.empty()) {
                    auto it = min_element(ready_queue.begin(), ready_queue.end(),
                        [](Process* a, Process* b){
                            return a->bursts[a->current_burst_index] < b->bursts[b->current_burst_index];
                        });
                    if ((*it)->bursts[(*it)->current_burst_index] < cpus[i].remaining) {
                        ready_queue.push_back(cpus[i].running);
                        cpus[i].running = nullptr;
                        cpus[i].remaining = 0;
                    }
                }
            }
        }
        for (int i = 0;i<2;i++) {
            if (!cpus[i].running && !ready_queue.empty()) {
                Process* chosen = nullptr;
                if (algo == "FIFO" || algo == "RR") {
                    chosen = ready_queue.front(); ready_queue.pop_front();
                } else if (algo == "SJF" || algo == "PSJF") {
                    auto it = min_element(ready_queue.begin(), ready_queue.end(),
                        [](Process* a,Process* b){
                            return a->bursts[a->current_burst_index] < b->bursts[b->current_burst_index];
                        });
                    chosen = *it;
                    ready_queue.erase(it);
                }
                if (chosen && chosen->current_burst_index < chosen->bursts.size()) {
                    int exec_time = chosen->bursts[chosen->current_burst_index];
                    if (algo=="RR") exec_time = min(exec_time, slice);
                    cpus[i].running = chosen;
                    cpus[i].remaining = exec_time;
                    cpus[i].start_time = current_time;
                }
            }
        }

        for (int c=0;c<2;c++) {
            if (cpus[c].running) {
                Process* p = cpus[c].running;
                cpus[c].remaining--;
                if (cpus[c].remaining == 0) {
                    int burst_num = (p->current_burst_index / 2) + 1;
                    cout << "CPU" << c << ": P" << p->id << "," << burst_num
                         << "\t" << cpus[c].start_time << "\t" << current_time << endl;

                    if (algo == "RR") {
                        int used = min(p->bursts[p->current_burst_index], slice);
                        p->bursts[p->current_burst_index] -= used;

                        if (p->bursts[p->current_burst_index] == 0) {
                            p->current_burst_index++;
                            if (p->current_burst_index < p->bursts.size()) {
                                int io_burst = p->bursts[p->current_burst_index];
                                io_waiting.push({current_time + 1 + io_burst, p});
                                p->current_burst_index++;
                            } else {
                                p->time_of_completion = current_time + 1;
                                turnaround_times.push_back(p->time_of_completion - p->time_of_arrival);
                            }
                        } else{
                            ready_queue.push_back(p);
                        }
                    } else{ 
                        p->current_burst_index++;
                        if (p->current_burst_index<p->bursts.size()) {
                            int io_burst=p->bursts[p->current_burst_index];
                            io_waiting.push({current_time+1+io_burst,p});
                            p->current_burst_index++;
                        } else {
                            p->time_of_completion = current_time + 1;
                            turnaround_times.push_back(p->time_of_completion - p->time_of_arrival);
                        }
                    }
                    cpus[c].running = nullptr;
                    cpus[c].remaining = 0;
                }
            }
        }

        current_time++;
    }

    auto simulator_endingtime = chrono::high_resolution_clock::now();
    chrono::duration<double> elapsed = simulator_endingtime - simulator_startingtime;
     if (!turnaround_times.empty()) {
        double average_turnaround_time =accumulate(turnaround_times.begin(),turnaround_times.end(), 0.0)/turnaround_times.size();
        int maximum_turnaround_time=*max_element(turnaround_times.begin(),turnaround_times.end());
        cout<<"\nAverage Turnaround Time = "<<average_turnaround_time<<"\n";
        cout<<"Maximum Turnaround Time = " <<maximum_turnaround_time<<"\n";
    }
    cout << "run time of the simulator is : " << elapsed.count() << " seconds\n";
    return 0;
}