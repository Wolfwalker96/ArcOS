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

#endif
