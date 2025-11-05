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

/// @brief A thread-safe doubly linked list.
/// @tparam T The type of elements stored in the list.
template<typename T>
class LinkedList
{
private:
    /// @brief A node in the linked list.
    /// @tparam T The type of value stored in the node.
    class ListNode
    {
    public:
        /// @brief Constructs a new ListNode with the given value.
        /// @param inValue The value to store in the node.
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
    /// @brief Constructs a new LinkedList.
    LinkedList() :
        head(NULL),
        tail(NULL)
    {
    }

    /// @brief Destroys the LinkedList and clears all nodes.
    ~LinkedList()
    {
        ClearAll();
    }

    /// @brief Inserts a new value into the list.
    /// @param value The value to insert.
    void Insert(T value)
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
    }

private:
    /// @brief Finds a node in the list.
    /// @param val The value to search for.
    /// @return A pointer to the node containing the value, or NULL if not found.
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

    /// @brief Deletes a node from the list.
    /// @param node The node to delete.
    /// @return A pointer to the next node after deletion, or NULL if the list is now empty.
    /// If the node is NULL, does nothing and returns NULL.
    ListNode *Delete(ListNode *node)
    {
        Lock lock(cs);

        if (node == NULL)
            return NULL;

        ListNode *ret = NULL;

        if (node->prev == NULL)
        {
            // Node is the head
            if (node->next == NULL)
            {
                // Node is the only element
                head = NULL;
                tail = NULL;
            }
            else
            {
                // Node is the head but not the only element
                head = node->next;
                head->prev = NULL;
                ret = head;
            }
        }
        else
        {
            // Node is not the head
            if (node->next == NULL)
            {
                // Node is the tail
                tail = node->prev;
                tail->next = NULL;
            }
            else
            {
                // Node is in the middle
                node->next->prev = node->prev;
                node->prev->next = node->next;
                ret = node->next;
            }
        }

        // Clean up the node
        node->next = NULL;
        node->prev = NULL;
        delete node;

        // Return the next node or NULL if the list is now empty
        return ret;
    }


public:
    /// @brief Deletes a node with the specified value from the list.
    /// @param val The value to delete.
    /// @return True if the node was found and deleted, false otherwise.
    /// If the node is not found, does nothing and returns false.
    bool Delete(T val)
    {
        Lock lock(cs);
        ListNode *n = Find(val);
        Delete(n);

        return n != NULL;
    }

    /// @brief Checks if the list is empty.
    /// @return True if the list is empty, false otherwise.
    bool IsEmpty()
    {
        Lock lock(cs);
        return head == NULL;
    }

    /// @brief Clears all nodes in the list.
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

    /// @brief Scans through all nodes in the list and applies an action to each value.
    /// @param action A function pointer that takes a value and a parameter, returning true to continue scanning or false to stop.
    /// @param param A parameter passed to the action function.
    /// The action function should return true to continue scanning or false to stop.
    /// @note The action function should not delete nodes or modify the list structure.
    void ScanNodes(bool (*action)(const T &value, void *param), void *param)
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