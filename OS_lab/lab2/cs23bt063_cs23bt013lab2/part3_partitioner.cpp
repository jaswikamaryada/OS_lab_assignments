#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
using namespace std;

void handle_sigterm(int signum) {
    cout << "[" << getpid() << "] received SIGTERM\n";
    exit(0);
}

int main(int argc, char **argv) {
    if (argc != 6) {
        cerr << "usage: ./part3_partitioner.out <file> <pattern> <start> <end> <max-chunk-size>\n";
        return -1;
    }

    signal(SIGTERM, handle_sigterm);

    char *file = argv[1];
    char *pattern = argv[2];
    int start = atoi(argv[3]);
    int end = atoi(argv[4]);
    int max_chunk_size = atoi(argv[5]);

    pid_t my_pid = getpid();
    int len = end - start + 1;

    cout << "[" << my_pid << "] start position = " << start
         << " ; end position = " << end << "\n";

    if (len <= max_chunk_size) {
        // create a searcher
        pid_t searcher_pid = fork();
        if (searcher_pid == 0) {
            execl("./part3_searcher.out", "./part3_searcher.out",
                  file, pattern,
                  to_string(start).c_str(),
                  to_string(end).c_str(),
                  (char *)NULL);
            perror("execl failed");
            exit(1);
        } else {
            cout << "[" << my_pid << "] forked searcher child " << searcher_pid << "\n";
            waitpid(searcher_pid, NULL, 0);
            cout << "[" << my_pid << "] searcher child returned\n";
        }
    } else {
        // split into left and right partitions
        int mid = (start + end) / 2;

        pid_t left = fork();
        if (left == 0) {
            execl("./part3_partitioner.out", "./part3_partitioner.out",
                  file, pattern,
                  to_string(start).c_str(),
                  to_string(mid).c_str(),
                  to_string(max_chunk_size).c_str(),
                  (char *)NULL);
            perror("execl failed");
            exit(1);
        }
        cout << "[" << my_pid << "] forked left child " << left << "\n";

        pid_t right = fork();
        if (right == 0) {
            execl("./part3_partitioner.out", "./part3_partitioner.out",
                  file, pattern,
                  to_string(mid + 1).c_str(),
                  to_string(end).c_str(),
                  to_string(max_chunk_size).c_str(),
                  (char *)NULL);
            perror("execl failed");
            exit(1);
        }
        cout << "[" << my_pid << "] forked right child " << right << "\n";

        // wait for both
        waitpid(left, NULL, 0);
        cout << "[" << my_pid << "] left child returned\n";

        waitpid(right, NULL, 0);
        cout << "[" << my_pid << "] right child returned\n";
    }

    return 0;
}
