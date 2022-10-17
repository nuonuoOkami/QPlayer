
#ifndef QPLAYER_SAFE_QUEUE_H
#define QPLAYER_SAFE_QUEUE_H


#include <queue>
#include <pthread.h>


using namespace std;

template<typename T>//函数泛型

/**
 *
 * @tparam T
 * 安全队列
 */
class SafeQueue {
private:
    typedef void (*ReleaseListener)(T *); // 函数指针定义 回调 用来释放T里面的内容的;
    typedef void (*DumpListener)(queue<T> &); // 函数指针定义 回调 用来释放T里面的内容的;
private:
    ReleaseListener release_listener;
    DumpListener dump_listener;
    queue<T> queue;//先进先出
    pthread_mutex_t lock;//互斥锁 加锁
    pthread_cond_t signal; //pthread_cond_signal 发送消息唤醒线程 pthread_cond_wait等待
    bool is_play;//是否在播放



public:
    SafeQueue() {
        pthread_mutex_init(&lock, 0);//初始化互斥锁
        pthread_cond_init(&signal, 0);//初始化信号
    }

    //析构函数
    ~SafeQueue() {
        pthread_cond_destroy(&signal);//释放信号
        pthread_mutex_destroy(&lock);//释放锁
    }

public:
    void insert(T t) {
        pthread_mutex_lock(&lock);
        if (is_play) {//如果在工作作态
            queue.push(t);//加入队列
            pthread_cond_signal(&signal);//通知外部
        } else {
            if (release_listener) {
                release_listener(&t);//将资源释放到外面
            }
        }
        pthread_mutex_unlock(&lock);

    }

    /**
     * 取出数据
     * @param t 参数类型
     * @return  返回是否成功
     */
    bool take(T &t) {
        int result = false; // 默认是false
        pthread_mutex_lock(&lock);
        while (is_play && queue.empty())//如果在播放且队列没数据了
        {
            pthread_cond_wait(&signal, &lock);//等待唤醒
        }
        if (!queue.empty()) {
            t = queue.front();//取出第一个元素
            queue.pop();//删除第一个元素
            result = true;//取出数据成功
        }
        pthread_mutex_unlock(&lock);
        return result;
    }

    /**
     *
     * @param listener
     * 设置释放监听器
     */
    void setReleaseListener(ReleaseListener listener) {
        this->dump_listener = listener;
    }


    /**
    *
    * @param listener
    * 设置清空监听器
    */
public:
    void setDumpListener(DumpListener listener) {
        this->dump_listener = listener;
    }

    /**
     * 是否开始播放
     * @param isPlay  是否开始播放
     */
    void setPlayState(bool isPlay) {
        pthread_mutex_lock(&lock);
        this->is_play = isPlay;
        pthread_cond_signal(&signal);//唤醒其他进行工作
        pthread_mutex_unlock(&lock);
    }


/**
     * 是否还有数据
     */
    int empty() {
        return queue.empty();
    }

    /**
     * 数据长度
     */
    int size() {
        return queue.size();
    }

    /**
     * 清理数据
     */
    void clear() {
        pthread_mutex_lock(&lock);
        unsigned int size = queue.size();

        for (int pos = 0; pos < size; pos++) {
            T t = queue.front();
            if (release_listener) {
                release_listener(&t);
            }
            queue.pop();
        }

        pthread_mutex_unlock(&lock);

    }

    /**
     * 清理所有数据
     */
    void dumpQueue() {
        pthread_mutex_lock(&lock);
        if (dump_listener) { dump_listener(&queue); }
        pthread_mutex_unlock(&lock);
    }

//初始化锁和信号

};


#endif
