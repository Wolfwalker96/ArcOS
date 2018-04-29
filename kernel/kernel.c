#include "../cpu/isr.h"
#include "../drivers/screen.h"
#include "kernel.h"
#include "../libc/string.h"
#include "../libc/mem.h"
#include "password.h"
#include <stdint.h>

int USER_LOGGED = 0;

void kernel_main() {
    isr_install();
    irq_install();

    asm("int $2");
    asm("int $3");

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
  if (strcmp(input, "END") == 0) {
      kprint("Stopping the CPU. Bye!\n");
      asm volatile("hlt");
  } else if (strcmp(input, "PAGE") == 0) {
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
  }
  kprint("You said: ");
  kprint(input);
  kprint("\n> ");
}

void user_input(char *input) {
  if(USER_LOGGED) user_shell(input);
  else user_login(input);
}
