#include "ssfs.h"
#include "../libc/mem.h"
#include "../drivers/screen.h"
#include "../libc/string.h"
#include <stdint.h>

file_struct* get_last_file();

file_struct* root_file;

void init_ssfs(){
  kmalloc(sizeof(file_struct), 1, &root_file);
  root_file->filename = "ROOT";
}

file_struct* create_file(char* filename){
  file_struct* last_file = get_last_file();
  kmalloc(sizeof(file_struct), 1, &(last_file->next_file));
  last_file->next_file->filename=filename;
}

int list_files(){
  int n=1;
  file_struct* current = root_file;
  kprint("Files on the system : \n");
  while(current->next_file != 0){
    current = current->next_file;
    kprint("    ");
    kprint(current->filename);
    kprint("\n");
    n++;
  }
  return n;
}

/*Private Methods*/

file_struct* get_last_file(){
  file_struct* current = root_file;
  while(current->next_file != 0){
    current = current->next_file;
  }
  return current;
}
