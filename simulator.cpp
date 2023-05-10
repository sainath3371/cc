#include <bits/stdc++.h>
#include <iostream>
#include <fstream>
using namespace std;

class cache{  

    public:
    vector<vector<unsigned long>> cache_L1;
    vector<vector<unsigned long>> cache_L2;
    vector<vector<unsigned long>> valid_L1;
    vector<vector<unsigned long>> valid_L2;
    vector<vector<unsigned long>> modified_L1;
    vector<vector<unsigned long>> modified_L2;

    //LRU array for eviction
    vector<vector<unsigned long>> lru_L1;
    vector<vector<unsigned long>> lru_L2;

    unsigned long num_sets_L1, num_sets_L2, waylen_L1, waylen_L2;
    // All belows variables for the divison of the addr
    unsigned long tagoffset_L1, tagoffset_L2, blockoffset, indexoffset_L1, indexoffset_L2;

    
        cache(unsigned long i1, unsigned long i2, unsigned long i3, unsigned long i4, unsigned long i5){
            this->waylen_L1 = (unsigned long)i3;
            this->waylen_L2 = (unsigned long)i5;
            this->blockoffset = (unsigned long)log2(i1);
            this->num_sets_L1 = (unsigned long)(i2/(i1*i3));
            this->num_sets_L2 = (unsigned long)(i4/(i1*i5));
            this->indexoffset_L1 = (unsigned long)log2(num_sets_L1);
            this->indexoffset_L2 = (unsigned long)log2(num_sets_L2);
            this->tagoffset_L1 = 32 - this->indexoffset_L1 - this->blockoffset;
            this->tagoffset_L2 = 32 - this->indexoffset_L2 - this->blockoffset;
            //intialise cache_L1, cache_L2

            this->cache_L1.resize(waylen_L1);
            this->valid_L1.resize(waylen_L1);
            this->modified_L1.resize(waylen_L1);

            this->cache_L2.resize(waylen_L2);
            this->valid_L2.resize(waylen_L2);
            this->modified_L2.resize(waylen_L2);

            for(unsigned long i=0; i<waylen_L1;i++){
                this->cache_L1[i].resize(num_sets_L1);
                this->valid_L1[i].resize(num_sets_L1);
                this->modified_L1[i].resize(num_sets_L1);
            }

            for(unsigned long i=0; i<waylen_L2;i++){
                this->cache_L2[i].resize(num_sets_L2);  
                this->valid_L2[i].resize(num_sets_L2);
                this->modified_L2[i].resize(num_sets_L2);
            }

            //lru
            this->lru_L1.resize(num_sets_L1);
            for(unsigned long i=0; i< num_sets_L1; i++)
                this->lru_L1[i].resize(waylen_L1);

            this->lru_L2.resize(num_sets_L2);
            for(unsigned long i=0; i< num_sets_L2; i++)
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

unsigned long iswayavailable(vector<vector<unsigned long>> &valid_L, unsigned long &index_L, unsigned long &L_ways){
    for(unsigned long i=0; i<L_ways; i++){  
        if(valid_L[i][index_L] ==0)
            return (i+1);
    }
    return 0;
}

unsigned long lru_evict(vector<vector<unsigned long>> &lru_L, unsigned long &index_L, unsigned long &L_ways){
    unsigned long evict_way;
    for(unsigned long i=0; i<L_ways; i++){
        if(lru_L[index_L][i]==4)
            evict_way = i;
        else
            lru_L[index_L][i]++;//update the lru
    }
    lru_L[index_L][evict_way] = 1;
    return evict_way;//return the way which will be evicted evict.
}

void add_to_cache(vector<vector<unsigned long>> &cache_L, vector<vector<unsigned long>> &valid_L, unsigned long &index_L, unsigned long way, unsigned long &tag){
    cache_L[way][index_L] = tag;
    valid_L[way][index_L] = 1;         
}




int main(int argc, char* argv[]){

    unsigned long L1_reads=0, L1_read_miss=0, L1_writes=0, L1_write_miss=0;

    cache cache_obj(strtol(argv[1], NULL, 10), strtol(argv[2], NULL, 10), strtol(argv[3], NULL, 10), strtol(argv[4], NULL, 10), strtol(argv[5], NULL, 10));

    char * ptr;
    unsigned long L1_ways = strtol(argv[3], &ptr, 10);
    unsigned long L2_ways = strtol(argv[5], &ptr, 10);

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

            unsigned long index_L1= strtol(offsets[1].c_str(), &ptr, 2);
            unsigned long index_L2 = strtol(offsets[4].c_str(), &ptr, 2);
            
            unsigned long  tag_L1= strtol(offsets[0].c_str(), &ptr, 2);
            unsigned long  tag_L2= strtol(offsets[3].c_str(), &ptr, 2);

            unsigned long in_L1=0, in_L2 =0;

            //check the adrr in L1.
            for(unsigned long i =0; i<L1_ways; i++){
                if((cache_obj.cache_L1[i][index_L1]==tag_L1)&&(cache_obj.valid_L1[i][index_L1]==1)){
                    in_L1 = 1;
                    break;
                }
            }
            //check the adrr in L2
            if(in_L1 == 0){
                for(unsigned long i=0; i<L2_ways; i++){
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
                    unsigned long free_way = iswayavailable(cache_obj.valid_L2, index_L2, L2_ways);
                    unsigned long evict_way;

                    //update in L2.
                    if(free_way==0){
                        evict_way = lru_evict(cache_obj.lru_L2, index_L2, L2_ways);
                        //if evict_way modified bit is 1 then it will updated in RAM
                        add_to_cache(cache_obj.cache_L2, cache_obj.valid_L2, index_L2, evict_way, tag_L2);
                    }
                    else{
                        for(unsigned long i=0; i<free_way-1; i++){
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
                            unsigned long temp_tag = strtol(temp[0].c_str(), &ptr, 2);
                            unsigned long temp_index = strtol(temp[1].c_str(), &ptr, 2);
                            free_way = iswayavailable(cache_obj.valid_L2, temp_index, L2_ways);
                            if(free_way==0){
                                evict_way = lru_evict(cache_obj.lru_L2, temp_index, L2_ways);
                                //if evict_way modified bit is 1 then it will updated in RAM
                                add_to_cache(cache_obj.cache_L2, cache_obj.valid_L2, temp_index, evict_way, temp_tag);
                            }
                            else{
                                for(unsigned long i=0; i<free_way-1; i++){
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
                        for(unsigned long i=0; i<free_way-1; i++){
                            cache_obj.lru_L1[index_L1][i]++;
                        }
                        cache_obj.lru_L1[index_L1][free_way-1] == 1;
                        add_to_cache(cache_obj.cache_L1, cache_obj.valid_L1, index_L1, free_way-1, tag_L1);
                    }

                }//update only L1
                else if(in_L2 == 1 && in_L1 == 0){
                    L1_read_miss;
                    unsigned long free_way = iswayavailable(cache_obj.valid_L1, index_L1, L1_ways);
                    unsigned long evict_way;

                    if(free_way==0){
                        evict_way = lru_evict(cache_obj.lru_L1, index_L1, L1_ways);
                        //if evict way modified bit is 1 the we need to update in L2 too.call fucntion to directly replace the page in L2
                        vector<string> temp = cache_obj.convertL1_toL2(offsets[1], offsets[0]);
                        //can call function as we took note of tag resided in it
                        add_to_cache(cache_obj.cache_L1, cache_obj.valid_L1, index_L1, evict_way, tag_L1);

                        if(cache_obj.modified_L1[evict_way][index_L1]==1){
                            unsigned long temp_tag = strtol(temp[0].c_str(), &ptr, 2);
                            unsigned long temp_index = strtol(temp[1].c_str(), &ptr, 2);
                            free_way = iswayavailable(cache_obj.valid_L2, temp_index, L2_ways);
                            if(free_way==0){
                                evict_way = lru_evict(cache_obj.lru_L2, temp_index, L2_ways);
                                //if evict_way modified bit is 1 then it will updated in RAM
                                add_to_cache(cache_obj.cache_L2, cache_obj.valid_L2, temp_index, evict_way, temp_tag);
                            }
                            else{
                                for(unsigned long i=0; i<free_way-1; i++){
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
                        for(unsigned long i=0; i<free_way-1; i++){
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
                    unsigned long free_way = iswayavailable(cache_obj.valid_L2, index_L2, L2_ways);
                    unsigned long evict_way;

                    //update in L2.
                    if(free_way==0){
                        evict_way = lru_evict(cache_obj.lru_L2, index_L2, L2_ways);
                        //if evict_way modified bit is 1 then it will updated in RAM
                        add_to_cache(cache_obj.cache_L2, cache_obj.valid_L2, index_L2, evict_way, tag_L2);
                    }
                    else{
                        for(unsigned long i=0; i<free_way-1; i++){
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
                            unsigned long temp_tag = strtol(temp[0].c_str(), &ptr, 2);
                            unsigned long temp_index = strtol(temp[1].c_str(), &ptr, 2);
                            free_way = iswayavailable(cache_obj.valid_L2, temp_index, L2_ways);
                            if(free_way==0){
                                evict_way = lru_evict(cache_obj.lru_L2, temp_index, L2_ways);
                                //if evict_way modified bit is 1 then it will updated in RAM
                                add_to_cache(cache_obj.cache_L2, cache_obj.valid_L2, temp_index, evict_way, temp_tag);
                            }
                            else{
                                for(unsigned long i=0; i<free_way-1; i++){
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
                        for(unsigned long i=0; i<free_way-1; i++){
                            cache_obj.lru_L1[index_L1][i]++;
                        }
                        cache_obj.lru_L1[index_L1][free_way-1] == 1;
                        add_to_cache(cache_obj.cache_L1, cache_obj.valid_L1, index_L1, free_way-1, tag_L1);
                        cache_obj.modified_L1[evict_way][index_L1] = 1;
                    }

                }//update only L1
                else if(in_L2 == 1 && in_L1 == 0){
                    
                    unsigned long free_way = iswayavailable(cache_obj.valid_L1, index_L1, L1_ways);
                    unsigned long evict_way;

                    if(free_way==0){
                        evict_way = lru_evict(cache_obj.lru_L1, index_L1, L1_ways);
                        //if evict way modified bit is 1 the we need to update in L2 too.call fucntion to directly replace the page in L2
                        vector<string> temp = cache_obj.convertL1_toL2(offsets[1], offsets[0]);
                        //can call function as we took note of tag resided in it
                        add_to_cache(cache_obj.cache_L1, cache_obj.valid_L1, index_L1, evict_way, tag_L1);

                        if(cache_obj.modified_L1[evict_way][index_L1]==1){
                            unsigned long temp_tag = strtol(temp[0].c_str(), &ptr, 2);
                            unsigned long temp_index = strtol(temp[1].c_str(), &ptr, 2);
                            free_way = iswayavailable(cache_obj.valid_L2, temp_index, L2_ways);
                            if(free_way==0){
                                evict_way = lru_evict(cache_obj.lru_L2, temp_index, L2_ways);
                                //if evict_way modified bit is 1 then it will updated in RAM
                                add_to_cache(cache_obj.cache_L2, cache_obj.valid_L2, temp_index, evict_way, temp_tag);
                            }
                            else{
                                for(unsigned long i=0; i<free_way-1; i++){
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
                        for(unsigned long i=0; i<free_way-1; i++){
                            cache_obj.lru_L1[index_L1][i]++;
                        }
                        cache_obj.lru_L1[index_L1][free_way-1] == 1;
                        add_to_cache(cache_obj.cache_L1, cache_obj.valid_L1, index_L1, free_way-1, tag_L1);
                        cache_obj.modified_L1[evict_way][index_L1] = 1;
                    }
                }
                else{
                    for(unsigned long i =0; i<L1_ways; i++){
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




