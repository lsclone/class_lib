/***************************
** 参考：https://www.cnblogs.com/TianFang/archive/2013/02/05/2892503.html
**      https://www.cnblogs.com/sssblog/p/11023777.html
** 官网：https://www.boost.org/doc/libs/1_61_0/doc/html/circular_buffer.html
****************************/

#include <boost/circular_buffer.hpp>
#include <iostream>

int main() {
  boost::circular_buffer<int> cb(3);

  std::cout << cb.capacity() << std::endl;
  std::cout << cb.size() << std::endl;

  cb.push_back(0);
  cb.push_back(1);
  cb.push_back(2);
  std::cout << cb.size() << std::endl;

  cb.push_back(3);
  cb.push_back(4);
  cb.push_back(5);
  std::cout << cb.size() << std::endl;

  for (int i : cb) {
    std::cout << i << std::endl;
  }

  return 0;
}
