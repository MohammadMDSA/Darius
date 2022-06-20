#pragma once
#include "Memory.hpp"

class ReferenceCounter
{
public:

	void Increment()
	{
		m_refCount++;
	}

	int Decrement()
	{
		return --m_refCount;
	}

	int GetCount() const
	{
		return m_refCount;
	}

private:
	m_refCount{ 0 };
};

template<typename T>
class Ref
{
public:
	Ref() {}
	Ref(T* object)
		: m_object(object)
		, m_referenceCounter(D_malloc(ReferenceCounter)
	{
		m_referenceCounter->Increment();
	}

	Ref(const Ref<T>& other)
		: m_object(other.m_object)
		, m_referenceCounter(other.m_referenceCounter)
	{
		m_refereceCounter->Increment();
	}

	Ref<T>& operator=(const Ref<T>& other)
	{
		if (this != &other)
		{
			if (m_referenceCounter && m_referenceCounter->Decrement() == 0)
			{
				delete m_referenceCounter;
				delete m_object;
			}
			m_object = other.m_object;
			m_referenceCounter = other.m_referenceCounter;
			m_referenceCounter.Increment();
		}
		return *this;
	}

	T& operator*()
	{
		return *m_object;
	}

	T* operator->()
	{
		return m_object;
	}

	virtual ~Ref()
	{
		if (m_referenceCounter)
		{
			int decrementedCount = m_referenceCounter->Decrement();
			if (decrementedCount <= 0)
			{
				delete m_referenceCounter;
				delete m_object;

				m_referenceCounter = nullptr;
				m_object = nullptr;
			}
		}
	}

private:
	T* m_object{ nullptr };
	ReferenceCounter* m_referenceCounter{ nullptr };
};

template<typename T>
inline Ref<T>::Ref(T* object)
{
}
