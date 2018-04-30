#include "../cpu/isr.h"
#include "../drivers/screen.h"
#include "kernel.h"
#include "../libc/string.h"
#include "../libc/mem.h"
#include "password.h"
#include "ssfs.h"
#include <stdint.h>

int USER_LOGGED = 0;

void kernel_main() {
    isr_install();
    irq_install();

    asm("int $2");
    asm("int $3");
    init_ssfs();

    create_file("PASSWORD");
    file_struct* password_file = open_file("PASSWORD");
    write_file(password_file, "123456789");

    clear_screen();
    kprint("Wellcome to ArcOS ! \nPlease enter the password : \n> ");
}

void user_login(char* password){
  if(password_check(password)){
    USER_LOGGED=1;
    kprint("Type something, it will go through the kernel\n"
        "Type END to halt the CPU or PAGE to request a kmalloc()\n> ");
  }
  else{
    kprint("Wrong password\n> ");
  }
}

void user_shell(char* input){
  char* command[512];
  str_clear(command, 512);
  char* arg[512];
  str_clear(arg, 512);
  split(input, command, arg);
  if (strcmp(command, "END") == 0) {
      kprint("Stopping the CPU. Bye!\n");
      asm volatile("hlt");
  } else if (strcmp(command, "PAGE") == 0) {
      /* Lesson 22: Code to test kmalloc, the rest is unchanged */
      uint32_t phys_addr;
      uint32_t page = kmalloc(1000, 1, &phys_addr);
      char page_str[16] = "";
      hex_to_ascii(page, page_str);
      char phys_str[16] = "";
      hex_to_ascii(phys_addr, phys_str);
      kprint("Page: ");
      kprint(page_str);
      kprint(", physical address: ");
      kprint(phys_str);
      kprint("\n");
  } else if (strcmp(command, "LIST") == 0) {
    list_files();
  }else if(strcmp(command, "CREATE") == 0){
    create_file(arg);
    kprint(arg);
    kprint(" was created.");
  } else if(strcmp(command, "READ") == 0){
    file_struct* file = open_file(arg);
    char* datas;
    read_file(file, datas);
    kprint(datas);
  } else if(strcmp(command, "WRITE") == 0){
    char* filename[512];
    str_clear(filename, 512);
    char* datas[512];
    str_clear(datas, 512);
    split(arg, filename, datas);
    file_struct* file = open_file(filename);
    write_file(file, datas);
  } else if(strcmp(command, "LOCK") == 0){
    USER_LOGGED=0;
  }
  else{
    kprint(command);
    kprint(" is not a command !");
  }
  kprint("\n> ");
}

void user_input(char *input) {
  if(USER_LOGGED) user_shell(input);
  else user_login(input);
}
