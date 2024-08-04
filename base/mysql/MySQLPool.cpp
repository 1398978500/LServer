#include <iostream>

#include "MySQLPool.h"

using namespace std;

MySQLPool::MySQLPool() {
    m_iCurConn = 0;
    m_iFreeConn = 0;
}

MySQLPool::~MySQLPool() {
    destroyPool();
}

// 单例模式
MySQLPool* MySQLPool::getInstance() {
    static MySQLPool connPool;
    return &connPool;
}

// 初始化
void MySQLPool::init(
    std::string szUrl, std::string szUser, std::string szPassword, std::string szDBName, int iPort, int iMaxConn) {
    m_iMaxConn = iMaxConn;
    for (int i = 0; i < m_iMaxConn; i++) {
        MYSQL* conn = NULL;
        conn = mysql_init(conn);
        if (conn == nullptr) {
            // LOG_CRIT << "mysql_init error ";
            cerr << "mysql init error" << endl;
            exit(1);
        }

        conn = mysql_real_connect(conn, szUrl.c_str(), szUser.c_str(), szPassword.c_str(), szDBName.c_str(), iPort, NULL, 0);
        if (conn == nullptr) {
            // LOG_CRIT << "mysql_real_connect error ";
            cerr << "mysql_real_connect error" << endl;
            exit(1);
        }

        m_connList.push_back(conn);
        ++m_iFreeConn;
    }

    m_sem = sem(m_iFreeConn);
}

// 获取数据库连接
MYSQL* MySQLPool::getConnection() {
    MYSQL* conn = nullptr;

    if (m_connList.empty()) {
        return nullptr;
    }

    m_sem.wait();

    // 上锁
    std::unique_lock<std::mutex> lock(this->m_mutex);
    conn = m_connList.front();
    m_connList.pop_front();
    --m_iFreeConn;
    ++m_iCurConn;

    return conn;
}

// 释放连接
bool MySQLPool::releaseConnection(MYSQL* conn) {
    if (conn == nullptr) {
        return false;
    }

    {
        // 上锁
        std::unique_lock<std::mutex> lock(this->m_mutex);
        ++m_iFreeConn;
        --m_iCurConn;
    }

    m_sem.post();

    return true;
}

// 获取连接
int MySQLPool::getFreeConn() {
    return this->m_iFreeConn;
}

// 销毁所有连接
void MySQLPool::destroyPool() {
    // 上锁
    std::unique_lock<std::mutex> lock(this->m_mutex);
    if (!m_connList.empty()) {
        for (auto& conn : m_connList) {
            mysql_close(conn);
        }

        m_iCurConn = 0;
        m_iFreeConn = 0;
        m_connList.clear();
    }
}

MySQLConn::MySQLConn(MYSQL** MySqlConn, MySQLPool* pMySQLPool) : m_pMySQLPool(pMySQLPool) {
    if (m_pMySQLPool != nullptr) {
        // 传出去MYSQL连接
        *MySqlConn = m_pMySQLPool->getConnection();

        m_pMySQLConn = *MySqlConn;
    }
}
MySQLConn::~MySQLConn() {
    if (m_pMySQLPool != nullptr && m_pMySQLConn != nullptr) {
        // 释放连接
        m_pMySQLPool->releaseConnection(m_pMySQLConn);
    }
}
