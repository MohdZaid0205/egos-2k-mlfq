#include "app.h"
#include "stdlib.h"
#include "string.h"

int main(int argc, char** argv) {
    
    // expect input as grep [PATTERN] [FILE]
    // TODO: add support for grep [STRING] [FILE]
    if (argc != 3) {
        INFO("usage: grep [PATTERN] [FILE]");
        return -1;
    }

    int file_ino = dir_lookup(workdir_ino, argv[2]);
    if (file_ino < 0) {
        INFO("grep: file %s not found", argv[2]);
        return -1;
    }

    // initialise buffer with predefine block size.
    char buf[BLOCK_SIZE];
    uint buffer_block_of;
    buffer_block_of = 00;   
    
    // to support synamically size lines for buffer.
    uint  line_buffer_size = 0x01;
    uint  line_current_len = 0x00;
    char* line_buffer = malloc(sizeof(char)*line_buffer_size);

    // reead from given file untill buffer_block_of
    // is out of scope of given file that is being
    // accessed.
    while (file_read(file_ino, buffer_block_of, buf) == 0){
        
        int files_EOF = 0; // hit file's EOF charchter

        // for each charachter present in buffer check
        // if it is a new_line or EOF charachter in
        // order to update line buffer or end current
        // programme.
        for (int i=0; i<BLOCK_SIZE; i++){
            if (line_buffer_size < line_current_len+1){
                line_buffer_size *= 2;
                char* temp_buffer = malloc(sizeof(char)*line_buffer_size);
                memcpy(temp_buffer, line_buffer, line_current_len);
                free(line_buffer); line_buffer = temp_buffer;
            }

            if (buf[i] == '\n' || buf[i] == 0){
                // assuming this represents new line in given code.
                line_buffer[line_current_len] = 00;
                
                // so beacause default behaviour of grep on any platform is
                // to check for matches on line-by-line basis, i have decided
                // to follow the traditional way coupled with boyer-moore
                char* sequence = line_buffer;
                uint  seq_size = line_current_len;

                char* compares = argv[1];
                uint  com_size = strlen(compares);
                
                uint  is_there = 0;
                uint  found_at = 0;

                // printf("find:%s inside:%s\n", compares, sequence);
                
                int last_found = 0;
                for (int s=0; s<seq_size-com_size+1; s++){
                    if (strncmp(compares, sequence+s, com_size)==0){
                        sequence[s] = 0; is_there = 1;
                        printf("%s\033[38;2;255;50;50m%s\033[0m",
                               sequence + last_found, compares);
                        last_found = s + com_size;
                    }
                }

                if (is_there == 1) printf("%s\n", sequence + last_found);

                // clean buffer an prepare way for next elements properly.
                line_buffer_size = 0x01;    // remove everything from buff
                line_current_len = -0x1;    // set start to -1 later ++ it
                free(line_buffer);

                if (buf[i] == 0) {
                    // printf("EOF\n"); // chack if EOF invoked valid
                    // if file's EOF has been hit at current position
                    files_EOF = 1;  break;
                }
                // printf("NLINE\n"); // if not EOF it is NLINE check

                line_buffer = malloc(sizeof(char)*line_buffer_size);
            }
            else {
                //printf("(%c,%d)", buf[i],(char)buf[i]);
                line_buffer[line_current_len] = buf[i];
            }
            line_current_len++; // move to next charachter in line_buff
        }
        
        buffer_block_of += BLOCK_SIZE;
        if (files_EOF == 1) { break; }
    }
    
    
    return 0;
}
