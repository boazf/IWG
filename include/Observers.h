#ifndef Observers_h
#define Observers_h

#include <LinkedList.h>

template <typename EventData>
class Observers
{
public: 
	Observers() :
		m_token(0)
	{
	}

    ~Observers()
    {
        ListNode<ObserverData *> *node = m_observers.head;
        while (node != NULL)
        {
            delete node->value;
            node->value = NULL;
            node = node->next;
        }
    }

	typedef void(*Handler)(const EventData &data, const void *context);

private:
    class ObserverData
    {
    public:
        ObserverData() :
            m_token(0),
            m_handler((Handler)0),
            m_context(NULL)
        {
        }

        ObserverData(int token, Handler handler, const void *context = NULL) :
            m_token(token),
            m_handler(handler),
            m_context(context)

        {
        }

        ObserverData(const ObserverData &other)
        {
            *this = other;
        }

        ObserverData &operator=(ObserverData &other)
        {
            this->m_token = other.m_token;
            this->m_handler = other.m_handler;
            this->m_context = other.m_context;

            return *this;
        }

        int m_token;
        Handler m_handler;
        const void *m_context;
    };

public:
	int addObserver(Handler h, const void *context = NULL)
	{
        ObserverData *observer = new ObserverData(++m_token, h, context);
		m_observers.Insert(observer);
        return m_token;
	}

	bool removeObserver(int token)
	{
        ListNode<ObserverData> *node = m_observers.head;

		while (node != NULL)
		{
			if (node->m_token == token)
			{
                delete node->value;
                node->value = NULL;
				m_observers.Delete(node);
				return true;
			}
		}

		return false;
	}

	void callObservers(const EventData &data)
	{
        m_observers.ScanNodes(callObserver, (const void *)&data);
	}

private:
	int m_token;
	LinkedList<ObserverData *> m_observers;

    static bool callObserver(ObserverData * const &observer, const void *data)
    {
        observer->m_handler(*((const EventData *)data), observer->m_context);
        return true;
    }
};

#endif // Observers_h
