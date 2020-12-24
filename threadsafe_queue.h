#ifndef THREADSAFE_QUEUE_H
#define THREADSAFE_QUEUE_H

#include <queue>
#include <mutex>
#include <condition_variable>
#include <memory>

template <typename T>
class Threadsafe_queue
{
private:
    mutable std::mutex m_mut;
    std::queue<T> m_data_queue;
    std::condition_variable m_data_cond;

public:
    Threadsafe_queue()
    {}
    Threadsafe_queue(const Threadsafe_queue& other)
    {
        std::lock_guard<std::mutex> lk(other.m_mut);
        m_data_queue = other.m_data_queue;
    }

    Threadsafe_queue& operator=(const Threadsafe_queue&) = delete;

    void push(T new_value)
    {
        std::lock_guard<std::mutex> lk(m_mut);
        m_data_queue.push(new_value);
        m_data_cond.notify_one();
    }

    void wait_and_pop(T& value)
    {
        std::unique_lock<std::mutex> lk(m_mut);
        m_data_cond.wait(lk, [this] {return !m_data_queue.empty();});
        value = m_data_queue.front();
        m_data_queue.pop();
    }

    std::shared_ptr<T> wait_and_pop()
    {
        std::unique_lock<std::mutex> lk(m_mut);
        m_data_cond.wait(lk, [this] {return !m_data_queue.empty();});
        std::shared_ptr<T> res(std::make_shared<T>(m_data_queue.front()));
        m_data_queue.pop();
        return res;
    }

    bool try_pop(T& value)
    {
        std::lock_guard<std::mutex> lk(m_mut);
        if (m_data_queue.empty())
            return false;
        value = m_data_queue.front();
        m_data_queue.pop();
        return true;
    }

    std::shared_ptr<T> try_pop()
    {
        std::lock_guard<std::mutex> lk(m_mut);
        if (m_data_queue.empty())
            return std::shared_ptr<T>();
        std::shared_ptr<T> res(std::make_shared<T>(m_data_queue.front()));
        m_data_queue.pop();
        return res;
    }

    bool empty() const
    {
        std::lock_guard<std::mutex> lk(m_mut);
        return m_data_queue.empty();
    }
};

#endif // THREADSAFE_QUEUE_H
