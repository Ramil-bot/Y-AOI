#include <iostream>
#include <rtc/rtc.hpp>

#include "parse_cl.h"


#include "server.h"
#include "client.h"



int main(int argc, char **argv) try {
  Cmdline params(argc, argv);

  int mode;
  std::cout << "Mode 1 - host, 2 - client: ";
  std::cin >> mode;
  std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
  
  if (mode == 1 ) {
    Server(params);
  } else if (mode == 2) {
    Client();
  }

  return 0;
} catch (const std::exception &e) {
  std::cout << "Error: " << e.what() << std::endl;
  return -1;
}






