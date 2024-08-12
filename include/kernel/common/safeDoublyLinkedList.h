#ifndef SAFEDOUBLYLINKEDLIST_H
#define SAFEDOUBLYLINKEDLIST_H

#include "common/doublyLinkedList.h"
#include "process/mutex.h"

namespace Common
{
    template<typename T>
    class SafeDoublyLinkedList : private DoublyLinkedList<T>
    {
    public:
        void clear()
        {
            m_mutex.acquire();
            DoublyLinkedList<T>::clear();
            m_mutex.release();
        }
        uint64_t size()
        {
            m_mutex.acquire();
            DoublyLinkedList<T>::size();
            m_mutex.release();
        }
        DoublyLinkedListIterator<T> append(T&& value)
        {
            m_mutex.acquire();
            auto ret = DoublyLinkedList<T>::append(move(value));
            m_mutex.release();
            return ret;
        }

        DoublyLinkedListIterator<T> append(const T& value)
        {
            m_mutex.acquire();
            auto ret = DoublyLinkedList<T>::append(value);
            m_mutex.release();
            return ret;
        }
        DoublyLinkedListIterator<T> insert(T&& value, DoublyLinkedListIterator<T> it)
        {
            m_mutex.acquire();
            auto ret = DoublyLinkedList<T>::insert(move(value), it);
            m_mutex.release();
            return ret;
        }
        DoublyLinkedListIterator<T> insert(const T& value, DoublyLinkedListIterator<T> it)
        {
            m_mutex.acquire();
            auto ret = DoublyLinkedList<T>::insert(value, it);
            m_mutex.release();
            return ret;
        }

        DoublyLinkedListIterator<T> remove(DoublyLinkedListIterator<T> it)
        {
            m_mutex.acquire();
            auto ret = DoublyLinkedList<T>::remove(it);
            m_mutex.release();
            return ret;
        }

        DoublyLinkedListIterator<T> first()
        {
            return DoublyLinkedList<T>::first();
        }

        DoublyLinkedListIterator<T> last()
        {
            return DoublyLinkedList<T>::last();
        }

    private:
    Mutex m_mutex;
    };
}

#endif
