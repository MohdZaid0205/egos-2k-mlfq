#include "app.h"
#include "string.h"

int main(int argc, char** argv) {
    
    // expect input as wcl [FILE1] [FILE2] ...
    if (argc < 2) {
        INFO("usage: wcl [FILE1] [FILE2] ...");
        return -1;
    }
    
    uint found_nline = 0;
    
    for (int f=1; f<argc ; f++) {

        int file_ino = dir_lookup(workdir_ino, argv[f]);
        if (file_ino < 0) {
            INFO("grep: file %s not found", argv[f]);
            return -1;
        }

        // initialise buffer with predefine block size.
        char buf[BLOCK_SIZE];
        uint buffer_block_of;
        buffer_block_of = 00;   
        
        
        while (file_read(file_ino, buffer_block_of, buf) == 0){
        
            int files_EOF = 0; // hit file's EOF charchter

            // for each charachter present in buffer check
            // if it is a new_line or EOF charachter in
            // order to update line buffer or end current
            // programme.
            for (int i=0; i<BLOCK_SIZE; i++){
                if (buf[i] == '\n' || buf[i] == 0){
                    // assuming this represents new line in given code.
                    found_nline ++;
                    if (buf[i] == 0) {
                        // printf("EOF\n"); // chack if EOF invoked valid
                        // if file's EOF has been hit at current position
                        files_EOF = 1;  break;
                    }
                }
            }
        
            buffer_block_of += BLOCK_SIZE;
            if (files_EOF == 1) { break; }
        }
    }
    printf("%d\n", found_nline);
    
    return 0;
}
