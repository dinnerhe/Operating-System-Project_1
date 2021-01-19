//
//  main.cpp
//  HW1
//
//  Created by Tony on 2021/1/9.
//

#include <iostream>
using namespace std;
struct OF_Struct{
    int buf[512];
    int current_pos, file_sz, des_index;
};

pair<int, int> fd_number_to_acutal_pos(int n){
    int disk = 192/32;
    int spec_pos = 192%32 *4;
    return make_pair(disk, spec_pos);
}
/*
 Disk Structure:
    block 0: bitmap
    block 1-6: file descriptor
        b1 fd 1-32
        b2 fd 33-64
        b3 fd 65-96
        b4 fd 97-128
        b5 fd 129-160
        b6 fd 130-192
    block 7+ file store location
 */
int main(int argc, const char * argv[]) {
    // insert code here...
    std::cout << "Hello, World!\n";
    int I[512], O[512], M[512],D[64][512];
    OF_Struct OFT[4];
    //initialize bitmap at block 0(bitmap);
    for(int i = 0; i< 64; i++){D[0][i] = 0;}
    //initialize first file descriptor: directory
    D[1][0] = 0;
    D[1][1] = 7;
    //initialize OFT 0 for driectory, other 3 for free
    OFT[0].current_pos = 0;
    OFT[0].file_sz = 0;
    OFT[0]. des_index = 0;
    OFT[1].current_pos = -1;
    OFT[2].current_pos = -1;
    OFT[3].current_pos = -1;
    
    while(1){
        break;
        string command;
        if(command.compare("cr") == 0){
            for(int i = 1; i< 7; i++){
                for(int j =0; j< 128; j+=4){
                    
                }
            }
        }
    }
    return 0;
}


