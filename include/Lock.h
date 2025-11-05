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

#ifndef Lock_h
#define Lock_h

#include <Arduino.h>

/// @brief A critical section class that provides thread-safe locking mechanisms.
/// This class uses FreeRTOS semaphores to ensure that critical sections are protected from concurrent access.
class CriticalSection
{
public:
    CriticalSection()
    {
        _binarySem = xSemaphoreCreateRecursiveMutex();
    }

    ~CriticalSection()
    {
        // Delete the semaphore when the critical section is destroyed.
        vSemaphoreDelete(_binarySem);
    }

    /// @brief Enters the critical section, blocking until it can acquire the semaphore.
    /// @note This method should be called before entering a critical section to ensure thread safety.
    /// It uses a recursive mutex, allowing the same thread to enter the critical section multiple times.
    /// @note The semaphore must be given once for each time it is taken.
    void Enter() const
    {
        //portENTER_CRITICAL(&mux);
        xSemaphoreTakeRecursive(_binarySem, portMAX_DELAY);
    }

    /// @brief Leaves the critical section, releasing the semaphore.
    /// @note This method should be called after exiting a critical section to allow other threads to enter.
    /// It uses a recursive mutex, allowing the same thread to leave the critical section multiple times.
    /// @note The semaphore must be given once for each time it is taken.
    void Leave() const
    {
        xSemaphoreGiveRecursive(_binarySem);
    }

private:
    xSemaphoreHandle _binarySem;
};

/// @brief A lock class that provides a scoped lock for critical sections.
/// This class automatically acquires the lock when constructed and releases it when destroyed.
/// It is designed to be used in a RAII (Resource Acquisition Is Initialization) style.
class Lock
{
public:
    /// @brief Constructs a new Lock object.
    /// @param cs The critical section to lock.
    Lock(const CriticalSection &cs) : _cs(cs)
    {
        _cs.Enter();
    }

    /// @brief Destroys the Lock object, releasing the lock.
    /// @note This method is called automatically when the Lock object goes out of scope.
    /// It ensures that the critical section is left, allowing other threads to enter.
    ~Lock()
    {
        _cs.Leave();
    }

private:
    /// @brief The critical section to lock.
    const CriticalSection &_cs;
};

/// @brief A block lock class that provides a scoped lock for critical sections.
/// This class is used to create a block of code that is protected by a lock.
/// It is designed to be used in a loop or conditional context, allowing the lock to be acquired once.
/// @note The `once` member variable ensures that the lock is only acquired once per block.
/// Usage example: for (BlockLock _lock(cs); _lock.once; _lock.once = false) { ... }
/// This is useful in creating macros that require a lock to be held for the duration of the block.
/// Example: #define CRITICAL_BLOCK(csLock) for (BlockLock _lock(csLock); _lock.once; _lock.once = false)
/// Then use: CRITICAL_BLOCK(cs) { ... }
class BlockLock : public Lock
{
public:
    BlockLock(const CriticalSection &cs) : Lock(cs), once(true) {}
    bool once;
};

#define CRITICAL_BLOCK(csLock) for (BlockLock _lock(csLock); _lock.once; _lock.once = false)
#endif // Lock_h