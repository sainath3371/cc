#include <bits/stdc++.h>
#include <iostream>
#include <fstream>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <iomanip>
#include <stdlib.h>
#include <cmath>
#include <bitset>
#include <stdio.h>
#include <map>
using namespace std;

class cache{  

    public:
    vector<vector<int>> cache_L1;
    vector<vector<int>> cache_L2;
    vector<vector<int>> valid_L1;
    vector<vector<int>> valid_L2;
    vector<vector<int>> modified_L1;
    vector<vector<int>> modified_L2;

    //LRU array for eviction
    vector<vector<int>> lru_L1;
    vector<vector<int>> lru_L2;

    int  num_sets_L1, num_sets_L2, waylen_L1, waylen_L2;
    // All belows variables for the divison of the addr
    int  tagoffset_L1, tagoffset_L2, blockoffset, indexoffset_L1, indexoffset_L2;

    
        cache(int  i1, int  i2, int  i3, int  i4, int  i5){
            this->waylen_L1 = (int )i3;
            this->waylen_L2 = (int )i5;
            this->blockoffset = (int )log2(i1);
            this->num_sets_L1 = (int )(i2/(i1*i3));
            this->num_sets_L2 = (int )(i4/(i1*i5));
            this->indexoffset_L1 = (int )log2(num_sets_L1);
            this->indexoffset_L2 = (int )log2(num_sets_L2);
            this->tagoffset_L1 = 32 - this->indexoffset_L1 - this->blockoffset;
            this->tagoffset_L2 = 32 - this->indexoffset_L2 - this->blockoffset;
            //intialise cache_L1, cache_L2

            this->cache_L1.resize(waylen_L1);
            this->valid_L1.resize(waylen_L1);
            this->modified_L1.resize(waylen_L1);

            this->cache_L2.resize(waylen_L2);
            this->valid_L2.resize(waylen_L2);
            this->modified_L2.resize(waylen_L2);

            for(int  i=0; i<waylen_L1;i++){
                this->cache_L1[i].resize(num_sets_L1);
                this->valid_L1[i].resize(num_sets_L1);
                this->modified_L1[i].resize(num_sets_L1);
            }

            for(int  i=0; i<waylen_L2;i++){
                this->cache_L2[i].resize(num_sets_L2);  
                this->valid_L2[i].resize(num_sets_L2);
                this->modified_L2[i].resize(num_sets_L2);
            }

            //lru
            this->lru_L1.resize(num_sets_L1);
            for(int  i=0; i< num_sets_L1; i++)
                this->lru_L1[i].resize(waylen_L1);

            this->lru_L2.resize(num_sets_L2);
            for(int  i=0; i< num_sets_L2; i++)
                this->lru_L2[i].resize(waylen_L2);
            
        }

        vector<string> parser(bitset<32> addr){
            string Addr = addr.to_string();
            vector<string> offset(6);
            offset[0] = Addr.substr(0, this->tagoffset_L1);
            offset[1] = Addr.substr(this->tagoffset_L1, this->indexoffset_L1);
            offset[2] = Addr.substr(this->tagoffset_L1+this->indexoffset_L1, this->blockoffset);

            offset[3] = Addr.substr(0, this->tagoffset_L2);
            offset[4] = Addr.substr(this->tagoffset_L2, this->indexoffset_L2);
            offset[5] = Addr.substr(this->tagoffset_L2+this->indexoffset_L2, this->blockoffset);
            return offset;
        }

        vector<string> convertL1_toL2(string &index_L1, string &tag_L1){
            
            string temp = tag_L1 + index_L1;

            vector<string> offset(2);
            offset[0] = temp.substr(0, this->tagoffset_L2);
            offset[1] = temp.substr(this->tagoffset_L2, this->indexoffset_L2);
            return offset;
        }

};

int  iswayavailable(vector<vector<int >> &valid_L, int  &index_L, int  &L_ways){
    for(int  i=0; i<L_ways; i++){  
        if(valid_L[i][index_L] ==0)
            return (i+1);
    }
    return 0;
}

int  lru_evict(vector<vector<int >> &lru_L, int  &index_L, int  &L_ways){
    int  evict_way;
    for(int  i=0; i<L_ways; i++){
        if(lru_L[index_L][i]==4)
            evict_way = i;
        else
            lru_L[index_L][i]++;//update the lru
    }
    lru_L[index_L][evict_way] = 1;
    return evict_way;//return the way which will be evicted evict.
}

void add_to_cache(vector<vector<int >> &cache_L, vector<vector<int >> &valid_L, int  &index_L, int  way, int  &tag){
    cache_L[way][index_L] = tag;
    valid_L[way][index_L] = 1;         
}


int main(int argc, char** argv){

    int  L1_reads=0, L1_read_miss=0, L1_writes=0, L1_write_miss=0;
    char * ptr;
    cout<<argv[1];
    int  s1 = stoi(argv[1]);
    int  s2 = stoi(argv[2]);
    int  s3 = stoi(argv[3]);
    int  s4 = stoi(argv[4]);
    int  s5 = stoi(argv[5]);

    cache cache_obj(s1,s2,s3,s4,s5);

    int  L1_ways = stoi(argv[3]);
    int  L2_ways = stoi(argv[5]);

    ifstream traces;
    traces.open(argv[6]);
    string line;
    string accesstype;  // the Read/Write access type from the memory trace;
    string xaddr;       // the address from the memory trace store in hex;
    unsigned int addr;  // the address from the memory trace store in unsigned int;
    bitset<32> addr_bits; // the address from the memory trace store in the bitset;

    if(traces.is_open()){
        while(getline(traces, line)){
            istringstream iss(line);
            if(!(iss >> accesstype >> xaddr)) { break; }

            stringstream saddr(xaddr);
            saddr >> std::hex >> addr;

            addr_bits = bitset<32>(addr);

            vector<string> offsets = cache_obj.parser(addr_bits);

            int  index_L1= stoi(offsets[1]);
            int  index_L2 = stoi(offsets[4]);
            
            int   tag_L1= stoi(offsets[0]);
            int   tag_L2= stoi(offsets[3]);

            int  in_L1=0, in_L2 =0;

            //check the adrr in L1.
            for(int  i =0; i<L1_ways; i++){
                if((cache_obj.cache_L1[i][index_L1]==tag_L1)&&(cache_obj.valid_L1[i][index_L1]==1)){
                    in_L1 = 1;
                    break;
                }
            }
            //check the adrr in L2
            if(in_L1 == 0){
                for(int  i=0; i<L2_ways; i++){
                    if((cache_obj.cache_L2[i][index_L2]==tag_L2)&&(cache_obj.valid_L2[i][index_L2]==1)){
                        in_L2 = 1;
                        break;
                    }
                }
            }


            if(accesstype.compare("r") == 0){
                L1_reads++;
                //L1 hit
                if(in_L1==0&&in_L2==0){
                    //get data from RAM
                    //check if ways in L2 are available.
                    L1_read_miss++;
                    int  free_way = iswayavailable(cache_obj.valid_L2, index_L2, L2_ways);
                    int  evict_way;

                    //update in L2.
                    if(free_way==0){
                        evict_way = lru_evict(cache_obj.lru_L2, index_L2, L2_ways);
                        //if evict_way modified bit is 1 then it will updated in RAM
                        add_to_cache(cache_obj.cache_L2, cache_obj.valid_L2, index_L2, evict_way, tag_L2);
                    }
                    else{
                        for(int  i=0; i<free_way-1; i++){
                            cache_obj.lru_L2[index_L2][i]++;
                        }
                        cache_obj.lru_L2[index_L2][free_way-1] = 1;
                        add_to_cache(cache_obj.cache_L2, cache_obj.valid_L2, index_L2, free_way-1, tag_L2);
                    }

                    //update in L1.
                    free_way = iswayavailable(cache_obj.valid_L1, index_L1, L1_ways);
                    if(free_way==0){
                        evict_way = lru_evict(cache_obj.lru_L1, index_L1, L1_ways);
                        //if evict way modified bit is 1 the we need to update in L2 too.call fucntion to directly replace the page in L2
                        vector<string> temp = cache_obj.convertL1_toL2(offsets[1], offsets[0]);
                        //can call function as we took note of tag resided in it
                        add_to_cache(cache_obj.cache_L1, cache_obj.valid_L1, index_L1, evict_way, tag_L1);

                        if(cache_obj.modified_L1[evict_way][index_L1]==1){
                            int  temp_tag = stoi(temp[0]);
                            int  temp_index = stoi(temp[1]);
                            free_way = iswayavailable(cache_obj.valid_L2, temp_index, L2_ways);
                            if(free_way==0){
                                evict_way = lru_evict(cache_obj.lru_L2, temp_index, L2_ways);
                                //if evict_way modified bit is 1 then it will updated in RAM
                                add_to_cache(cache_obj.cache_L2, cache_obj.valid_L2, temp_index, evict_way, temp_tag);
                            }
                            else{
                                for(int  i=0; i<free_way-1; i++){
                                    cache_obj.lru_L2[temp_index][i]++;
                                }
                                cache_obj.lru_L2[temp_index][free_way-1] == 1;
                                add_to_cache(cache_obj.cache_L2, cache_obj.valid_L2, temp_index, free_way-1, temp_tag);
                            }
                            //evict completed from L1 to L2 due to modified bit==1.
                        }
                        cache_obj.modified_L1[evict_way][index_L1] = 0;
                    }
                    else{
                        for(int  i=0; i<free_way-1; i++){
                            cache_obj.lru_L1[index_L1][i]++;
                        }
                        cache_obj.lru_L1[index_L1][free_way-1] == 1;
                        add_to_cache(cache_obj.cache_L1, cache_obj.valid_L1, index_L1, free_way-1, tag_L1);
                    }

                }//update only L1
                else if(in_L2 == 1 && in_L1 == 0){
                    L1_read_miss;
                    int  free_way = iswayavailable(cache_obj.valid_L1, index_L1, L1_ways);
                    int  evict_way;

                    if(free_way==0){
                        evict_way = lru_evict(cache_obj.lru_L1, index_L1, L1_ways);
                        //if evict way modified bit is 1 the we need to update in L2 too.call fucntion to directly replace the page in L2
                        vector<string> temp = cache_obj.convertL1_toL2(offsets[1], offsets[0]);
                        //can call function as we took note of tag resided in it
                        add_to_cache(cache_obj.cache_L1, cache_obj.valid_L1, index_L1, evict_way, tag_L1);

                        if(cache_obj.modified_L1[evict_way][index_L1]==1){
                            int  temp_tag = stoi(temp[0]);
                            int  temp_index = stoi(temp[1]);
                            free_way = iswayavailable(cache_obj.valid_L2, temp_index, L2_ways);
                            if(free_way==0){
                                evict_way = lru_evict(cache_obj.lru_L2, temp_index, L2_ways);
                                //if evict_way modified bit is 1 then it will updated in RAM
                                add_to_cache(cache_obj.cache_L2, cache_obj.valid_L2, temp_index, evict_way, temp_tag);
                            }
                            else{
                                for(int  i=0; i<free_way-1; i++){
                                    cache_obj.lru_L2[temp_index][i]++;
                                }
                                cache_obj.lru_L2[temp_index][free_way-1] == 1;
                                add_to_cache(cache_obj.cache_L2, cache_obj.valid_L2, temp_index, free_way-1, temp_tag);
                            }
                            //evict completed from L1 to L2 due to modified bit==1.
                        }
                        cache_obj.modified_L1[evict_way][index_L1] = 0;
                    }
                    else{
                        for(int  i=0; i<free_way-1; i++){
                            cache_obj.lru_L1[index_L1][i]++;
                        }
                        cache_obj.lru_L1[index_L1][free_way-1] == 1;
                        add_to_cache(cache_obj.cache_L1, cache_obj.valid_L1, index_L1, free_way-1, tag_L1);
                    }

                }
                else{
                    //L1_hit.
                }
            }//write
            else{
                L1_writes++;
                if(in_L1==0&&in_L2==0){
                    //get data from RAM
                    //check if ways in L2 are available.
                    int  free_way = iswayavailable(cache_obj.valid_L2, index_L2, L2_ways);
                    int  evict_way;

                    //update in L2.
                    if(free_way==0){
                        evict_way = lru_evict(cache_obj.lru_L2, index_L2, L2_ways);
                        //if evict_way modified bit is 1 then it will updated in RAM
                        add_to_cache(cache_obj.cache_L2, cache_obj.valid_L2, index_L2, evict_way, tag_L2);
                    }
                    else{
                        for(int  i=0; i<free_way-1; i++){
                            cache_obj.lru_L2[index_L2][i]++;
                        }
                        cache_obj.lru_L2[index_L2][free_way-1] = 1;
                        add_to_cache(cache_obj.cache_L2, cache_obj.valid_L2, index_L2, free_way-1, tag_L2);
                    }

                    //update in L1.
                    free_way = iswayavailable(cache_obj.valid_L1, index_L1, L1_ways);
                    if(free_way==0){
                        evict_way = lru_evict(cache_obj.lru_L1, index_L1, L1_ways);
                        //if evict way modified bit is 1 the we need to update in L2 too.call fucntion to directly replace the page in L2
                        vector<string> temp = cache_obj.convertL1_toL2(offsets[1], offsets[0]);
                        //can call function as we took note of tag resided in it
                        add_to_cache(cache_obj.cache_L1, cache_obj.valid_L1, index_L1, evict_way, tag_L1);

                        if(cache_obj.modified_L1[evict_way][index_L1]==1){
                            int  temp_tag = stoi(temp[0]);
                            int  temp_index = stoi(temp[1]);
                            free_way = iswayavailable(cache_obj.valid_L2, temp_index, L2_ways);
                            if(free_way==0){
                                evict_way = lru_evict(cache_obj.lru_L2, temp_index, L2_ways);
                                //if evict_way modified bit is 1 then it will updated in RAM
                                add_to_cache(cache_obj.cache_L2, cache_obj.valid_L2, temp_index, evict_way, temp_tag);
                            }
                            else{
                                for(int  i=0; i<free_way-1; i++){
                                    cache_obj.lru_L2[temp_index][i]++;
                                }
                                cache_obj.lru_L2[temp_index][free_way-1] == 1;
                                add_to_cache(cache_obj.cache_L2, cache_obj.valid_L2, temp_index, free_way-1, temp_tag);
                            }
                            //evict completed from L1 to L2 due to modified bit==1.
                        }

                        cache_obj.modified_L1[evict_way][index_L1] = 1;
                    }
                    else{
                        for(int  i=0; i<free_way-1; i++){
                            cache_obj.lru_L1[index_L1][i]++;
                        }
                        cache_obj.lru_L1[index_L1][free_way-1] == 1;
                        add_to_cache(cache_obj.cache_L1, cache_obj.valid_L1, index_L1, free_way-1, tag_L1);
                        cache_obj.modified_L1[evict_way][index_L1] = 1;
                    }

                }//update only L1
                else if(in_L2 == 1 && in_L1 == 0){
                    
                    int  free_way = iswayavailable(cache_obj.valid_L1, index_L1, L1_ways);
                    int  evict_way;

                    if(free_way==0){
                        evict_way = lru_evict(cache_obj.lru_L1, index_L1, L1_ways);
                        //if evict way modified bit is 1 the we need to update in L2 too.call fucntion to directly replace the page in L2
                        vector<string> temp = cache_obj.convertL1_toL2(offsets[1], offsets[0]);
                        //can call function as we took note of tag resided in it
                        add_to_cache(cache_obj.cache_L1, cache_obj.valid_L1, index_L1, evict_way, tag_L1);

                        if(cache_obj.modified_L1[evict_way][index_L1]==1){
                            int  temp_tag = stoi(temp[0]);
                            int  temp_index = stoi(temp[1]);
                            free_way = iswayavailable(cache_obj.valid_L2, temp_index, L2_ways);
                            if(free_way==0){
                                evict_way = lru_evict(cache_obj.lru_L2, temp_index, L2_ways);
                                //if evict_way modified bit is 1 then it will updated in RAM
                                add_to_cache(cache_obj.cache_L2, cache_obj.valid_L2, temp_index, evict_way, temp_tag);
                            }
                            else{
                                for(int  i=0; i<free_way-1; i++){
                                    cache_obj.lru_L2[temp_index][i]++;
                                }
                                cache_obj.lru_L2[temp_index][free_way-1] == 1;
                                add_to_cache(cache_obj.cache_L2, cache_obj.valid_L2, temp_index, free_way-1, temp_tag);
                            }
                            //evict completed from L1 to L2 due to modified bit==1.
                        }
                        cache_obj.modified_L1[evict_way][index_L1] = 1;
                    }
                    else{
                        for(int  i=0; i<free_way-1; i++){
                            cache_obj.lru_L1[index_L1][i]++;
                        }
                        cache_obj.lru_L1[index_L1][free_way-1] == 1;
                        add_to_cache(cache_obj.cache_L1, cache_obj.valid_L1, index_L1, free_way-1, tag_L1);
                        cache_obj.modified_L1[evict_way][index_L1] = 1;
                    }
                }
                else{
                    for(int  i =0; i<L1_ways; i++){
                        if((cache_obj.cache_L1[i][index_L1]==tag_L1)&&(cache_obj.valid_L1[i][index_L1]==1)){
                            cache_obj.modified_L1[i][index_L1] = 1;
                            break;
                        }
                    }
                    
                }

            }

        }
    }

    cout<<L1_reads<<endl<<L1_read_miss<<endl<<L1_writes;
    return 0;
}




