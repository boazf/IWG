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
		assert(false);
		return (State)0;
	}

	void doEnter(Param *param) const
	{
		m_onEntry(param);
	}

	Verb doState(Param *param) const
	{
		return m_onState(param);
	}

	Verb doExit(Verb verb, Param *param) const
	{
		return m_onExit(verb, param);
	}

	State getState() const 
	{
		return m_state;
	}

public:
	static void OnEnterDoNothing(Param *param)
	{
	}

	static Verb OnExitDoNothing(Verb verb, Param *param)
	{
		return verb;
	}

private:
	void CopyTransitions(const Transition<Verb, State> *transitions, int nTransitions)
	{
		assert(nTransitions > 0);
		for (int i = 0; i < nTransitions; i++)
			m_transitions[transitions[i].m_verb] = transitions[i].m_state;
	}

private:
	State m_state;
	OnEntry m_onEntry;
	OnState m_onState;
	OnExit m_onExit;
	TransitionsMap m_transitions;
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
		m_first(true),
		m_param(param)
#ifdef DEBUG_STATE_MACHINE
		, m_name(name)
#endif
	{
		assert(nStates > 0);
		for (int i = 0; i < nStates; i++)
			m_states[states[i].getState()] = states[i];
		m_current = &m_states[states[0].getState()];
	}

	void ApplyVerb(Verb verb)
	{
		m_nextVerb = verb;
	}

	void HandleState()
	{
		if (!m_first)
		{
			if (m_nextVerb != static_cast<Verb>(0))
			{
#ifdef DEBUG_STATE_MACHINE
				Tracef("State machine: %s, exiting state: %s\n", 
					m_name, 
					StringableEnum<State>(m_current->getState()).ToString().c_str());
#endif
				Verb verb = m_current->doExit(m_nextVerb, m_param);
#ifdef DEBUG_STATE_MACHINE
				Tracef("State machine: %s, transferring from state: %s, verb: %s\n", 
					m_name, 
					StringableEnum<State>(m_current->getState()).ToString().c_str(), 
					StringableEnum<Verb>(verb).ToString().c_str());
#endif
				State state = m_current->PerformTransition(verb);
#ifdef DEBUG_STATE_MACHINE
				Tracef("State machine: %s, new state: %s\n", 
					m_name, 
					StringableEnum<State>(state).ToString().c_str());
#endif
				typename StatesMap::const_iterator newState = m_states.find(state);
#ifdef DEBUG_STATE_MACHINE
				if (newState == m_states.end())
					Tracef("Error: State machine: %s, unknown new state: %n (%s)\n", 
						m_name, 
						static_cast<int>(state), 
						StringableEnum<State>(state).ToString().c_str());
#endif
				assert(newState != m_states.end());
				m_current = &(newState->second);
#ifdef DEBUG_STATE_MACHINE
				Tracef("State machine: %s, entering  state: %s\n", 
					m_name, 
					StringableEnum<State>(m_current->getState()).ToString().c_str());
#endif
				m_current->doEnter(m_param);
			}
		}
		else
		{
#ifdef DEBUG_STATE_MACHINE
			Tracef("State machine: %s, entering starting state: %s\n", 
				m_name, 
				StringableEnum<State>(m_current->getState()).ToString().c_str());
#endif
			m_current->doEnter(m_param);
			m_first = false;
		}
		Verb newVerb = m_current->doState(m_param);
		ApplyVerb(newVerb);
	}

	SMState<Verb, State, Param> *current()
	{
		return m_current;
	}

private:
	const SMState<Verb, State, Param> *m_current;
	Verb m_nextVerb;
	StatesMap m_states;
	bool m_first;
	Param *m_param;
	const char *m_name;
};

#endif // StateMachine_h