#ifndef PRIORITYQUEUE_H
#define PRIORITYQUEUE_H

#include "common/doublyLinkedList.h"

namespace Common {
    template<typename T>
    class PriorityQueue
    {
        public:
        T& top()
        {
            return *m_ll.first();
        }
        void pop()
        {
            auto it = m_ll.first();
            if (it.is_end()) { return; }
            m_ll.remove(it);
        }
        void push(T item)
        {
            // Compare the element to insert with each element and insert it
            // before the first element it is larger than.

            // This keeps the list sorted.
            auto it = m_ll.first();
            for (; !it.is_end(); ++it)
            {
                if (item > *it) {
                    m_ll.insert_before(item, it);
                    break;
                }
            }
            if (it.is_end())
            {
                m_ll.append(item);
            }
        }
        uint64_t size() { return m_ll.size(); }
        private:
        DoublyLinkedList<T> m_ll;
    };
}

#endif
