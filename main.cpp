#include "sxpFS.h"

int main() {
  my_initialize();
  std::string op;
  while(1) {
    std::cout << curPath << '/' << "# ";
    std::cin >> op;
    if(op=="cd") my_cd();
    if(op=="mkdir") my_mkdir();
    if(op=="rmdir") my_rmdir();
    if(op=="ls") my_ls();
    if(op=="create") my_create();
    if(op=="rm") my_rm();
    if(op=="open") my_open();
    if(op=="close") my_close();
    if(op=="write") my_write();
    if(op=="read") my_read();
    if(op=="exit") {
      my_exit();
      break;
    }
  }
  return 0;
}