#include <iostream>

#include "MySQLPool.h"

using namespace std;

int main(int argc, char const* argv[]) {
    MySQLPool* sqlPool = MySQLPool::getInstance();
    if (sqlPool == nullptr) {
        cerr << "sqlPool is nullptr" << endl;
        exit(1);
    }
    sqlPool->init("127.0.0.1", "test", "test", "db_test", 3306, 3);

    // 3
    cout << "sqlPool free conn count is " << sqlPool->getFreeConn() << endl;

    MYSQL* mysql;
    {
        // 2 析构时会释放连接以供以后使用
        MySQLConn conn(&mysql, sqlPool);
        cout << "sqlPool free conn count is " << sqlPool->getFreeConn() << endl;

        if (mysql == nullptr) {
            cerr << "mysql is nullptr" << endl;
            exit(1);
        }

        if (mysql_query(mysql, "select username, password from users")) {
            cerr << "mysql_query error" << endl;
            exit(1);
        }

        auto result = mysql_store_result(mysql);
        if (result != nullptr) {
            auto num_fields = mysql_num_fields(result);
            auto fields = mysql_fetch_fields(result);

            while (auto row = mysql_fetch_row(result)) {
                cout << "username: " << row[0] << "   password : " << row[1] << endl;
            }
        }
    }

    // 3
    cout << "sqlPool free conn count is " << sqlPool->getFreeConn() << endl;

    return 0;
}
