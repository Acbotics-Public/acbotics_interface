/*******************************************************************/
/*    NAME: Oscar Viquez                                           */
/*    ORG:  Acbotics Research, LLC                                 */
/*    FILE: thread_safe_queue.h                                    */
/*    DATE: Apr 4th 2024                                           */
/*                                                                 */
/*    For help, contact us at: support@acbotics.com                */
/*******************************************************************/

#ifndef thread_safe_queue_HEADER
#define thread_safe_queue_HEADER

#include <condition_variable>
#include <mutex>
#include <queue>

class tsQueueBase {
public:
  virtual ~tsQueueBase() {} // Virtual destructor for proper cleanup
};

template <typename T> class tsQueue : public tsQueueBase {
private:
  std::queue<T> m_queue;
  std::mutex m_mutex;
  std::condition_variable m_cond;
  size_t m_length;

public:
  tsQueue() { this->m_length = 0; }
  tsQueue(size_t max_length) : tsQueue() { this->m_length = max_length; }

  // Insert items
  // ============
  void push(T item) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_queue.push(item);
    if (m_length > 0 && m_queue.size() > m_length) {
      m_queue.pop();
    }
    m_cond.notify_one();
  }

  void push_all(std::vector<T> item_vec) {
    std::lock_guard<std::mutex> lock(m_mutex);
    for (int ii = 0; ii < item_vec.size(); ii++) {
      m_queue.push(item_vec.at(ii));
    }
    while (m_length > 0 && m_queue.size() > m_length) {
      m_queue.pop();
    }
    m_cond.notify_one();
  }

  // Retrieve items
  // ==============
  T pop() {
    std::unique_lock<std::mutex> lock(m_mutex);
    m_cond.wait(lock, [this]() { return !m_queue.empty(); });

    T item = m_queue.front();
    m_queue.pop();
    return item;
  }

  bool pop(T &dst) {
    std::unique_lock<std::mutex> lock(m_mutex);
    if (m_cond.wait_for(lock, std::chrono::seconds(1), [this]() { return !m_queue.empty(); })) {

      dst = m_queue.front();
      m_queue.pop();
      return true;
    }
    return false;
  }

  std::vector<T> pop_N(size_t nn) {
    std::unique_lock<std::mutex> lock(m_mutex);
    m_cond.wait(lock, [this, nn]() { return m_queue.size() >= nn; });

    std::vector<T> vec;
    size_t q_count = m_queue.size();
    for (int ii = 0; ii < q_count; ii++) {
      vec.push_back(m_queue.front());
      m_queue.pop();
    }
    return vec;
  }

  std::vector<T> pop_all() {
    std::unique_lock<std::mutex> lock(m_mutex);
    m_cond.wait(lock, [this]() { return !m_queue.empty(); });

    std::vector<T> vec;
    size_t q_count = m_queue.size();
    for (int ii = 0; ii < q_count; ii++) {
      vec.push_back(m_queue.front());
      m_queue.pop();
    }
    return vec;
  }

  bool pop_all(std::vector<T> &dst) {
    std::unique_lock<std::mutex> lock(m_mutex);
    if (m_cond.wait_for(lock, std::chrono::seconds(1), [this]() { return !m_queue.empty(); })) {

      dst.clear();
      size_t q_count = m_queue.size();
      for (int ii = 0; ii < q_count; ii++) {
        dst.push_back(m_queue.front());
        m_queue.pop();
      }
      return true;
    }
    return false;
  }

  std::vector<T> pop_limit(int max_its) {
    std::unique_lock<std::mutex> lock(m_mutex);
    m_cond.wait(lock, [this]() { return !m_queue.empty(); });

    std::vector<T> vec;
    size_t q_count = m_queue.size();
    for (int ii = 0; ii < q_count && ii < max_its; ii++) {
      vec.push_back(m_queue.front());
      m_queue.pop();
    }
    return vec;
  }

  T front() {
    std::unique_lock<std::mutex> lock(m_mutex);
    m_cond.wait(lock, [this]() { return !m_queue.empty(); });

    T item = m_queue.front();
    return item;
  }

  T back() {
    std::unique_lock<std::mutex> lock(m_mutex);
    m_cond.wait(lock, [this]() { return !m_queue.empty(); });

    T item = m_queue.back();
    return item;
  }

  // Clear queue
  // ===========
  void clear() {
    std::lock_guard<std::mutex> lock(m_mutex);
    size_t q_count = m_queue.size();
    for (int ii = 0; ii < q_count; ii++) {
      m_queue.pop();
    }
  }

  // Query queue info
  // ================
  size_t size() {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_queue.size();
  }
};

#endif
