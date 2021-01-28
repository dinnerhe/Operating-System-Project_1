//
//  main.cpp
//  HW1
//
//  Created by Tony on 2021/1/9.
//

#include <iostream>
#include <string>
#include <vector>
#include <string.h>
#include <stdio.h>

#define BLOCK_SIZE 512
using namespace std;

struct OF_Struct{
    unsigned char buf[512];
    int current_pos, file_sz, descriptor_index;
};
struct file_descriptor{
    int file_length;
    int block1;
    int block2;
    int block3;
};

struct dir_entry{
    char name[4];
    int fd_index;
};

unsigned char I[512], O[512], M[512];
unsigned char D[64][512];
OF_Struct OFT[4];


pair<int, int> fd_number_to_acutal_pos(int n){
    int disk = n/32 +1;
    int spec_pos = n%32 *16;
    return make_pair(disk, spec_pos);
}

int buffToInteger(char * buffer)
{
    int a = int((unsigned char)(buffer[0]) << 24 |
                (unsigned char)(buffer[1]) << 16 |
                (unsigned char)(buffer[2]) << 8 |
                (unsigned char)(buffer[3]));
    return a;
}

vector<string> tokenize(char strc[80])
{
    //char * strc = new char[strlen(str.c_str())+1];
    //strcpy(strc, str.c_str());   //string转换成C-string
    vector<string> res;
    char* temp = strtok(strc, " ");
    while(temp != NULL)
    {
        res.push_back(string(temp));
        temp = strtok(NULL, " ");
    }
    //delete[] strc;
    return res;
}



int seek(int index, int position){
    pair<int, int> fd_pos = fd_number_to_acutal_pos(OFT[index].descriptor_index);
    file_descriptor buffer;
    memcpy(&buffer, &D[fd_pos.first][fd_pos.second], sizeof(file_descriptor));
    OFT[index].file_sz = buffer.file_length;
    //printf("Set Open File Index %d to position%d\n", index, position);
    if(position > buffer.file_length){
        //perror("current position is past the end of file in seek");
        cout << "error" << endl;
        return 0;
    }
    /* check if dest pos in the buffer. if not, save the current buffer to related block and read new block to buffer*/
    //cout << "Target position: " << position << " Current position: " << OFT[index].current_pos << endl;
    if(position/512 != OFT[index].current_pos /512){
        //cout << "Current block: " <<OFT[index].current_pos/512 << endl;
        //crcout << "Switching to block: " <<position/512 << endl;
        switch (OFT[index].current_pos/512) {
            case 0:
                memcpy(&D[buffer.block1], &OFT[index].buf, sizeof(D[buffer.block1]));
                //cout << "Saving Buffer to block 1" << endl;
                break;
            case 1:
                memcpy(&D[buffer.block2], &OFT[index].buf, sizeof(D[buffer.block2]));
                //cout << "Saving Buffer to block 2" << endl;
                break;
            case 2:
            case 3:
                memcpy(&D[buffer.block3], &OFT[index].buf, sizeof(D[buffer.block3]));
                //cout << "Saving Buffer to block 3" << endl;
                break;
            default:
                break;
        }
        switch (position/512) {
            case 0:
                memcpy(&OFT[index].buf, &D[buffer.block1], sizeof(D[buffer.block2]));
                //cout << "Cpoying Buffer from block 1" << endl;
                break;
            case 1:
                memcpy(&OFT[index].buf, &D[buffer.block2], sizeof(D[buffer.block2]));
                //cout << "Copying Buffer from block 2" << endl;
                break;
            case 2:
                memcpy(&OFT[index].buf, &D[buffer.block3], sizeof(D[buffer.block2]));
                //cout << "Cpoying Buffer from block 3" << endl;
                break;
            default:
                break;
        }
    }
    //set OFT current_pos to the position
    OFT[index].current_pos = position;
    return 1;
}

int read(int OF_index, int mem_index, int size){
    pair<int, int> fd_pos = fd_number_to_acutal_pos(OFT[OF_index].descriptor_index);
    file_descriptor buffer;
    memcpy(&buffer, &D[fd_pos.first][fd_pos.second], sizeof(file_descriptor)); // copy the fd into buffer
    int cur_pos = OFT[OF_index].current_pos;
    int base_pos = cur_pos;
    int current_block;
    if(cur_pos <512){ current_block = 0;}
    else{
        if (cur_pos < 1024){
            current_block = 1;
        }else{
            current_block = 2;
        }
    }
    for(int i =0; i< size; i++){
        memcpy(&I, &OFT[OF_index].buf, sizeof(I));
        memcpy(&M[mem_index+i], &OFT[OF_index].buf[base_pos- current_block*512+i], sizeof(unsigned char));
        cur_pos ++;
        OFT[OF_index].current_pos++;
        if(cur_pos >= (current_block+1) *512){
            //cout << "Reading - Changing Blcok" <<endl;
            //cout << "Current block: " <<(OFT[OF_index].current_pos-1)/512 << endl;
            //cout << "Switching to block: " << current_block +1 << endl;
            switch (current_block) {
                case 0:
                    memcpy(&D[buffer.block1], &OFT[OF_index].buf, sizeof(D[buffer.block1]));
                    memcpy(&OFT[OF_index].buf, &D[buffer.block2], sizeof(D[buffer.block2]));
                    break;
                case 1:
                    memcpy(&D[buffer.block2], &OFT[OF_index].buf, sizeof(D[buffer.block2]));
                    memcpy(&OFT[OF_index].buf, &D[buffer.block3], sizeof(D[buffer.block3]));
                    break;
                default:
                    break;
            }
            current_block ++;
        }
    }
    return 1;
}

int write(int OF_index, int mem_index, int size){
    pair<int, int> fd_pos = fd_number_to_acutal_pos(OFT[OF_index].descriptor_index);
    file_descriptor buffer;
    memcpy(&buffer, &D[fd_pos.first][fd_pos.second], sizeof(file_descriptor)); // copy the fd into buffer
    int cur_pos = OFT[OF_index].current_pos;
    int base_pos = cur_pos;
    int current_block;
    if(cur_pos <512){ current_block = 0;}
    else{
        if (cur_pos < 1024){
            current_block = 1;
        }else{
            current_block = 2;
        }
    }
    for(int i =0; i< size; i++){
        memcpy(&OFT[OF_index].buf[base_pos- current_block*512 + i], &M[mem_index+i], sizeof(unsigned char));
        memcpy(&I[base_pos- current_block*512 + i], &OFT[OF_index].buf[base_pos- current_block*512 + i], sizeof(unsigned char));
        memcpy(&I, &OFT[OF_index].buf, sizeof(M));
        cur_pos ++;
        OFT[OF_index].current_pos++;
        if(cur_pos >= (current_block+1) *512 && OFT[OF_index].current_pos <1536){
            //cout << "Writing - Changing Block" <<endl;
            //cout << "Current block: " <<(OFT[OF_index].current_pos-1)/512 << endl;
            //cout << "Switching to block: " << current_block +1 << endl;
            switch (current_block) {
                case 0:
                    if(buffer.block2 == -1){
                        for(int i =8; i < 64; i++){
                            if (D[0][i] == '0'){
                                D[0][i] = '1';
                                buffer.block2 = i;
                                break;
                            }
                        }
                    }
                    /*//for testing
                    for(int i = 0; i<10; i++){
                        OFT[OF_index].buf[i] = 'C';
                    }
                    memcpy(&D[8], &OFT[OF_index].buf[i], sizeof(D[8]));
                    for(int i = 0; i<10; i++){
                        cout << D[8][i] << endl;
                    }
                    //test end*/
                    memcpy(&I, &OFT[OF_index].buf, sizeof(buffer));
                    memcpy(&D[30], &O, 100);
                    dir_entry fentry;
                    memcpy(&fentry, &OFT[OF_index].buf, sizeof(fentry));
                    //cout << "Directory ZERO name: " << fentry.name <<" FD index: " << fentry.fd_index <<endl;
                    memcpy(&D[buffer.block1], &OFT[OF_index].buf, sizeof(D[buffer.block1]));
                    memcpy(&OFT[OF_index].buf, &D[buffer.block2], sizeof(D[buffer.block2]));
                    break;
                case 1:
                    if(buffer.block3 == -1){
                        for(int i =8; i < 64; i++){
                            if (D[0][i] == '0'){
                                D[0][i] = '1';
                                buffer.block2 = i;
                                break;
                            }
                        }
                    }
                    memcpy(&D[buffer.block2], &OFT[OF_index].buf, sizeof(D[buffer.block2]));
                    memcpy(&OFT[OF_index].buf, &D[buffer.block3], sizeof(D[buffer.block3]));
                    break;
                default:
                    break;
            }
            current_block ++;
        }
    }
    if(cur_pos > buffer.file_length){ //update fd if file size increases
        buffer.file_length = cur_pos;
    }
    memcpy(&D[fd_pos.first][fd_pos.second], &buffer, sizeof(file_descriptor)); // copy buffer back to fd
    return 1;
}

void create(string filename){
    unsigned char m_temp[512];
    memcpy(&m_temp, &M, sizeof(m_temp)); // copy m to temporary buffer
    file_descriptor fd;
    char name[4];
    strcpy(name, filename.c_str());
    //traverse the directory to find duplicate file name
    seek(0, 0);
    while(OFT[0].current_pos < OFT[0].file_sz){
        read(0, 0, 8);
        dir_entry entry;
        memcpy(&entry, &M[0], sizeof(entry));
        if(strcmp(entry.name, name) == 0){
            perror("duplicate file name!");
        }
    }
    //find an empty file descriptor and allocate it
    bool allocated_fd = false;
    int fd_index = -1;
    for(int i = 1; i< 7; i++){
        for(int j =0; j< 128; j+=16){
            memcpy(&fd, &D[i][j], sizeof(fd));
            if(fd.file_length == -1 && !allocated_fd){ // if found an empty fd, allocate it
                fd.file_length = 0;
                fd.block1 = -1;
                fd.block2 = -1;
                fd.block3 = -1;
                memcpy(&D[i][j], &fd, sizeof(fd));
                fd_index = ((i-1)*512 +j)/16;
                allocated_fd = true;
            }
        }
    }
    //find an empty directory entry and allocate it
    seek(0, 0);
    bool allocated_dir_entry = false;
    while(OFT[0].current_pos < 1536){
        read(0, 0, 8);
        seek(0, OFT[0].current_pos-8);
        dir_entry entry;
        memcpy(&entry, &M[0], sizeof(entry));
        if(strcmp(entry.name, "0") == 0 && !allocated_dir_entry){
            strcpy(entry.name, name);
            entry.fd_index = fd_index;
            memcpy(&M[0], &entry, sizeof(entry));
            write(0, 0, 8);
            allocated_dir_entry = true;
            break;
        }
        read(0, 0, 8);
    }
    if(OFT[0].current_pos >= 1536 && !allocated_dir_entry){
        perror("no free directory entry found");
    }
    //update directory file descriptor
    //memcpy(&fd, &D[1][0], sizeof(fd));
    //OFT[0].file_sz +=8;
    //fd.file_length +=8;
    //memcpy(&D[1][0], &fd, sizeof(fd));
    cout<< filename << " created"   << endl;
    memcpy(&M,&m_temp, sizeof(m_temp)); // restore M
}

void destory(string filename){
    unsigned char m_temp[512];
    memcpy(&m_temp, &M, sizeof(m_temp)); // copy m to temporary buffer
    file_descriptor fd;
    //cout << "Creating file: " << filename << endl;
    char name[4];
    strcpy(name, filename.c_str());
    //traverse the directory to find the file name
    seek(0, 0);
    //printf("Dir Current pos: %d\n",OFT[0].current_pos );
    //printf("Dir Current file sz: %d\n",OFT[0].file_sz);
    
    while(OFT[0].current_pos < OFT[0].file_sz){
        read(0, 0, 8);
        seek(0, OFT[0].current_pos-8);
        dir_entry entry;
        memcpy(&entry, &M[0], sizeof(entry));
        if(strcmp(entry.name, name) == 0){
            //printf("Found the file %s\n", name);
            pair<int, int> actual_index = fd_number_to_acutal_pos(entry.fd_index);
            memcpy(&fd, &D[actual_index.first][actual_index.second], sizeof(fd));
            fd.file_length = -1;
            if(fd.block1!=-1){ D[0][fd.block1] = '0';}
            if(fd.block2!=-1){ D[0][fd.block3] = '0';}
            if(fd.block3!=-1){ D[0][fd.block3] = '0';}
            strcpy(entry.name,"0");
            memcpy(&D[actual_index.first][actual_index.second], &fd, sizeof(fd));
            memcpy(&M[0], &entry,sizeof(entry));
            write(0, 0, sizeof(entry));
            cout << filename << " destroyed" << endl;
            memcpy(&M,&m_temp, sizeof(m_temp)); // restore M
            return;
        }
        read(0, 0, 8);
    }perror("file does not exist");
    memcpy(&M,&m_temp, sizeof(m_temp)); // restore M
};

void open(string filename){
    unsigned char m_temp[512];
    memcpy(&m_temp, &M, sizeof(m_temp)); // copy m to temporary buffer
    file_descriptor fd;
    
    char name[4];
    strcpy(name, filename.c_str());
    //traverse the directory to find duplicate file name
    seek(0, 0);
    bool gotEmptyOFT = false;
    while(OFT[0].current_pos < OFT[0].file_sz){
        read(0, 0, 8);
        dir_entry entry;
        memcpy(&entry, &M[0], sizeof(entry));
        if(strcmp(entry.name, name) == 0){
            pair<int, int> actual_index = fd_number_to_acutal_pos(entry.fd_index);
            memcpy(&fd, &D[actual_index.first][actual_index.second], sizeof(fd)); // find the file descriptor
            for(int j =1; j <4; j++){
                if(OFT[j].current_pos ==-1 && !gotEmptyOFT){
                    gotEmptyOFT = true;
                    OFT[j].descriptor_index = entry.fd_index;
                    OFT[j].file_sz = fd.file_length;
                    OFT[j].current_pos = 0;
                    if(fd.file_length ==0){ //check if is empty file
                        for(int i =8; i < 64; i++){
                            if (D[0][i] == '0'){
                                D[0][i] = '1';
                                fd.block1 = i;
                                break;
                            }
                        }
                    }
                    memcpy(&OFT[j].buf, &D[fd.block1], sizeof(OFT[j].buf)); // copy the first block of file to buffer
                    memcpy(&D[actual_index.first][actual_index.second], &fd, sizeof(fd)); // update fd in case of empty file
                    cout << filename << " opened "<< j  << endl;
                    memcpy(&M,&m_temp, sizeof(m_temp)); // restore M
                    return;
                }
            }perror("too many files open");
            memcpy(&M,&m_temp, sizeof(m_temp)); // restore M
        }
    }perror("file does not exist");
    memcpy(&M,&m_temp, sizeof(m_temp)); // restore M
};

void close(int oft_index){
    unsigned char m_temp[512];
    memcpy(&m_temp, &M, sizeof(m_temp)); // copy m to temporary buffer
    file_descriptor fd;
    pair<int, int> fd_pos = fd_number_to_acutal_pos(OFT[oft_index].descriptor_index);
    memcpy(&fd, &D[fd_pos.first][fd_pos.second], sizeof(fd));
    if (OFT[oft_index].current_pos < 512){
        memcpy(&D[fd.block1], &OFT[oft_index].buf, sizeof(D[fd.block1]));
    }else if (OFT[oft_index].current_pos < 1024){
        memcpy(&D[fd.block2], &OFT[oft_index].buf, sizeof(D[fd.block2]));
    }else{
        memcpy(&D[fd.block3], &OFT[oft_index].buf, sizeof(D[fd.block3]));
    }
    fd.file_length = OFT[oft_index].file_sz;
    OFT[oft_index].current_pos = -1;
    cout << oft_index << " closed" << endl;
    memcpy(&M,&m_temp, sizeof(m_temp)); // restore M
};

void display_directory(){
    unsigned char m_temp[512];
    memcpy(&m_temp, &M, sizeof(m_temp)); // copy m to temporary buffer
    dir_entry entry;
    file_descriptor fd;
    seek(0, 0);
    for(int i=0; i <1536; i+=8){
        read(0, 0, 8);
        memcpy(&I, &OFT[0].buf, sizeof(I));
        memcpy(&entry, &M[0], sizeof(entry));
        if(entry.fd_index != -1 && strcmp(entry.name, "0") !=0){
            pair<int, int> fd_pos = fd_number_to_acutal_pos(entry.fd_index);
            memcpy(&fd, &D[fd_pos.first][fd_pos.second], sizeof(fd));
            printf("%s %d ", entry.name, fd.file_length);
        }
    }
    printf("\n");
    memcpy(&M,&m_temp, sizeof(m_temp)); // restore M
}

void init(){
    //initialize bitmap at block 0(bitmap);
    for(int i = 0; i< 64; i++){D[0][i] = '0';}
    //initialize I, O, M buffer
    for(int i = 0; i< 512; i++){
        I[i] = '\0';
        O[i] = '\0';
        M[i] = '\0';
    }
    //initialize first file descriptor: directory
    file_descriptor fd;
    fd.file_length =0;
    fd.block1 = 7;
    fd.block2 = 8;
    fd.block3 = 9;
    for(int i = 0; i< 10; i++){D[0][i] = '1';}
    memcpy(&D[1][0], &fd, sizeof(fd));
    //initialize other file descriptors
    fd.file_length =-1;
    fd.block1 = -1;
    fd.block2 = -1;
    fd.block3 = -1;
    for(int i =1; i < 192; i++){
        int blockn = fd_number_to_acutal_pos(i).first;
        int byten = fd_number_to_acutal_pos(i).second;
        memcpy(&D[blockn][byten], &fd, sizeof(fd));
    }
    //initialize OFT 0 for driectory, other 3 for free
    OFT[0].current_pos = 0;
    OFT[0].file_sz = 0;
    OFT[0]. descriptor_index = 0;
    memcpy(&OFT[0].buf, &D[7], sizeof(D[7]));
    OFT[1].current_pos = -1;
    OFT[2].current_pos = -1;
    OFT[3].current_pos = -1;
    //fill free dir entry
    for(int i =0; i< 512; i++) {I[i] = 'A';}
    seek(0, 0);
    dir_entry di;
    strcpy(di.name, "0");
    di.fd_index = -1;
    memcpy(&M[0], &di, 8);
    int init_size = 0;
    while(init_size < 1536){
        write(0, 0, 8);
        init_size +=8;
    }
    cout << "system initialized" << endl;
};

void memory_write(int pos, string input){
    for(int i = 0; i< input.length() ; i++){
        M[pos] = input[i];
        pos++;
    }
    cout << input.length() << " bytes written to M" << endl;
};
void memory_read(int pos, int count){
    for(int i=0; i< count; i++){
        if(M[pos+i] != '\0')
        cout << M[pos+i];
    }
    cout << endl;
    //cout << count << " bytes read from " << pos <<endl;
};
/*
 Disk Structure:
    block 0: bitmap  0: free 1:occupied
    block 1-6: file descriptor
        b1 fd 1-32
        b2 fd 33-64
        b3 fd 65-96
        b4 fd 97-128
        b5 fd 129-160
        b6 fd 130-192
            16 byte/4 integers for each fd
    block 7+ file store location
    block 7, 8, 9 is for directory entry
 */
int main() {
    // insert code here...
    std::cout << "Hello, World!\n";
    
    //initialize
    //init();
    //get input
    char input[80];
    string x;
    vector<string> args;
    while(1){
        
        //cout << "> ";
        //cin >> input;
        //fgets(input, (sizeof input / sizeof input[0]), stdin);
        //cin.ignore(numeric_limits<streamsize>::max(),'\n');
        cin.getline(input, 80);
        if(strcmp(input, "\n") == 0 ){
            cout << endl;
            continue;}
        if(strcmp(input, "") == 0 ){
            //cout << endl;
            continue;}
        if(input[strlen(input)-1] == '\n') input[strlen(input)-1]=0; // clear \n
        args = tokenize(input);
        for(int i =0; i< args.size(); i++){
            //cout<< "argument " << i << ": "<< args[i] << endl;
        }
        if(args[0] == "cr" || args[0] == "cr\n"){ //create file
            create(args[1]);
        }
        if(args[0] == "de" || args[0] == "de\n"){
            destory(args[1]);
        }
        if(args[0] == "op"){
            open(args[1]);
        }
        if(args[0] == "cl"){
            close(stoi(args[1]));
        }
        if(args[0] == "wr"){
            if(write(stoi(args[1]), stoi(args[2]), stoi(args[3])))
            cout << stoi(args[3]) << " bytes written to "<< stoi(args[1]) << endl;
        }
        if(args[0] == "rd"){
            if(read(stoi(args[1]), stoi(args[2]), stoi(args[3]))){
                cout << stoi(args[3]) << " bytes read from "<< stoi(args[1]) << endl;
            }
        }
        if(args[0] == "sk"){
            if(seek(stoi(args[1]), stoi(args[2]))) cout << "position is " << stoi(args[2]) << endl;
        }
        if(args[0] == "dr"){
            display_directory();
        }
        if(args[0] == "in"){
            init();
        }
        if(args[0] == "rm"){
            memory_read(stoi(args[1]), stoi(args[2]));
        }
        if(args[0] == "wm"){
            memory_write(stoi(args[1]), args[2]);
        }
        if(args[0] == "quit") {break;}
    }
    return 0;
}


