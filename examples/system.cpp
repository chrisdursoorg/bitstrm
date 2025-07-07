// system.cpp
//

#include <cstdlib>
#include <iostream>

int
main(int /*argc*/, char** /*argv*/ ){

  std::cout << std::flush;
  return system("lscpu");
}
