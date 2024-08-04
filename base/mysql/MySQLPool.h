#ifndef __MYSQL_POOL_H__
#define __MYSQL_POOLHH

#include <mysql/mysql.h>
#include <semaphore.h>
#include <list>
#include <mutex>
#include <string>

// c++11没有信号量，封装一个
class sem {
public:
    sem() {
        if (sem_init(&m_sem, 0, 0) != 0) {
            throw std::exception();
        }
    }
    sem(int num) {
        if (sem_init(&m_sem, 0, num) != 0) {
            throw std::exception();
        }
    }
    ~sem() {
        sem_destroy(&m_sem);
    }
    bool wait() {
        return sem_wait(&m_sem) == 0;
    }
    bool post() {
        return sem_post(&m_sem) == 0;
    }

private:
    sem_t m_sem;
};

class MySQLPool {
public:
    // 单例模式
    static MySQLPool* getInstance();

    // 初始化
    void init(std::string szUrl, std::string szUser, std::string szPassword, std::string szDBName, int iPort, int iMaxConn);

    // 获取数据库连接
    MYSQL* getConnection();

    // 释放连接
    bool releaseConnection(MYSQL* conn);

    // 获取空闲连接数
    int getFreeConn();

    // 销毁所有连接
    void destroyPool();

private:
    // 不给构造
    MySQLPool();
    // 防止外部调用delete
    ~MySQLPool();

    int m_iMaxConn;  // 最大连接数
    int m_iCurConn;  // 已使用连接数
    int m_iFreeConn; // 当前空闲连接数
    std::mutex m_mutex;

    std::list<MYSQL*> m_connList; // 连接池
    sem m_sem;
};

// 定义一个类使用连接池
class MySQLConn {
public:
    MySQLConn(MYSQL** con, MySQLPool* pMySQLPool);
    ~MySQLConn();

private:
    MYSQL* m_pMySQLConn;
    MySQLPool* m_pMySQLPool;
};

#endif
