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
  kmalloc(sizeof(char)*(strlen(filename)), 1, &(last_file->next_file->filename));
  strcopy(last_file->next_file->filename, filename);
  return last_file->next_file;
}

int list_files(){
  int n=0;
  file_struct* current = root_file;
  kprint("Files on the system : \n");
  while(current->next_file != 0){
    current = current->next_file;
    kprint("    ");
    kprint(current->filename);
    kprint("\n");
    n++;
  }
  char* n_as_str;
  int_to_ascii(n, n_as_str);
  kprint(n_as_str);
  kprint(" files.");
  return n;
}

file_struct* open_file(char* filename){
  file_struct* current = root_file;
  while(current->next_file != 0){
    current=current->next_file;
    if(strcmp(current->filename, filename) == 0){
      return current;
    }
  }
  return create_file(filename);
}

void write_file(file_struct* file, char* datas){
  kmalloc(sizeof(char)*strlen(datas), 1, &(file->data_start));
  strcopy(file->data_start, datas);
}

void read_file(file_struct* file, char* datas){
  str_clear(datas, strlen(datas));
  kmalloc(sizeof(char)*strlen(file->data_start), 1, datas);
  strcopy(datas, file->data_start);
}

/*Private Methods*/

file_struct* get_last_file(){
  file_struct* current = root_file;
  while(current->next_file != 0){
    current = current->next_file;
  }
  return current;
}
