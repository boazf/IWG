#ifndef LinkedList_h
#define LinkedList_h

#include <Lock.h>

template<typename T>
class ListNode
{
public:
    ListNode(T inValue) :
        value(inValue),
        next(NULL),
        prev(NULL)
    {
    }

    ~ListNode()
    {
    }

public:
    T value;
    ListNode<T> *next;
    ListNode<T> *prev;
};

template<typename T>
class LinkedList
{
public:
    LinkedList() :
        head(NULL),
        tail(NULL)
    {
    }

    ~LinkedList()
    {
        ClearAll();
    }

    ListNode<T> *Insert(T value)
    {
#ifdef ESP32
        Lock lock(cs);
#endif

        ListNode<T> *newNode = new ListNode<T>(value);
        if (head == NULL)
        {
            head = newNode;
            tail = newNode;
        }
        else
        {
            tail->next = newNode;
            newNode->prev = tail;
            tail = newNode;
        }
        
        return newNode;
    }

    ListNode<T> *Delete(ListNode<T> *node)
    {
#ifdef ESP32
        Lock lock(cs);
#endif

        ListNode<T> *ret = NULL;

        if (node->prev == NULL)
        {
            if (node->next == NULL)
            {
                head = NULL;
                tail = NULL;
            }
            else
            {
                head = node->next;
                head->prev = NULL;
                ret = head;
            }
        }
        else
        {
            if (node->next == NULL)
            {
                tail = node->prev;
                tail->next = NULL;
            }
            else
            {
                node->next->prev = node->prev;
                node->prev->next = node->next;
                ret = node->next;
            }
        }

        node->next = NULL;
        node->prev = NULL;
        delete node;

        return ret;
    }

    void ClearAll()
    {
#ifdef ESP32
        Lock lock(cs);
#endif

        while(head != NULL)
        {
            ListNode<T> *next = head->next;
            delete head;
            head = next;
        }
        head = NULL;
        tail = NULL;
    }

    void ScanNodes(bool (*action)(const T &value, const void *param), const void *param)
    {
#ifdef ESP32
        Lock lock(cs);
#endif

        ListNode<T> *node = head;
        while(node != NULL)
        {
            if (!action(node->value, param))
                break;
            node = node->next;
        }
    }

public: 
    ListNode<T> *head;
    ListNode<T> *tail;

#ifdef ESP32
private:
    CriticalSection cs;
#endif
};

#endif // LinkedList_h