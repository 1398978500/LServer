#include <functional>
#include <iostream>

#include "ThreadPool.h"

using namespace std;

void test() {
    cout << "test " << endl;
}

void test2(int a, int b) {
    cout << "test 2 a = " << a << " b =" << b << endl;
}

int main(int argc, char* argv[]) {
    ThreadPool pool(3);

    // 匿名函数添加
    pool.addTask([]() { cout << "task : lambda " << endl; });

    // 直接添加
    pool.addTask(test);

    auto f1 = [](int a) { cout << "lambda a = " << a << "  b = " << endl; };
    pool.addTask(f1, 1);

    pool.addTask(test2, 3, 4);

    return 0;
}
