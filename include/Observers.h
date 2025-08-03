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

#ifndef Observers_h
#define Observers_h

#include <LinkedList.h>
#include <atomic>

/// @brief Observer pattern implementation
/// @tparam EventData Type of the event data
template <typename EventData>
class Observers
{
public: 
    /// @brief Constructs a new Observers object.
    /// Initializes the token to 0.
    Observers() :
        m_token(0)
    {
    }

    /// @brief Destroys the Observers object.
    /// Clears all observers from the list.
    ~Observers()
    {
        m_observers.ClearAll();
    }

    /// @brief Handler function type for observers.
    /// @param data The event data to pass to the observer.
    /// @param context Optional context pointer for additional data.
    /// This function type is used to define the signature of observer functions.
	typedef void(*Handler)(const EventData &data, const void *context);

private:
    /// @brief Data structure to hold observer information.
    /// This structure contains the token, handler function, and context pointer for each observer.
    class ObserverData
    {
    public:
        /// @brief Default constructor for ObserverData.
        ObserverData() :
            m_token(0),
            m_handler((Handler)0),
            m_context(NULL)
        {
        }

        /// @brief Parameterized constructor for ObserverData.
        /// @param token Unique identifier for the observer.
        /// @param handler Function pointer to the observer's handler.
        /// @param context Optional context pointer for additional data.
        ObserverData(int token, Handler handler, const void *context = NULL) :
            m_token(token),
            m_handler(handler),
            m_context(context)

        {
        }

        /// @brief Copy constructor for ObserverData.
        /// @param other The ObserverData object to copy from.
        /// This constructor initializes the observer data from another ObserverData object.
        ObserverData(const ObserverData &other)
        {
            *this = other;
        }

        /// @brief Assignment operator for ObserverData.
        /// @param other The ObserverData object to copy from.
        /// @return A reference to this ObserverData object.
        ObserverData &operator=(const ObserverData &other)
        {
            this->m_token = other.m_token;
            this->m_handler = other.m_handler;
            this->m_context = other.m_context;

            return *this;
        }

        /// @brief Equality operator for ObserverData.
        /// @param o The ObserverData object to compare with.
        /// @return True if the tokens are equal, false otherwise.
        /// This operator checks if two ObserverData objects are equal based on their tokens.
        bool operator==(const ObserverData &o) const
        {
            return m_token == o.m_token;
        }

        int m_token; ///< Unique identifier for the observer.
        Handler m_handler; ///< Function pointer to the observer's handler.
        const void *m_context; ///< Optional context pointer for additional data.
    };

public:
	/// @brief Adds an observer to the list.
	/// @param h The handler function for the observer.
	/// @param context Optional context pointer for additional data.
	/// @return A unique token identifying the observer.
	int addObserver(Handler h, const void *context = NULL)
	{
		m_observers.Insert(ObserverData(++m_token, h, context));
        return m_token;
	}

	/// @brief Removes an observer from the list.
	/// @param token The unique token identifying the observer returned by addObserver.
	/// @return True if the observer was successfully removed, false otherwise.
	bool removeObserver(int token)
	{
        return m_observers.Delete(ObserverData(token, NULL, NULL));
	}

    /// @brief Calls all observers with the given event data.
    /// @param data The event data to pass to the observers.
	void callObservers(const EventData &data)
	{
        // Scan observers and call their handlers with the event data and context.
        m_observers.ScanNodes([](const ObserverData &observer, const void *data)->bool
        {
            observer.m_handler(*(static_cast<const EventData *>(data)), observer.m_context);
            return true;
        }, &data);
	}

private:
	std::atomic<int> m_token; ///< Unique token for the next observer to be added.
	LinkedList<ObserverData> m_observers; ///< List of all registered observers.
};

#endif // Observers_h
