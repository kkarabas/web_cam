#include "threadsafe_queue.h"
/*
template <typename T>
Threadsafe_queue<T>::Threadsafe_queue(const Threadsafe_queue& other)
{
    std::lock_guard<std::mutex> lk(other.m_mut);
    m_data_queue = other.m_data_queue;
}

template <typename T>
void Threadsafe_queue<T>::push(T new_value)
{
    std::lock_guard<std::mutex> lk(m_mut);
    m_data_queue.push(new_value);
    m_data_cond.notify_one();
}

template <typename T>
void Threadsafe_queue<T>::wait_and_pop(T& value)
{
    std::unique_lock<std::mutex> lk(m_mut);
    m_data_cond.wait(lk, [this] {return !m_data_queue.empty();});
    value = m_data_queue.front();
    m_data_queue.pop();
}

template <typename T>
std::shared_ptr<T> Threadsafe_queue<T>::wait_and_pop()
{
    std::unique_lock<std::mutex> lk(m_mut);
    m_data_cond.wait(lk, [this] {return !m_data_queue.empty();});
    std::shared_ptr<T> res(std::make_shared<T>(m_data_queue.front()));
    m_data_queue.pop();
    return res;
}

template <typename T>
bool Threadsafe_queue<T>::try_pop(T& value)
{
    std::lock_guard<std::mutex> lk(m_mut);
    if (m_data_queue.empty())
        return false;
    value = m_data_queue.front();
    m_data_queue.pop();
    return true;
}

template <typename T>
std::shared_ptr<T> Threadsafe_queue<T>::try_pop()
{
    std::lock_guard<std::mutex> lk(m_mut);
    if (m_data_queue.empty())
        return std::shared_ptr<T>();
    std::shared_ptr<T> res(std::make_shared<T>(m_data_queue.front()));
    m_data_queue.pop();
    return res;
}

template <typename T>
bool Threadsafe_queue<T>::empty() const
{
    std::lock_guard<std::mutex> lk(m_mut);
    return m_data_queue.empty();
}
*/
