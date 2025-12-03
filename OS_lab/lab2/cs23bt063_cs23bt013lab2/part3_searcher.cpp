#include <iostream>
#include <fstream>
#include <string>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>

using namespace std;

pid_t parent_pid;

void handle_sigterm(int signum) {
    cout << "[" << getpid() << "] received SIGTERM\n";
    exit(0);
}

int main(int argc, char **argv) {
    if (argc != 5) {
        cerr << "usage: ./part3_searcher.out <file> <pattern> <start> <end>\n";
        return -1;
    }

    signal(SIGTERM, handle_sigterm);

    char *filename = argv[1];
    string pattern = argv[2];
    int start = atoi(argv[3]);
    int end = atoi(argv[4]);

    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "[" << getpid() << "] could not open file\n";
        return -1;
    }

    file.seekg(start);
    string buffer;
    char c;
    int pos = start;

    while (file.get(c) && pos < end) {
        buffer.push_back(c);
        if (buffer.size() > pattern.size())
            buffer.erase(buffer.begin());  // keep window

        if (buffer == pattern) {
            cout << "[" << getpid() << "] found at position " << pos - pattern.size() + 1 << "\n";

            // kill all other processes
            kill(-getpgrp(), SIGTERM);
            return 0;
        }
        pos++;
    }

    cout << "[" << getpid() << "] didn't find\n";
    return 0;
}
