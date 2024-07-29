#include <iostream>
#include <queue>
#include <pthread.h>
#include <unistd.h>
#include <atomic>

using namespace std;

typedef void (*func)(int);
queue<func> qTask;

int g_iNum = 0;

pthread_mutex_t mutex;
pthread_cond_t cond;

// 是否停止
atomic<bool> g_bStop = false;

void haha(int a) {
    cout << "haha" << a << endl;
}

void* work(void*) {
    while(1) {
        pthread_mutex_lock(&mutex);

        // 工作队列是空的,就等待唤醒
        if(qTask.empty()) {
            pthread_cond_wait(&cond, &mutex);
            // 被唤醒了,发现没任务
            if(qTask.empty()) {
                pthread_mutex_unlock(&mutex);

                // 要停止,就退出循环
                if(g_bStop) {
                    cout << "exit ..." << endl;
                    break;
                }

                continue;
            }
        }

        // 任务队列不是空的,取出一个任务执行
        func f = qTask.front();
        qTask.pop();
        // 实际工作函数需要锁的话要定义工作函数自己的锁
        f(g_iNum++);
        pthread_mutex_unlock(&mutex);
    }
    return NULL;
}

int main(int argc, char* argv[]) {
    cout << "hellp practice " << endl;
    int iRet = 0;

    iRet = pthread_mutex_init(&mutex, NULL);
    if(iRet != 0) {
        perror("pthread_mutex_init: ");
        exit(iRet);
    }
    iRet = pthread_cond_init(&cond, NULL);
    if(iRet != 0) {
        perror("pthread_mutex_init: ");
        exit(iRet);
    }

    const int iThreadNum = 4;
    pthread_t iPthId[iThreadNum] = {0};
    for(int i = 0; i < iThreadNum; i++) {
        if(0 != pthread_create(&iPthId[i], NULL, work, NULL)) {
            perror("pthread_create : ");
            exit(-1);
        }
    }

    char a = 0;
    while(true) {
        cin >> a;
        if(a == 'i') {
            // 插入一个任务
            pthread_mutex_lock(&mutex);
            qTask.push(haha);
            // 工作队列大于线程数量,唤醒所有
            if(qTask.size() >= iThreadNum) {
                pthread_cond_broadcast(&cond);
            } else {
                pthread_cond_signal(&cond);
            }
            pthread_mutex_unlock(&mutex);
        }else {
            g_bStop = true;
            break;
        }
    }

    pthread_mutex_destroy(&mutex);
    // 销毁cond前需要广播,放开所有cond
    pthread_cond_broadcast(&cond);
    pthread_cond_destroy(&cond);

    // 等待线程退出
    for(int i = 0; i < iThreadNum; i++) {
        pthread_join(iPthId[i], NULL);
    }
    return 0;
}

