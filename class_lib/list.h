#pragma once

#include <stdio.h>
#include <assert.h>

/*
    类成员函数中const的使用
    一般放在函数体后，形如：void fun() const;

    任何不会修改数据成员的函数都因该声明为const类型。

    如果在编写const成员函数时，不慎修改了数据成员，
    或者调用了其他非const成员函数，编译器将报错，这大大提高了程序的健壮性。
*/

/*
    operator++(int) 匹配i++
    operator++() 匹配++i
*/

template <typename ElementType, typename DataType>
struct element_iterator
{
    element_iterator() : m_pElement(NULL) {}

    void operator=(ElementType *pElement)
    {
        m_pElement = pElement;
    }

    bool operator==(ElementType *pElement) const
    {
        return (m_pElement == pElement ? true : false);
    }

    bool operator!=(ElementType *pElement) const
    {
        return (m_pElement != pElement ? true : false);
    }

    ElementType* operator++()
    {
        assert(m_pElement != NULL);
        m_pElement = m_pElement->pBackward;
        return m_pElement;
    }

    ElementType* operator++(int)
    {
        assert(m_pElement != NULL);
        ElementType* pTmp = m_pElement;
        m_pElement = m_pElement->pBackward;
        return pTmp;
    }

    DataType* operator->() const
    {
        assert(m_pElement != NULL);
        return &m_pElement->data;
    }

    DataType& operator*() const
    {
        assert(m_pElement != NULL);
        return m_pElement->data;
    }

    ElementType* m_pElement;
};

template <typename Data>
class list
{
public:
	typedef struct _Element
	{
        _Element() : pForward(NULL), pBackward(NULL) {}

		Data data;
		struct _Element *pForward;
		struct _Element *pBackward;
	}Element;

    typedef struct element_iterator<Element, Data> iterator;
	typedef struct element_iterator<Element, Data> const_iterator;

	list()
	{
		pBegin = pEnd = NULL;
		m_nSize = 0;
	}

	~list()
	{
		while(pEnd != NULL)
		{
			Element *pTmp = NULL;
			pTmp = pEnd;
			pEnd = pEnd->pForward;
			delete pTmp;
		}
	}

    Element* begin()
    {
        return pBegin;
    }

    Element* end()
    {
        return NULL;
    }

	void clear()
	{
		while(pEnd != NULL)
		{
			Element *pTmp = NULL;
			pTmp = pEnd;
			pEnd = pEnd->pForward;
			delete pTmp;
		}
		pBegin = NULL;
		m_nSize = 0;
	}

	int size()
	{
		return m_nSize;
	}

    //void list<Data>::push_back(const Data &)
    void push_back(const Data &data)
	{
		m_nSize ++;

		Element *pTmp = new Element;
		pTmp->data = data;

		if(pEnd == NULL)
		{
			pTmp->pForward = NULL;
			pTmp->pBackward = NULL;
			pBegin = pEnd = pTmp;
		}
		else
		{
			pEnd->pBackward = pTmp;
			pTmp->pForward = pEnd;
			pTmp->pBackward = NULL;
			pEnd = pTmp;
		}
	}

    void push_front(const Data &data)
	{
		m_nSize ++;

		Element *pTmp = new Element;
		pTmp->data = data;

		if(pBegin == NULL)
		{
			pTmp->pForward = NULL;
			pTmp->pBackward = NULL;
			pBegin = pEnd = pTmp;
		}
		else
		{
			pBegin->pForward = pTmp;
			pTmp->pBackward = pBegin;
			pTmp->pForward = NULL;
			pBegin = pTmp;
		}
	}

	void pop_back()
	{
		if(pEnd != NULL)
		{
			Element *pTmp = pEnd;
			if(pEnd->pForward != NULL)
			{
				pEnd = pEnd->pForward;
				pEnd->pBackward = NULL;
			}
			else
			{
				pBegin = pEnd = NULL;
			}
			delete pTmp;
			m_nSize --;
		}
	}

	void pop_front()
    {
		if(pBegin != NULL)
		{
            Element *pTmp = pBegin;
            if(pBegin->pBackward != NULL)
            {
                pBegin = pBegin->pBackward;
                pBegin->pForward = NULL;
            }
            else
            {
                pBegin = pEnd = NULL;
            }
            delete pTmp;
            m_nSize --;
		}
	}

	const Data& front()
	{
		assert(pBegin != NULL);
		return (pBegin->data);
	}

	const Data& back()
	{
		assert(pEnd != NULL);
		return (pEnd->data);
	}

	// 算法需改进
    Data* at(int index)
	{
		if(index < 0 || index >= m_nSize)
		{
			return NULL;
		}

		Element *pIndex = NULL;
		Element *pTmp = pBegin;
		while(index+1)
		{
			pIndex = pTmp;
			pTmp = pTmp->pBackward;
			index --;
		}
		return &(pIndex->data);
	}

    iterator& erase(iterator& it)
    {
        assert(it.m_pElement != NULL);

        if(it.m_pElement->pForward == NULL)
        {
            if(it.m_pElement->pBackward == NULL)
            {
                pBegin = pEnd = NULL;
                delete it.m_pElement;
                it.m_pElement = NULL;
            }
            else
            {
                pBegin = it.m_pElement->pBackward;
                pBegin->pForward = NULL;
                delete it.m_pElement;
                it.m_pElement = pBegin;
            }
        }
        else
        {
            if(it.m_pElement->pBackward == NULL)
            {
                pEnd = it.m_pElement->pForward;
                pEnd->pBackward = NULL;
                delete it.m_pElement;
                it.m_pElement = NULL;
            }
            else
            {
                it.m_pElement->pForward->pBackward = it.m_pElement->pBackward;
                it.m_pElement->pBackward->pForward = it.m_pElement->pForward;
                Element* pTmp = it.m_pElement;
                it.m_pElement = it.m_pElement->pBackward;
                delete pTmp;
            }
        }

		m_nSize--;

		return it;
    }

private:
	Element *pBegin;
	Element *pEnd;
	int m_nSize;
};
