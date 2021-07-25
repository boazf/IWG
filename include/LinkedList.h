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
        Lock lock(cs);

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

    ListNode<T> *Find(T val)
    {
        Lock lock(cs);
        ListNode<T> *n;

        for(n = head; n != NULL; n = n->next)
        {
            if (n->value == val)
                return n;
        }

        return NULL;
    }

    ListNode<T> *Delete(ListNode<T> *node)
    {
        Lock lock(cs);

        if (node == NULL)
            return NULL;

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


    ListNode<T> *Delete(T val)
    {
        Lock lock(cs);
        return Delete(Find(val));
    }

    bool IsEmpty()
    {
        Lock lock(cs);
        return head == NULL;
    }

    void ClearAll()
    {
        Lock lock(cs);

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
        Lock lock(cs);

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

private:
    CriticalSection cs;
};

#endif // LinkedList_h