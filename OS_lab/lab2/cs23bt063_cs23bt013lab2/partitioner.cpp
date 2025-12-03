#include <iostream>
#include <fstream>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>

using namespace std;

int main(int argc, char **argv)
{
	if(argc != 6)
	{
		cout <<"usage: ./partitioner.out <path-to-file> <pattern> <search-start-position> <search-end-position> <max-chunk-size>\nprovided arguments:\n";
		for(int i = 0; i < argc; i++)
			cout << argv[i] << "\n";
		return -1;
	}
	
	char *file_to_search_in = argv[1];
	char *pattern_to_search_for = argv[2];
	int search_start_position = atoi(argv[3]);
	int search_end_position = atoi(argv[4]);
	int max_chunk_size = atoi(argv[5]);
	
	//TODO
	//cout << "[" << my_pid << "] start position = " << search_start_position << " ; end position = " << search_end_position << "\n";
	//cout << "[" << my_pid << "] forked left child " << my_children[0] << "\n";
	//cout << "[" << my_pid << "] forked right child " << my_children[1] << "\n";
	//cout << "[" << my_pid << "] left child returned\n";
	//cout << "[" << my_pid << "] right child returned\n";
	//cout << "[" << my_pid << "] left child returned\n";
	//cout << "[" << my_pid << "] right child returned\n";*/
	//cout << "[" << my_pid << "] forked searcher child " << searcher_pid << "\n";
	//cout << "[" << my_pid << "] searcher child returned \n";
	//cout << "[" << my_pid << "] received SIGTERM\n"; //applicable for Part III of the assignment

	return 0;
}
