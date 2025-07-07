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

#ifndef StateMachine_h
#define StateMachine_h

#include <Arduino.h>
#include <assert.h>
#include <Common.h>
#ifdef DEBUG_STATE_MACHINE
#include <StringableEnum.h>
#include <Trace.h>
#endif
#include <map>

#define TRANSITIONS(a) a, NELEMS(a)

/// @brief Class representing a transition in a state machine.
/// @tparam Verb When a state machine is in a certain state, it can perform actions based on verbs.
/// @tparam State State to transfer to when the verb is returned from the state.
/// @note The transition is defined by a verb and a state. When the verb is returned
/// from the state, the state machine will transition to the specified state.
/// @note A state returns a verb upon returning from the state function. The exit function receives the verb
/// and can modify it before the state machine transitions to the next state.
template<typename Verb, typename State>
class Transition
{
public:
	Transition()
	{
	}

	Transition(Verb verb, State state)
	{
		m_verb = verb;
		m_state = state;
	}

	Transition(const Transition &other)
	{
        *this = other;
    }

	Transition &operator =(const Transition &other)
	{
		m_verb = other.m_verb;
		m_state = other.m_state;

		return *this;
	}

	Verb m_verb;
	State m_state;
};

template<typename Verb, typename State, typename Param>
struct StateMachine;

/// @brief State machine state class.
/// @tparam Verb The verb type that triggers transitions in the state machine.
/// @tparam State The state type that the state machine can be in.
/// @note The state machine can have multiple states, each with its own entry, state, and exit actions.
/// @tparam Param A context parameter type that can be used to pass additional information to the states of the state machine.
template<typename Verb, typename State, typename Param>
class SMState
{
public:
	/// @brief Function pointer type for entry action of the state.
	typedef void (*OnEntry)(Param *param);
	/// @brief Function pointer type for state action of the state.
	/// @note The state action can return a verb that will be used to determine the next state.
	typedef Verb(*OnState)(Param *param);
	/// @brief Function pointer type for exit action of the state.
	/// @note The exit action can modify the verb before the state machine transitions to the next
	typedef Verb(*OnExit)(Verb verb, Param *param);

public:
	/// @brief Default constructor for SMState.
	/// @note Initializes the state to 0 and all function pointers to NULL.
	SMState() :
		m_state(static_cast<State>(0)),
		m_onEntry(NULL),
		m_onState(NULL),
		m_onExit(NULL)
	{
	}

	/// @brief Class constructor for SMState.
	/// @param state The state identifier for this state.
	/// @param onEntry The entry action function pointer for this state.
	/// @param onState The state action function pointer for this state.
	/// @param onExit The exit action function pointer for this state.
	/// @param transitions An array of transitions for this state, where each transition is defined by a verb and a state.
	/// @param nTransitions The number of transitions in the transitions array.
	/// @note The transitions array is copied into the state machine state.
	SMState(
		State state, 
		OnEntry onEntry, 
		OnState onState, 
		OnExit onExit, 
		const Transition<Verb, State> *transitions, 
		int nTransitions) :
		m_state(state),
		m_onEntry(onEntry),
		m_onState(onState),
		m_onExit(onExit)
	{
		//Copy the transitions into the state machine state
		CopyTransitions(transitions, nTransitions);
	}

    /// @brief Copy constructor for SMState.
    /// @param other The other SMState to copy from.
	/// @note Copies the state, function pointers, and transitions from the other SMState.
    SMState(const SMState<Verb, State, Param> &other)
    {
        *this = other;
    }

	/// @brief Assignment operator for SMState.
	/// @param other The other SMState to assign from.
	/// @note Copies the state, function pointers, and transitions from the other SMState.
	/// @return The current SMState object after assignment.
	SMState &operator =(const SMState &other)
	{
		m_state = other.m_state;
		m_onEntry = other.m_onEntry;
		m_onState = other.m_onState;
		m_onExit = other.m_onExit;
		m_transitions = other.m_transitions;

		return *this;
	}

private:
	/// @brief Get the current state identifier
	/// @return The current state identifier.
	State getState() const 
	{
		return m_state;
	}

	using TransitionsMap = std::map<Verb, State>;

	/// @brief Perform a transition based on the verb.
	/// @param verb The verb that triggers the transition.
	/// @note This function looks up the verb in the transitions map and returns the corresponding state
	/// @return The new state identifier to transition to.
	/// @throws std::runtime_error if the verb is not found in the transitions map.
	State PerformTransition(Verb verb) const
	{
		// Look for the verb in the transitions map
		typename TransitionsMap::const_iterator state = m_transitions.find(verb);
		if (state != m_transitions.end()) // found
			return state->second;

		// Verb not found, throw an error
#ifdef DEBUG_STATE_MACHINE
		Tracef("Error: Transition not found, state=%s, verb=%s\n", 
			StringableEnum<State>(m_state).ToString().c_str(), 
			StringableEnum<Verb>(verb).ToString().c_str());
#endif		
		throw std::runtime_error("Transition not found");
	}

	/// @brief Perform the entry action for this state.
	/// @param param The context parameter to pass to the entry action.
	/// @note This function calls the entry action function pointer if it is not NULL.
	void doEnter(Param *param) const
	{
		if (m_onEntry)
			m_onEntry(param);
	}

	/// @brief Perform the state action for this state.
	/// @param param The context parameter to pass to the state action.
	/// @throws std::runtime_error if the state action function pointer is NULL.
	/// @return The verb identifying the next state to transition to.
	/// @note This function calls the state action function pointer and returns the verb identifying the next state.
	/// The next state can be the same state or a different one, depending on the verb returned by the state action.
	/// Always the first Verb in the enumeration is considered as "no action" and will not cause no state transition.
	Verb doState(Param *param) const
	{
		return m_onState(param);
	}

	/// @brief Perform the exit action for this state.
	/// @param verb The verb returned from the state action.
	/// @param param The context parameter to pass to the exit action.
	/// @note This function calls the exit action function pointer if it is not NULL.
	/// @return The verb that may be modified by the exit action before the state machine transitions to the next state.
	/// @note If the exit action is NULL, it simply returns the verb as is.
	Verb doExit(Verb verb, Param *param) const
	{
		return m_onExit != NULL ? m_onExit(verb, param) : verb;
	}

private:
	/// @brief Copy transitions from an array of Transition objects.
	/// @param transitions The array of Transition objects to copy from.
	/// @note This function copies the transitions from the provided array into the internal transitions map.
	/// @param nTransitions The number of transitions in the array.
	/// @throws std::invalid_argument if nTransitions is less than or equal to 0
	void CopyTransitions(const Transition<Verb, State> *transitions, int nTransitions)
	{
		if (nTransitions <= 0)
			throw std::invalid_argument("Invalid number of transitions");
		for (int i = 0; i < nTransitions; i++)
			m_transitions.emplace(transitions[i].m_verb, transitions[i].m_state);
	}

private:
	/// @brief 
	State m_state;
	/// @brief Function pointer for the entry action of the state.
	OnEntry m_onEntry;
	/// @brief Function pointer for the state action of the state.
	OnState m_onState;
	/// @brief Function pointer for the exit action of the state.
	OnExit m_onExit;
	/// @brief Map of transitions for this state, where the key is the verb and the value is the state to transition to.
	TransitionsMap m_transitions;

	/// @brief Allow StateMachine to access private members of SMState
	friend class StateMachine<Verb, State, Param>;
};

/// @brief The state machine class.
/// @tparam Verb The verb type that triggers transitions in the state machine. This should be an enum or similar type.
/// @tparam State The state type that the state machine can be in. This should be an enum or similar type.
/// @note The state machine can have multiple states, each with its own entry, state, and exit actions.
/// @note Each state in the state machine is identified by a unique State identifier from the State enum.
/// @tparam Param The context parameter type that can be used to pass additional information to the states of the state machine.
template<typename Verb, typename State, typename Param>
struct StateMachine
{
	using StatesMap=std::map<State, SMState<Verb, State, Param>>;
public:
	/// @brief The constructor for the StateMachine class.
	/// @param states The array of states that the state machine can be in.
	/// @note The states array should contain at least one state, and the first state in the array will be the starting state of the state machine.
	/// @throws std::invalid_argument if the states array is NULL or the number of states is less than or equal to 0.
	/// @note The states array should contain valid SMState objects, each with a unique State identifier.
	/// @note The state machine will copy the states into its internal map, so the states array can be destroyed after the StateMachine is constructed.
	/// @param nStates The number of states in the states array. std::invalid_argument if nStates is less than or equal to 0.
	/// @param param The context parameter that will be passed to the states of the state machine.
	/// @note This parameter can be used to pass additional information to the states, such as configuration data or context information.
	/// @note The state machine will not take ownership of the param pointer, so it should remain valid for the lifetime of the StateMachine object.
	/// @note The param pointer can be NULL if no context parameter is needed.
	/// @param name The name of the state machine for debugging purposes.
	/// @note This parameter is optional and can be used to identify the state machine in debugging output.
	/// @note If DEBUG_STATE_MACHINE is defined, the name will be used in debug output
	StateMachine(SMState<Verb, State, Param> states[], int nStates, Param *param
#ifdef DEBUG_STATE_MACHINE
		, const char *name
#endif
		) :
		m_param(param)
#ifdef DEBUG_STATE_MACHINE
		, m_name(name)
#endif
	{
		/// @brief Verify that the states array is valid and contains at least one state.
		/// @throws std::invalid_argument if the states array is NULL or the number of states is less than or equal to 0.
		if(states == NULL || nStates <= 0)
			throw std::invalid_argument("Invalid states");
		/// @brief Copy the states into the internal map of the state machine.
		/// @note The states array should contain valid SMState objects, each with a unique State identifier.
		for (int i = 0; i < nStates; i++)
			m_states.emplace(states[i].getState(), states[i]);
		/// @brief Set the current state to the first state in the states array.
		m_current = &m_states[states[0].getState()];
	}

	/// @brief Enter the starting state of the state machine.
	/// @note This function calls the entry action of the starting state, if it is defined
	void Start()
	{
#ifdef DEBUG_STATE_MACHINE
		Tracef("State machine: %s, entering starting state: %s\n", 
			m_name.c_str(), 
			StringableEnum<State>(m_current->getState()).ToString().c_str());
#endif
		m_current->doEnter(m_param);
	}

	/// @brief Handle the current state of the state machine.
	/// @note This function calls the state action of the current state, which may return a verb that triggers a transition to a new state.
	/// @note If the state action returns a verb that is the first Verb in the enumeration (usually Verb::NONE or similar), no state transition will occur.
	/// @note If a transition occurs, the exit action of the current state will be called with the verb returned from the state action.
	/// @note After the exit action, the state machine will transition to the new state returned from the PerformTransition function.
	/// @note After the transition, the entry action of the new state will be called. The new state action will be executed on the next call to HandleState.
	/// @note This function should be called repeatedly to handle the state machine's behavior.
	/// @throws std::runtime_error if the transition is not found or the new state is not found in the states map.
	void HandleState()
	{
		/// @brief call the state action of the current state.
		Verb m_nextVerb = m_current->doState(m_param);

		/// @brief If the state action returns the first Verb in the enumeration, no state transition will occur.
		/// @note This is usually Verb::NONE or similar, indicating that the state should remain the same.
		if (m_nextVerb == static_cast<Verb>(0))
		{
			return;
		}

#ifdef DEBUG_STATE_MACHINE
		Tracef("State machine: %s, exiting state: %s\n", 
			m_name.c_str(), 
			StringableEnum<State>(m_current->getState()).ToString().c_str());
#endif
		/// @brief call the exit action of the current state with the verb returned from the state action.
		Verb verb = m_current->doExit(m_nextVerb, m_param);
#ifdef DEBUG_STATE_MACHINE
		Tracef("State machine: %s, transferring from state: %s, verb: %s\n", 
			m_name.c_str(), 
			StringableEnum<State>(m_current->getState()).ToString().c_str(), 
			StringableEnum<Verb>(verb).ToString().c_str());
#endif
		/// @brief Perform the transition based on the verb returned from the exit action.
		State state = m_current->PerformTransition(verb);
#ifdef DEBUG_STATE_MACHINE
		Tracef("State machine: %s, new state: %s\n", 
			m_name.c_str(), 
			StringableEnum<State>(state).ToString().c_str());
#endif
		/// @brief Find the new state in the states map.
		typename StatesMap::const_iterator newState = m_states.find(state);
		if (newState == m_states.end())
		{
			/// @brief If the new state is not found in the states map, throw an error.
			/// @throws std::runtime_error if the new state is not found in the states map.
#ifdef DEBUG_STATE_MACHINE
			Tracef("Error: State machine: %s, unknown new state: %d (%s)\n", 
				m_name.c_str(), 
				static_cast<int>(state), 
				StringableEnum<State>(state).ToString().c_str());
#endif
			throw std::runtime_error("Unknown new state");
		}
		/// @brief Set the current state to the new state found in the states map.
		m_current = &(newState->second);
#ifdef DEBUG_STATE_MACHINE
		Tracef("State machine: %s, entering  state: %s\n", 
			m_name.c_str(), 
			StringableEnum<State>(m_current->getState()).ToString().c_str());
#endif
		/// @brief Call the entry action of the new state.
		m_current->doEnter(m_param);
	}

private:
	/// @brief Pointer to the current state of the state machine.
	/// @note This pointer is set to the first state in the states map when the state machine is constructed.
	/// @note It is updated to point to the new state after each transition.
	const SMState<Verb, State, Param> *m_current;
	/// @brief Map of states in the state machine, where the key is the State identifier and the value is the SMState object.
	StatesMap m_states;
	/// @brief Pointer to the context parameter that is passed to the states of the state machine.
	Param *m_param;
	/// @brief Name of the state machine for debugging purposes.
	/// @note This name is used in debug output to identify the state machine.
	String m_name;
};

#endif // StateMachine_h