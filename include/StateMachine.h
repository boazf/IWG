#ifndef StateMachine_h
#define StateMachine_h

#include <Arduino.h>
#include <assert.h>
#include <Common.h>

#define TRANSITIONS(a) a, NELEMS(a)

template<typename Verb, typename StateName>
class Transition
{
public:
	Transition()
	{
	}

	Transition(Verb verb, StateName state)
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
	StateName m_state;
};

template<typename Verb, typename StateName>
class SMState
{
public:
	typedef void (*OnEntry)(void *param);
	typedef Verb(*OnState)(void *param);
	typedef Verb(*OnExit)(Verb verb, void *param);

public:
	SMState() :
		m_state((StateName)0),
		m_onEntry(0),
		m_onState(0),
		m_onExit(0),
		m_transitions(NULL),
		m_nTransitions(0)
	#ifdef DEBUG_STATE_MACHINE
		, m_name("")
	#endif
	{
	}

	~SMState()
	{
		delete[] m_transitions;
		m_transitions = NULL;
	}

	SMState(StateName state, OnEntry onEntry, OnState onState, OnExit onExit, const Transition<Verb, StateName> *transitions, int nTransitions
	#ifdef DEBUG_STATE_MACHINE
		, const char *name
	#endif
		) :
		m_state(state),
		m_onEntry(onEntry),
		m_onState(onState),
		m_onExit(onExit),
		m_nTransitions(nTransitions)
	#ifdef DEBUG_STATE_MACHINE
		, m_name(name)
	#endif
	{
		m_transitions = new Transition<Verb, StateName>[m_nTransitions];
		for (int i = 0; i < m_nTransitions; i++)
			m_transitions[i] = transitions[i];
	}

    SMState(const SMState<Verb, StateName> &other)
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
#ifdef DEBUG_STATE_MACHINE
		m_name = other.m_name;
#endif
		m_transitions = new Transition <Verb, StateName>[m_nTransitions];
		for (int i = 0; i < m_nTransitions; i++)
			m_transitions[i] = other.m_transitions[i];

		return *this;
	}

#ifdef DEBUG_STATE_MACHINE
	const char *getStateName()
	{
		return m_name;
	}
#endif // DEBUG_STATE_MACHINE

	StateName PerformTransition(Verb verb)
	{
		for (int i = 0; i < m_nTransitions; i++)
		{
			if (m_transitions[i].m_verb == verb)
				return m_transitions[i].m_state;
		}
#ifdef DEBUG_STATE_MACHINE
		Tracef("Error: Transition not found, state=%s, verb=%d\n", m_name, (int)verb);
#endif		
		assert(false);
		return (StateName)0;
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

	StateName State()
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
	StateName m_state;
	OnEntry m_onEntry;
	OnState m_onState;
	OnExit m_onExit;
	Transition<Verb, StateName> *m_transitions;
	int m_nTransitions;
#ifdef DEBUG_STATE_MACHINE
	const char *m_name;
#endif	
};

template<typename Verb, typename StateName>
struct StateMachine
{
public:
	StateMachine(SMState<Verb, StateName> states[], int nStates, void *param
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
		m_states = new SMState<Verb, StateName>[m_nStates];
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
			if (m_nextVerb != (Verb)0)
			{
#ifdef DEBUG_STATE_MACHINE
				Tracef("State machine: %s, exiting state: %s\n", m_name, m_current->getStateName());
#endif
				Verb verb = m_current->doExit(m_nextVerb, m_param);
#ifdef DEBUG_STATE_MACHINE
				Tracef("State machine: %s, transfering from state: %s, verb: %d\n", m_name, m_current->getStateName(), (int)verb);
#endif
				StateName state = m_current->PerformTransition(verb);
#ifdef DEBUG_STATE_MACHINE
				Tracef("State machine: %s, new state: %d\n", m_name, (int)state);
#endif
				int i = 0;
				for (; i < m_nStates; i++)
				{
					if (m_states[i].State() == state)
						break;
				}
#ifdef DEBUG_STATE_MACHINE
				if (i >= m_nStates)
					Tracef("Error: State machine: %s, unknown new state: %d\n", m_name, (int)state);
#endif
				assert(i < m_nStates);
				m_current = m_states + i;
#ifdef DEBUG_STATE_MACHINE
				Tracef("State machine: %s, entering  state: %s\n", m_name, m_current->getStateName());
#endif
				m_current->doEnter(m_param);
			}
		}
		else
		{
#ifdef DEBUG_STATE_MACHINE
			Tracef("State machine: %s, entering starting state: %s\n", m_name, m_current->getStateName());
#endif
			m_current->doEnter(m_param);
			m_first = false;
		}
		Verb newVerb = m_current->doState(m_param);
		ApplyVerb(newVerb);
	}

	SMState<Verb, StateName> *current()
	{
		return m_current;
	}

private:
	SMState<Verb, StateName> *m_current;
	Verb m_nextVerb;
	SMState<Verb, StateName> *m_states;
	int m_nStates;
	bool m_first;
	void *m_param;
	const char *m_name;
};

#endif // StateMachine_h