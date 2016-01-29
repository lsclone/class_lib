#include "filesystem.h"
#include <vector>
#include <string>

using namespace std;

int main() {

  string dir("./");
  vector<string> list;
  
  for_each(recursive_directory_iterator(dir), recursive_directory_iterator(),
    [&](path file) {
      if (string::npos != file.name().rfind(".txt")) {
        list.push_back(file.name());
      }
  });
  
  return 0;
}
