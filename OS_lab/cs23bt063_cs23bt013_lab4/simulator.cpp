#include <bits/stdc++.h>
using namespace std;

using AddrType=uint64_t;
using vpntype=uint64_t;
class PageFrame{
public:
    int pid;
    vpntype vpn; 
    bool occupied;
    PageFrame() {
                pid=-1; 
                 vpn=0; 
                 occupied=false; 
                }
    void assignvalues(int p,vpntype v){ 
                pid=p; 
                vpn=v; 
                occupied=true; 
               }
    void freevalues() { 
                   pid=-1; 
                   vpn=0; 
                   occupied=false; 
                }
};
class PageTable{
    unordered_map<vpntype,int> table;
public:
    bool checkcontains(vpntype vpn){ 
           return table.find(vpn)!=table.end(); 
        }
    int getFrameNumber(vpntype vpn){ 
        return table.at(vpn); 
        }
    void mapPageToFrame(vpntype vpn,int frame) { 
                   table[vpn] = frame; 
        }
    void evictPage(vpntype vpn){ 
        table.erase(vpn); 
    }
};

int getRandom(int n){ 
        return rand() % n; 
}

pair<int,vector<int>> simulate(int size_of_page,int num_frames,string replacementpolicy, string allocationpolicy,
                                const vector<pair<int,AddrType>>&trace, const set<int>&runningprocesses)
{
    int number_of_processes=runningprocesses.size();
    vector<int>proc_ids(runningprocesses.begin(),runningprocesses.end());

    vector<PageFrame>frame_tables_global;
    unordered_map<int,vector<PageFrame>>frame_tables_local;
    queue<int>fifo_queue_global; list<int>lru_list_global;
    unordered_map<int,queue<int>>localfifo_queues;
    unordered_map<int,list<int>>locallru_lists;

   
    unordered_map<int,int>frames_per_process;
    if(allocationpolicy=="Local"){
        int base= num_frames/number_of_processes;
        int rem= num_frames%number_of_processes;
        for(int i=0;i<number_of_processes;i++){
            int extra=(i<rem ? 1 : 0);
            frames_per_process[proc_ids[i]]=base+extra;
            frame_tables_local[proc_ids[i]].resize(base+extra);
        }
    }
    else frame_tables_global.resize(num_frames); 

    unordered_map<int,PageTable>page_tables;
    int all_page_faults=0;
    unordered_map<int,int>per_process_faults;
    for(auto p:runningprocesses) per_process_faults[p] = 0;

    unordered_map<int,unordered_map<vpntype,queue<size_t>>>nextaccesses;
    if(replacementpolicy=="Optimal"){
        for(size_t i=0;i<trace.size();i++){
            int pid=trace[i].first;
            vpntype vpn=trace[i].second/ size_of_page;
            nextaccesses[pid][vpn].push(i);
        }
    }

    for(size_t t=0;t<trace.size();t++){
        int pid=trace[t].first;
        vpntype vpn = trace[t].second / size_of_page;

        if(page_tables[pid].checkcontains(vpn)){
            if(replacementpolicy=="LRU"){
                int f = page_tables[pid].getFrameNumber(vpn);
                if(allocationpolicy=="Global"){
                    lru_list_global.remove(f); lru_list_global.push_back(f);
                } else {
                    locallru_lists[pid].remove(f);
                    locallru_lists[pid].push_back(f);
                }
            }
            if(replacementpolicy=="Optimal" && !nextaccesses[pid][vpn].empty())
                nextaccesses[pid][vpn].pop();
            continue;
        }

        // PAGE FAULT
        all_page_faults++;
        per_process_faults[pid]++;

        vector<PageFrame>* frames;
        queue<int>* fifo_queue=nullptr;
        list<int>* lru_list=nullptr;
        int frame_count;

        if(allocationpolicy=="Global"){
            frames = &frame_tables_global;
            fifo_queue = &fifo_queue_global;
            lru_list = &lru_list_global;
            frame_count = num_frames;
        } else {
            frames = &frame_tables_local[pid];
            fifo_queue = &localfifo_queues[pid];
            lru_list = &locallru_lists[pid];
            frame_count = frames_per_process[pid];
        }

        int frame_to_use=-1;
        for(int i=0;i<frame_count;i++) if(!(*frames)[i].occupied){ frame_to_use=i; break; }

        if(frame_to_use==-1){
            if(replacementpolicy=="FIFO"){
                frame_to_use = fifo_queue->front(); fifo_queue->pop();
            } else if(replacementpolicy=="LRU"){
                frame_to_use = lru_list->front(); lru_list->pop_front();
            } else if(replacementpolicy=="Random"){
                frame_to_use = getRandom(frame_count);
            } else if(replacementpolicy=="Optimal"){
                size_t farthest=0, victim=0; bool first=true;
                for(int i=0;i<frame_count;i++){
                    int f_pid = (*frames)[i].pid;
                    vpntype f_vpn = (*frames)[i].vpn;
                    size_t next_use = trace.size();
                    if(!nextaccesses[f_pid][f_vpn].empty()) next_use = nextaccesses[f_pid][f_vpn].front();
                    if(first || next_use>farthest){ farthest=next_use; victim=i; first=false; }
                }
                frame_to_use = victim;
            }
            int old_pid = (*frames)[frame_to_use].pid;
            vpntype old_vpn = (*frames)[frame_to_use].vpn;
            page_tables[old_pid].evictPage(old_vpn);
            (*frames)[frame_to_use].freevalues();
        }

        (*frames)[frame_to_use].assignvalues(pid, vpn);
        page_tables[pid].mapPageToFrame(vpn, frame_to_use);

        if(replacementpolicy=="FIFO") fifo_queue->push(frame_to_use);
        if(replacementpolicy=="LRU") lru_list->push_back(frame_to_use);
        if(replacementpolicy=="Optimal" && !nextaccesses[pid][vpn].empty())
            nextaccesses[pid][vpn].pop();
    }

    vector<int> result_vector;
    for(auto p: runningprocesses) result_vector.push_back(per_process_faults[p]);
    return {all_page_faults, result_vector};
}

int main(int argc,char* argv[]){
    if(argc<6){
        cout<<"Usage: ./simulator <page_size> <num_frames> <replacementpolicy> <allocationpolicy> <trace_file>\n";
        return 1;
    }

    int size_of_page=stoi(argv[1]);
    int num_frames=stoi(argv[2]);
    string replacementpolicy=argv[3];
    string allocationpolicy=argv[4];
    string trace_file=argv[5];

    ifstream fin(trace_file);
    if(!fin){ 
        cout<<"Cannot open trace file\n";
         return 1; 
    }

    vector<pair<int,AddrType>>trace;
    set<int>runningprocesses;
    string line;
    while(getline(fin,line)){
        if(line.empty()) continue;
        stringstream ss(line);
        string pid_s,addr_s;
        if(!getline(ss,pid_s,',')) continue;
        if(!getline(ss,addr_s)) continue;
        try{
            int pid=stoi(pid_s);
            AddrType addr=(AddrType)stoull(addr_s);
            trace.push_back({pid, addr});
            runningprocesses.insert(pid);
        } catch(...){
             continue; 
            }
    }

    auto result = simulate(size_of_page, num_frames, replacementpolicy, allocationpolicy, trace, runningprocesses);

    cout<<"Allocation: "<<allocationpolicy<<"\n";
    cout<<"Replacement: "<<replacementpolicy<<"\n";
    cout<<"Total Page Faults:"<<result.first<<"\n";
    int idx=0;
    for(auto p:runningprocesses){
        cout << "Process "<< p << " Number of PageFaults: " <<result.second[idx++]<< "\n";
    }

    return 0;
}