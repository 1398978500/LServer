#include <functional>
#include <iostream>

#include "ThreadPool.h"

using namespace std;

void test() {
    cout << "test " << endl;
}

int main(int argc, char* argv[]) {
    ThreadPool pool(3);

    // 匿名函数添加
    pool.addTask([]() { cout << "task : lambda " << endl; });

    // 直接添加
    pool.addTask(test);

    return 0;
}
