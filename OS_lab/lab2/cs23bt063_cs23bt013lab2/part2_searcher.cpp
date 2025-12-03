#include <iostream>
#include <fstream>
#include <unistd.h>
#include <cstdlib>

using namespace std;

int main(int argc, char **argv)
{
    if(argc != 5)
    {
        cerr << "usage: ./part2_searcher.out <file> <pattern> <start> <end>\n";
        return -1;
    }

    char *file_to_search_in = argv[1];
    string pattern = argv[2];
    int start_pos = atoi(argv[3]);
    int end_pos = atoi(argv[4]);

    pid_t my_pid = getpid();

    ifstream infile(file_to_search_in);
    if(!infile.is_open())
    {
        cerr << "[" << my_pid << "] error: could not open file\n";
        return -1;
    }

    // Seek to the start position
    infile.seekg(start_pos);
    string buffer(end_pos - start_pos + 1, '\0');
    infile.read(&buffer[0], buffer.size());

    size_t found = buffer.find(pattern);

    if(found != string::npos)
    {
        cout << "[" << my_pid << "] found at " << (start_pos + found) << "\n";
    }
    else
    {
        cout << "[" << my_pid << "] didn't find\n";
    }

    return 0;
}
