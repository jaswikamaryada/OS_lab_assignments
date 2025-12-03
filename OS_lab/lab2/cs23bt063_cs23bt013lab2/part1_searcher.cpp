#include <iostream>
#include <fstream>
#include <cstring>
#include <unistd.h>
#include <signal.h>

using namespace std;

int main(int argc, char **argv)
{
	if(argc != 5)
	{
		cout <<"usage: ./partitioner.out <path-to-file> <pattern> <search-start-position> <search-end-position>\nprovided arguments:\n";
		for(int i = 0; i < argc; i++)
			cout << argv[i] << "\n";
		return -1;
	}
	
	char *file_to_search_in = argv[1];
	char *pattern_to_search_for = argv[2];
	int search_start_position = atoi(argv[3]);
	int search_end_position = atoi(argv[4]);

	//TODO
    ifstream file(file_to_search_in);
    if(!file)
    {
        cout<<"ERROR";
        return -1;
    }
    // if(*pattern_to_search_for in file)
    // {
    //     cout<<getpid(),"found at",search_start_position + position;
    //     return 1;
    // } 
    // else{
    //     cout<<not found;
    //     return -1;
    // }
    file.seekg(search_start_position);
    int length_to_check= search_end_position - search_start_position + 1; 
   // file.read(&buffer[0], length);
    string buffer(length_to_check, '\0');
    file.read(&buffer[0], length_to_check);
    buffer.resize(file.gcount());
    
    size_t pos = buffer.find(pattern_to_search_for);
    if (pos < buffer.length()) {

        cout << "[" << getpid() << "] found at " << (search_start_position + pos) << "\n";
        return 1;
   

    
    }
    else {
        cout << "[" << getpid() << "] didn't find\n";
    }

   


    return 0;
}






// 	cout << "[-1] didn't find\n";
// 	return 0;
// }
