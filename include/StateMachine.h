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

template<typename Verb, typename State, typename Param>
class SMState
{
public:
	typedef void (*OnEntry)(Param *param);
	typedef Verb(*OnState)(Param *param);
	typedef Verb(*OnExit)(Verb verb, Param *param);

public:
	SMState() :
		m_state(static_cast<State>(0)),
		m_onEntry(0),
		m_onState(0),
		m_onExit(0)
	{
	}

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
		CopyTransitions(transitions, nTransitions);
	}

    SMState(const SMState<Verb, State, Param> &other)
    {
        *this = other;
    }

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
	State getState() const 
	{
		return m_state;
	}

	using TransitionsMap = std::map<Verb, State>;

	State PerformTransition(Verb verb) const
	{
		typename TransitionsMap::const_iterator state = m_transitions.find(verb);
		if (state != m_transitions.end())
			return state->second;

#ifdef DEBUG_STATE_MACHINE
		Tracef("Error: Transition not found, state=%s, verb=%s\n", 
			StringableEnum<State>(m_state).ToString().c_str(), 
			StringableEnum<Verb>(verb).ToString().c_str());
#endif		
		throw std::runtime_error("Transition not found");
	}

	void doEnter(Param *param) const
	{
		if (m_onEntry)
			m_onEntry(param);
	}

	Verb doState(Param *param) const
	{
		return m_onState(param);
	}

	Verb doExit(Verb verb, Param *param) const
	{
		return m_onExit != NULL ? m_onExit(verb, param) : verb;
	}

private:
	void CopyTransitions(const Transition<Verb, State> *transitions, int nTransitions)
	{
		assert(nTransitions > 0);
		for (int i = 0; i < nTransitions; i++)
			m_transitions.emplace(transitions[i].m_verb, transitions[i].m_state);
	}

private:
	State m_state;
	OnEntry m_onEntry;
	OnState m_onState;
	OnExit m_onExit;
	TransitionsMap m_transitions;

	friend class StateMachine<Verb, State, Param>;
};

template<typename Verb, typename State, typename Param>
struct StateMachine
{
	using StatesMap=std::map<State, SMState<Verb, State, Param>>;
public:
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
		if(states == NULL || nStates <= 0)
			throw std::invalid_argument("Invalid states");
		for (int i = 0; i < nStates; i++)
			m_states.emplace(states[i].getState(), states[i]);
		m_current = &m_states[states[0].getState()];
	}

	void Start()
	{
#ifdef DEBUG_STATE_MACHINE
		Tracef("State machine: %s, entering starting state: %s\n", 
			m_name.c_str(), 
			StringableEnum<State>(m_current->getState()).ToString().c_str());
#endif
		m_current->doEnter(m_param);
	}

	void HandleState()
	{
		m_nextVerb = m_current->doState(m_param);

		if (m_nextVerb == static_cast<Verb>(0))
		{
			return;
		}

#ifdef DEBUG_STATE_MACHINE
		Tracef("State machine: %s, exiting state: %s\n", 
			m_name.c_str(), 
			StringableEnum<State>(m_current->getState()).ToString().c_str());
#endif
		Verb verb = m_current->doExit(m_nextVerb, m_param);
#ifdef DEBUG_STATE_MACHINE
		Tracef("State machine: %s, transferring from state: %s, verb: %s\n", 
			m_name.c_str(), 
			StringableEnum<State>(m_current->getState()).ToString().c_str(), 
			StringableEnum<Verb>(verb).ToString().c_str());
#endif
		State state = m_current->PerformTransition(verb);
#ifdef DEBUG_STATE_MACHINE
		Tracef("State machine: %s, new state: %s\n", 
			m_name.c_str(), 
			StringableEnum<State>(state).ToString().c_str());
#endif
		typename StatesMap::const_iterator newState = m_states.find(state);
		if (newState == m_states.end())
		{
#ifdef DEBUG_STATE_MACHINE
			Tracef("Error: State machine: %s, unknown new state: %d (%s)\n", 
				m_name.c_str(), 
				static_cast<int>(state), 
				StringableEnum<State>(state).ToString().c_str());
#endif
			throw std::runtime_error("Unknown new state");
		}
		m_current = &(newState->second);
#ifdef DEBUG_STATE_MACHINE
		Tracef("State machine: %s, entering  state: %s\n", 
			m_name.c_str(), 
			StringableEnum<State>(m_current->getState()).ToString().c_str());
#endif
		m_current->doEnter(m_param);
	}

private:
	const SMState<Verb, State, Param> *m_current;
	Verb m_nextVerb;
	StatesMap m_states;
	Param *m_param;
	String m_name;
};

#endif // StateMachine_h