#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
//
#include <ctime> // time.h
#include <cstdlib> // stdlib.h
#include <cstdio> //stdio.h

using namespace std;

int main(int argc, char *argv[]){
	int replace_count = 0;
	// int num_total_read = 0;

	// open the input and output file
	ifstream input_file(argv[1], ios::in);
	if(!input_file){
		cerr<<"something wrong in input file"<<endl;
		exit(1);
	}
	ofstream output_file(argv[2], ios::out);
	if(!output_file){
		cerr<<"something wrong in output file"<<endl;
		exit(1);
	}

	// read the settingparameter form input file
	int cache_size = 0, block_size = 0, associativity = 0, policy = 0;
	input_file >> cache_size;
	input_file >> block_size;
	input_file >> associativity;
	input_file >> policy;

    //
    //associativity = 1;
	// policy = 2;
	//

	cout << "cache_size: " << cache_size << endl;
	cout << "block_size: " << block_size << endl;
	cout << "associativity: " << associativity << endl;
	cout << "policy: " << policy << endl;
    cout << endl;
	///////////////////  index offset  ///////////////////////
	int num_blocks = cache_size * 1024 / block_size; // cache size(KB,need to time 2^10)
	int temp_num_blocks = num_blocks; // to cal the num_index_bits

	int num_index_bits = 0;
	int num_offset_bits = 0;
	while(temp_num_blocks != 1){
		temp_num_blocks = temp_num_blocks >> 1 ;
		++num_index_bits;
	}
	while(block_size != 1){
		block_size = block_size >> 1;
		++num_offset_bits;
	}
    
    cout << "blocks: " << num_blocks << endl;
   /////////////////// tag  set entry  ///////////////////////

	int num_sets = 0, num_entries = 0;
	int num_tag_bits = 32 - num_index_bits - num_offset_bits;
    
	if(associativity == 0){ // direct map
		num_sets = num_blocks;
		num_entries = 1;
	}
	else if(associativity == 1){ // 4-way
		num_sets = num_blocks / 4;
		num_entries = 4;
		num_index_bits -= 2;
		num_tag_bits += 2;
	}
	else if(associativity == 2){ // full-way
		num_sets = 1;
		num_entries = num_blocks;
		num_tag_bits += num_index_bits;
		num_index_bits = 0;
	}
	
    cout << "tag: " << num_tag_bits << "  index: " << num_index_bits << "  offset: " << num_offset_bits << endl;
    cout << "sets: " << num_sets << "  entries: " << num_entries << endl;
    cout << endl;


    //////////////////////////////////////////
	int valid[num_sets][num_entries];

	cout << "checkpoint 1" << endl;
	unsigned int tag[num_sets][num_entries]; // warning: this line will stop excute the following program without any warning when read trace3.txt
	cout << "checkpoint 2" << endl;

	// count for potential victim by policy
	int FIFO[num_sets]; // 0
	int LRU[num_sets][num_entries]; // 1
	int LRU_count[num_sets];
	
    // initialize variants
	for(int i = 0;i < num_sets;i++){
		LRU_count[i] = 0;
		FIFO[i] = -1;
		for(int j = 0;j < num_entries;j++){
			valid[i][j] = 0;
			tag[i][j] = 0;
			LRU[i][j] = -1;
		}
	}

	string str;
	srand(time(NULL));

	int count = 0; 
	while(input_file >> str){
		count++;

		// variants for handle adress
		unsigned int address;
		stringstream ss; // stringstream: from <sstream>
		ss << std::hex << str;
		ss >> address;

        

		unsigned int tag_value = address >> num_offset_bits >> num_index_bits;
		unsigned int index = ((unsigned int)(address << num_tag_bits)) >> num_tag_bits >> num_offset_bits;

        // cout << "tag value: " << tag_value << endl;
        // cout << "index: " << index << endl;
		// cout << "num_entries:" <<num_entries<<endl;

        // traversal all the set(which position are considered as index)
		for(int i = 0;i < num_entries;i++){

			// if hit
			if(valid[index][i] && tag[index][i] == tag_value){
				// cout << "hit"<<endl;
				output_file << -1 << endl;

				LRU_count[index]++; // each set has its count.
				LRU[index][i] = LRU_count[index]; // update the current entry's rank by LRU_count
				
                break;
			}
			// if not a valid entry(miss), then update this entry
			else if(!valid[index][i]){
				// cout << "update"<<endl;
				output_file << -1 << endl;
				tag[index][i] = tag_value;
				valid[index][i] = 1;

                // find potential victim(depend on policy)
                FIFO[index] = ++FIFO[index] % num_entries;
				LRU_count[index]++;
				LRU[index][i] = LRU_count[index];
				

				break;
			}
			// if in the last one entry, don't have wanted block,  replace. (Choose the replacement policy)
			else if(i == (num_entries -1)){
				replace_count++;
				// cout << "replace"<<endl;
				int victim_entry = 0; // find the victim entry
				if(num_entries == 1){
					victim_entry = 0;
				}
				else{
					//FIFO
					if(policy == 0){
						victim_entry = FIFO[index];
						FIFO[index] = ++FIFO[index] % num_entries;
					}
					//LRU
					else if(policy == 1){
                        // find the smallest count in the set
						for(int k = 0;k < num_entries;k++){
							if(LRU[index][k] < LRU[index][victim_entry]){ 
							  victim_entry = k;
							}
						}
						LRU[index][victim_entry] = ++LRU_count[index];
					}
					//RR
					else if(policy == 2){
						victim_entry = rand() % num_entries;
					}
					
				}
				output_file << tag[index][victim_entry] << endl;
				tag[index][victim_entry] = tag_value;
			}

		}
	}
	cout << "\n\nreplace count: " <<replace_count<<endl;

	// close the file
	input_file.close();
	output_file.close();

	return 0;
}