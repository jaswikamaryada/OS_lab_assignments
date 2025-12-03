#include <iostream>
#include <fstream>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <cstdlib>

using namespace std;

int main(int argc, char **argv)
{
    if(argc != 6)
    {
        cout <<"usage: ./part2_partitioner.out <path-to-file> <pattern> <search-start-position> <search-end-position> <max-chunk-size>\nprovided arguments:\n";
        for(int i = 0; i < argc; i++)
            cout << argv[i] << "\n";
        return -1;
    }
    
    char *file_to_search_in = argv[1];
    char *pattern_to_search_for = argv[2];
    int search_start_position = atoi(argv[3]);
    int search_end_position = atoi(argv[4]);
    int max_chunk_size = atoi(argv[5]);

    pid_t my_pid = getpid();

    cout << "[" << my_pid << "] start position = " << search_start_position 
         << " ; end position = " << search_end_position << "\n";

    int range = search_end_position - search_start_position + 1;

    if(range > max_chunk_size)
    {
        int mid = (search_start_position + search_end_position) / 2;

        //Left Child
        pid_t left_pid = fork();
        if(left_pid == 0)
        {
            execl("./part2_partitioner.out", "part2_partitioner.out",
                  file_to_search_in, pattern_to_search_for,
                  to_string(search_start_position).c_str(),
                  to_string(mid).c_str(),
                  to_string(max_chunk_size).c_str(),
                  (char *)NULL);
            perror("execl failed");
            exit(1);
        }
        else
        {
            cout << "[" << my_pid << "] forked left child " << left_pid << "\n";
        }

        //Right Child
        pid_t right_pid = fork();
        if(right_pid == 0)
        {
            execl("./part2_partitioner.out", "part2_partitioner.out",
                  file_to_search_in, pattern_to_search_for,
                  to_string(mid+1).c_str(),
                  to_string(search_end_position).c_str(),
                  to_string(max_chunk_size).c_str(),
                  (char *)NULL);
            perror("execl failed");
            exit(1);
        }
        else
        {
            cout << "[" << my_pid << "] forked right child " << right_pid << "\n";
        }

        //Wait for Children
        int status;
        pid_t wpid;

        wpid = waitpid(left_pid, &status, 0);
        if(wpid > 0)
            cout << "[" << my_pid << "] left child " << wpid << " returned\n";

        wpid = waitpid(right_pid, &status, 0);
        if(wpid > 0)
            cout << "[" << my_pid << "] right child " << wpid << " returned\n";
    }
    else
    {
        // Range small enough → spawn searcher
        pid_t searcher_pid = fork();
        if(searcher_pid == 0)
        {
            execl("./part2_searcher.out", "part2_searcher.out",
                  file_to_search_in, pattern_to_search_for,
                  to_string(search_start_position).c_str(),
                  to_string(search_end_position).c_str(),
                  (char *)NULL);
            perror("execl failed");
            exit(1);
        }
        else
        {
            cout << "[" << my_pid << "] forked searcher child " << searcher_pid << "\n";
        }

        int status;
        waitpid(searcher_pid, &status, 0);
        cout << "[" << my_pid << "] searcher child " << searcher_pid << " returned\n";
    }

    return 0;
}
