/**************************
**  参考：https://blog.csdn.net/yangguanghaozi/article/details/51590215
**        https://www.iteye.com/blog/aigo-2292024
**        https://blog.csdn.net/weixin_30412013/article/details/98697420
**  代码参考：https://www.cnblogs.com/sssblog/p/11330990.html
**  官网参考：https://www.boost.org/doc/libs/1_66_0/doc/html/lockfree.html
***************************/
#include <boost/lockfree/spsc_queue.hpp>
#include <thread>
#include <iostream>

boost::lockfree::spsc_queue<int> q(100);
int sum = 0;

void produce()
{
  for (int i = 1; i <= 100; ++i)
    q.push(i);
}

void consume()
{
  int i;
  while (q.pop(i))
    sum += i;
}

int main()
{
  std::thread t1(produce);
  std::thread t2(consume);
  t1.join();
  t2.join();
  consume();
  std::cout << sum << std::endl;
  return 0;
}
