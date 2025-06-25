#ifndef Observers_h
#define Observers_h

#include <LinkedList.h>
#include <atomic>

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
        m_observers.ClearAll();
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

        ObserverData &operator=(const ObserverData &other)
        {
            this->m_token = other.m_token;
            this->m_handler = other.m_handler;
            this->m_context = other.m_context;

            return *this;
        }

        bool operator==(const ObserverData &o) const
        {
            return m_token == o.m_token;
        }

        int m_token;
        Handler m_handler;
        const void *m_context;
    };

public:
	int addObserver(Handler h, const void *context = NULL)
	{
		m_observers.Insert(ObserverData(++m_token, h, context));
        return m_token;
	}

	bool removeObserver(int token)
	{
        return m_observers.Delete(ObserverData(token, NULL, NULL));
	}

	void callObservers(const EventData &data)
	{
        m_observers.ScanNodes([](const ObserverData &observer, const void *data)->bool
        {
            observer.m_handler(*(static_cast<const EventData *>(data)), observer.m_context);
            return true;
        }, &data);
	}

private:
	std::atomic<int> m_token;
	LinkedList<ObserverData> m_observers;
};

#endif // Observers_h
