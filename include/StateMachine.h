#ifndef StateMachine_h
#define StateMachine_h

#include <Arduino.h>
#include <assert.h>
#include <Common.h>
#ifdef DEBUG_STATE_MACHINE
#include <StringableEnum.h>
#endif

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

template<typename Verb, typename State>
class SMState
{
public:
	typedef void (*OnEntry)(void *param);
	typedef Verb(*OnState)(void *param);
	typedef Verb(*OnExit)(Verb verb, void *param);

public:
	SMState() :
		m_state(static_cast<State>(0)),
		m_onEntry(0),
		m_onState(0),
		m_onExit(0),
		m_transitions(NULL),
		m_nTransitions(0)
	{
	}

	~SMState()
	{
		delete[] m_transitions;
		m_transitions = NULL;
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
		m_onExit(onExit),
		m_nTransitions(nTransitions)
	{
		m_transitions = new Transition<Verb, State>[m_nTransitions];
		for (int i = 0; i < m_nTransitions; i++)
			m_transitions[i] = transitions[i];
	}

    SMState(const SMState<Verb, State> &other)
    {
        *this = other;
    }

	SMState &operator =(const SMState &other)
	{
		m_state = other.m_state;
		m_onEntry = other.m_onEntry;
		m_onState = other.m_onState;
		m_onExit = other.m_onExit;
		m_nTransitions = other.m_nTransitions;
		m_transitions = new Transition <Verb, State>[m_nTransitions];
		for (int i = 0; i < m_nTransitions; i++)
			m_transitions[i] = other.m_transitions[i];

		return *this;
	}

	State PerformTransition(Verb verb)
	{
		for (int i = 0; i < m_nTransitions; i++)
		{
			if (m_transitions[i].m_verb == verb)
				return m_transitions[i].m_state;
		}
#ifdef DEBUG_STATE_MACHINE
		Tracef("Error: Transition not found, state=%s, verb=%s\n", 
			StringableEnum<State>(m_state).ToString().c_str(), 
			StringableEnum<Verb>(verb).ToString().c_str());
#endif		
		assert(false);
		return (State)0;
	}

	void doEnter(void *param)
	{
		m_onEntry(param);
	}

	Verb doState(void *param)
	{
		return m_onState(param);
	}

	Verb doExit(Verb verb, void *param)
	{
		return m_onExit(verb, param);
	}

	State getState()
	{
		return m_state;
	}

public:
	static void OnEnterDoNothing(void *param)
	{
	}

	static Verb OnExitDoNothing(Verb verb, void *param)
	{
		return verb;
	}

private:
	State m_state;
	OnEntry m_onEntry;
	OnState m_onState;
	OnExit m_onExit;
	Transition<Verb, State> *m_transitions;
	int m_nTransitions;
};

template<typename Verb, typename State>
struct StateMachine
{
public:
	StateMachine(SMState<Verb, State> states[], int nStates, void *param
#ifdef DEBUG_STATE_MACHINE
		, const char *name
#endif
		) :
		m_nStates(nStates),
		m_first(true),
		m_param(param)
#ifdef DEBUG_STATE_MACHINE
		, m_name(name)
#endif
	{
		m_states = new SMState<Verb, State>[m_nStates];
		for (int i = 0; i < m_nStates; i++)
			m_states[i] = states[i];
		m_current = m_states;
	}

	~StateMachine()
	{
		delete[] m_states;
		m_states = NULL;
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
				Tracef("State machine: %s, transfering from state: %s, verb: %s\n", 
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
				int i = 0;
				for (; i < m_nStates; i++)
				{
					if (m_states[i].getState() == state)
						break;
				}
#ifdef DEBUG_STATE_MACHINE
				if (i >= m_nStates)
					Tracef("Error: State machine: %s, unknown new state: %n (%s)\n", 
						m_name, 
						static_cast<int>(state), 
						StringableEnum<State>(state).ToString().c_str());
#endif
				assert(i < m_nStates);
				m_current = m_states + i;
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

	SMState<Verb, State> *current()
	{
		return m_current;
	}

private:
	SMState<Verb, State> *m_current;
	Verb m_nextVerb;
	SMState<Verb, State> *m_states;
	int m_nStates;
	bool m_first;
	void *m_param;
	const char *m_name;
};

#endif // StateMachine_h