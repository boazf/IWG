/*
 * Copyright 2020-2025 Boaz Feldboim
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

// SPDX-License-Identifier: Apache-2.0

#ifndef LinkedList_h
#define LinkedList_h

#include <Lock.h>

template<typename T>
class LinkedList
{
private:
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
        ListNode *next;
        ListNode *prev;
    };

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

    void *Insert(T value)
    {
        Lock lock(cs);

        ListNode *newNode = new ListNode(value);
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

private:
    ListNode *Find(T val)
    {
        Lock lock(cs);
        ListNode *n;

        for(n = head; n != NULL; n = n->next)
        {
            if (n->value == val)
                return n;
        }

        return NULL;
    }

    ListNode *Delete(ListNode *node)
    {
        Lock lock(cs);

        if (node == NULL)
            return NULL;

        ListNode *ret = NULL;

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


public:
    bool Delete(T val)
    {
        Lock lock(cs);
        ListNode *n = Find(val);
        Delete(n);

        return n != NULL;
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
            ListNode *next = head->next;
            delete head;
            head = next;
        }
        head = NULL;
        tail = NULL;
    }

    void ScanNodes(bool (*action)(const T &value, const void *param), const void *param)
    {
        Lock lock(cs);

        ListNode *node = head;
        while(node != NULL)
        {
            if (!action(node->value, param))
                break;
            node = node->next;
        }
    }

private: 
    ListNode *head;
    ListNode *tail;

private:
    CriticalSection cs;
};

#endif // LinkedList_h