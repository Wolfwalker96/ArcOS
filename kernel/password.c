#include "password.h"
#include "../libc/hash.h"
#include "../libc/string.h"

const char* DEFAULT_PASSWORD = "1234";

/*Public function*/
int password_check(char* password){
  return strcmp(hash(password),hash(DEFAULT_PASSWORD))==0;
}
