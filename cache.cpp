#include <iostream>
#include <string>
#include <vector>
#include <fstream>
//
#include <sstream>
#include <ctime> // time.h
#include <cstdlib> // stdlib.h
#include <cstdio> //stdio.h
//
#include <bitset>

using namespace std;

int main(int argc, char *argv[]){

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

	//////////////////////////////////////////
	int num_blocks = cache_size * 1024 / block_size;
	int temp_num_blocks = num_blocks; // to cal the num_indexes

	int num_indexes = 0;
	int offset = 0;
	while(temp_num_blocks != 1){
		temp_num_blocks = temp_num_blocks >> 1 ;
		++num_indexes;
	}
	while(block_size != 1){
		block_size = block_size >> 1;
		++offset;
	}
   //////////////////////////////////////////

	int num_sets = 0, num_entries = 0;
	int num_tag_bits = 32 - num_indexes - offset;

	if(associativity == 0){ // direct map
		num_sets = num_blocks;
		num_entries = 1;
	}
	else if(associativity == 1){ // 4-way
		num_sets = num_blocks / 4;
		num_entries = 4;
		num_indexes -= 2;
		num_tag_bits += 2;
	}
	else if(associativity == 2){ // full-way
		num_sets = 1;
		num_entries = num_blocks;
		num_tag_bits += num_indexes;
		num_indexes = 0;
	}
    //////////////////////////////////////////

	int valid[num_sets][num_entries];
	unsigned int tag[num_sets][num_entries];
	int FIFO[num_sets];
	int LRU[num_sets][num_entries];
	int LRU_current[num_sets];
	int RR[num_sets][num_entries];

    // initialize variants
	for(int i = 0;i < num_sets;i++){
		LRU_current[i] = 0;
		FIFO[i] = -1;
		for(int j = 0;j < num_entries;j++){
			valid[i][j] = 0;
			tag[i][j] = 0;
			LRU[i][j] = -1;
			RR[i][j] = 0;
		}
	}

	string str;
	int count = 0; 

	srand(time(NULL)); // 

	while(input_file >> str){
		count++;

		// variants for handle adress
		unsigned int address;
		stringstream ss; // stringstream: from <sstream?
		ss << std::hex << str;
		ss >> address;

		unsigned int tag_value = address >> offset >> num_indexes;
		unsigned int index = ((unsigned int)(address << num_tag_bits)) >> num_tag_bits >> offset;

		
		for(int i = 0;i < num_entries;i++){
			// if hit
			if(valid[index][i] && tag[index][i] == tag_value){
				output_file << -1 << endl;
				LRU_current[index]++;
				LRU[index][i] = LRU_current[index];

				RR[index][i]++;
				break;
			}
			// if not a valid entry(miss), then update this entry
			else if(!valid[index][i]){
				output_file << -1 << endl;
				tag[index][i] = tag_value;
				valid[index][i] = 1;
				LRU_current[index]++;
				LRU[index][i] = LRU_current[index];
				FIFO[index] = ++FIFO[index] % num_entries;
				// self-defined policy
				RR[index][i]++;
				break;
			}
			// if miss and valid, then replace. Choose the replacement policy
			else if(i == (num_entries -1)){
				int search = 0;
				if(num_entries == 1){
					search = 0;
				}
				else{
					//FIFO
					if(policy == 0){
						//cout << "FIFO\n";
						search = FIFO[index];
						FIFO[index] = ++FIFO[index] % num_entries;
					}
					//LRU
					else if(policy == 1){
						for(int k = 0;k < num_entries;k++){
							if(LRU[index][k] < LRU[index][search]){
							  search = k;
							}
						}
						LRU[index][search] = ++LRU_current[index];
					}
					//RR
					else if(policy == 2){
						search = rand % num_entries;
						RR[index][search]++;
					}
				}
				output_file << tag[index][search] << endl;
				tag[index][search] = tag_value;
			}
		}
	}

	// close the file
	input_file.close();
	output_file.close();

	return 0;
}