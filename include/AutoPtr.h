#ifndef AutoPtr_h
#define AutoPtr_h

#include <assert.h>

template <typename T>
class AutoPtr
{
public:
    AutoPtr() :
        m_ptr(NULL)
    {
    }

    AutoPtr(T *ptr) :
        m_ptr(ptr)
    {
    }

    AutoPtr(AutoPtr<T> &p)
    {
        m_ptr = p.Detach();
    }

    ~AutoPtr()
    {
        Free();
    }

	template<typename TSrc>
	AutoPtr<T>& operator=(AutoPtr<TSrc>& p)
	{
		if (m_ptr == p.m_ptr)
		{
			// This means that two AutoPtrs of two different types had the same m_p in them
			// which is never correct
			assert(false);
		}
		else
		{
			Free();
			Attach(p.Detach());  // Transfer ownership
		}
		return(*this);
	}

	AutoPtr<T>& operator=(AutoPtr<T>& p) throw()
	{
		if (*this == p)
		{
			if (this != &p)
			{
				// If this assert fires, it means you attempted to assign one AutoPtr to another when they both contained
				// a pointer to the same underlying object. This means a bug in your code, since your object will get
				// double-deleted.
				// For safety, we are going to detach the other AutoPtr to avoid a double-free. Your code still
				// has a bug, though.
				p.Detach();
			}
			else
			{
				// Alternatively, this branch means that you are assigning a AutoPtr to itself, which is
				// pointless but permissible

				// nothing to do
			}
		}
		else
		{
			Free();
			Attach(p.Detach());  // Transfer ownership
		}
		return(*this);
	}

    void Attach(T *ptr)
    {
        Free();
        m_ptr = ptr;
    }

    T *Detach()
    {
        T *temp = m_ptr;
        m_ptr = NULL;
        return temp;
    }

	bool operator!=(AutoPtr<T>& p) const
	{
		return !operator==(p);
	}

	bool operator==(AutoPtr<T>& p) const
	{
		return m_ptr == p.m_ptr;
	}

    operator T *() const
    {
        return m_ptr;
    }

    T *operator ->() const
    {
        return m_ptr;
    }

    void Free()
    {
        delete m_ptr;
        m_ptr = NULL;
    }

private:
    T *m_ptr;
};

#endif // AutoPtr_h