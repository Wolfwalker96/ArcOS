#ifndef SSFS_H
#define SSFS_H

typedef struct file_struct file_struct;
struct file_struct{
  char* filename;
  char* data_start;
  unsigned long file_size;
  file_struct* next_file;
};

/* Public kernel API */
void init_ssfs();
file_struct* create_file(char* filename);
int list_files();

file_struct* open_file(char* filename);
void write_file(file_struct* file, char* datas);
void read_file(file_struct* file, char* datas);

#endif
