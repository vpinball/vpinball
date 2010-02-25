#pragma once
#ifndef __VECTOR_H__
#define __VECTOR_H__

static const int START_SIZE=10;
static const int GROW_SIZE=10;

class VectorVoid
{
protected:
	int		m_cMax;		// Number of elements allocated
	int		m_cSize;	// Last element used
	void 		**m_rg;		// Data buffer

public:

	VectorVoid()
		{
		m_cSize = 0;
		m_cMax = 0;
		m_rg = NULL;
		SetSize(8);
		}

	VectorVoid(int cSize)
		{
		m_cSize = 0;
		m_rg = (void **)malloc(sizeof(void *) * cSize);
		m_cMax = (!m_rg) ? 0 : cSize;
		}

	VectorVoid(VectorVoid *pvector)
		{
		int i;
		
		m_rg = (void **)malloc(sizeof(void *) * m_cMax);

		if (!m_rg)
			{
			m_cMax = 0;
			m_cSize = 0;
			return;
			}

		m_cMax = pvector->m_cMax;
		m_cSize = pvector->m_cSize;

		for (i=0;i<m_cSize;i++)	// We need this for smart pointers - they need to be ref counted
			{
			m_rg[i] = pvector->m_rg[i];
			}
		}

	~VectorVoid()
		{
		if (m_rg)
			free(m_rg);
		}
		
	// essentially a debug-only method to tell the Vector to toss its heap mem
	inline void Reset(void)
		{
		if (m_rg)
			{
			free(m_rg);
			m_rg = NULL;
			m_cSize = 0;
			m_cMax = 0;
			}
		}

	inline void Empty(void)
		{
		if (m_rg)
			free(m_rg);
		m_cSize = 0;
		m_rg = (void **)malloc(sizeof(void *) * START_SIZE);
		m_cMax = (!m_rg) ? 0 : START_SIZE;  // if !m_rg, we're actually OOM, but this will get caught the next time anything gets added to the Vector
		}

	inline int Size(void)
		{
		return m_cSize;
		}

	inline int Capacity(void)
		{
		return m_cMax;
		}
		
	inline void *ElementAt(int iItem)
		{
		return ((m_rg[iItem]));
		}

	inline void *GetArray()
		{
		return m_rg;
		}

	BOOL SetSize(int i)
		{
		if (i > m_cMax)
			{		
			void **m_rgNew;

			if (m_rg)
				{
				m_rgNew = (void **)realloc((void *)m_rg, sizeof(void *) * (i));
				}
			else
				{
				m_rgNew = (void **)malloc(sizeof(void *) * i);
				}

			if (m_rgNew == NULL)
				{
				return fFalse;
				}
			m_rg = m_rgNew;
			m_cMax = i;
			}
		return fTrue;
		}

	BOOL Extend(int cNewSize)
		{
		if (cNewSize > m_cSize)
			{
			if (!SetSize(cNewSize))
				return FALSE;
			memset(&m_rg[m_cSize], 0, sizeof(void *) * (cNewSize - m_cSize));
			m_cSize = cNewSize;
			}
		return fTrue;
		}

	BOOL Clone(VectorVoid *pvector)
		{
		int i;

		if (m_rg)
			{
			pvector->Reset();
			if ((pvector->m_rg = (void **)malloc(sizeof(void *) * m_cMax)) == NULL)
				{
				pvector->m_cMax = 0;
				pvector->m_cSize = 0;
				return fFalse;  // OOM
				}
			}
		else
			{
			pvector->m_rg = NULL;
			}
		
		pvector->m_cMax = m_cMax;
		pvector->m_cSize = m_cSize;
			
		for (i=0;i<m_cSize;i++)	// We need this for smart pointers - they need to be ref counted
			{
			pvector->m_rg[i] = m_rg[i];
			}
			
		return fTrue;
		}

	int LastIndexOf(void *pT)
		{
		int i;
		
		if (!m_rg)
			return -1;

		for (i=m_cSize-1;i >= 0; i--)
			if (m_rg[i] == pT)
				break;
		return i;
		}

	int IndexOf(void *pvItem)
		{
		int i;

		if (m_rg == NULL)
			return -1;

		for (i=0;i<m_cSize;i++)
			{
			if (pvItem == m_rg[i])
				return i;
			}
		return -1;
		}

	// returns current size of the Vector, or -1 on failure (OOM)
	int AddElement(void *pItem)
		{
		if(m_cSize == m_cMax)
			{
			if (!SetSize(m_cSize*2))
				return -1;
			}

		m_rg[m_cSize++] = pItem;

		return(m_cSize - 1);
		}

	// fTrue for success, fFalse if failure (OOM)
	BOOL InsertElementAt(void *pItem, int iPos)
		{					
		if(m_cSize == m_cMax)
			{		
			if (!SetSize(m_cSize+GROW_SIZE))
				return fFalse;
			}

		if (m_cSize!=iPos)
			{
			memcpy(m_rg+iPos+1,m_rg+iPos,sizeof(void *) * (m_cSize-iPos));
			}

		m_rg[iPos] = pItem;
		m_cSize++;
		return fTrue;
		}

	inline void ReplaceElementAt(void *pItem, int iPos)
		{
		if (!m_rg)
			return;

		m_rg[iPos] = pItem;
		}

	inline void RemoveElementAt(int iItem)
		{
		if (!m_rg)
			return;

		memcpy(m_rg+iItem,m_rg+iItem+1,sizeof(void *) * (m_cSize-iItem-1));
		m_cSize--;
		}

	inline void RemoveElement(void *pvItem)
		{
		if (!m_rg)
			return;

		int i = IndexOf(pvItem);
		if (i>=0)
			RemoveElementAt(i);
		}

	void RemoveAllElements(void)
		{
		//int i;
		if (!m_rg)
			return;
		//for (i=0;i<m_cSize;i++) 	// We need this for smart pointers - they need to be ref counted
			//{
			//m_rg[i] = NULL;
			//}
		m_cSize = 0;
		}

    // LIFO stack support
    inline int Push(void *pItem)
    {
        return AddElement(pItem);
    }

    inline BOOL Pop(void **ppItem)
		{
        if(m_cSize)
        {
            *ppItem = m_rg[--m_cSize];
            m_rg[m_cSize] = NULL;
			return TRUE;
        }

        return FALSE;
		}

	BOOL Top(void **ppItem)
		{
		if(m_cSize)
			{
			*ppItem = m_rg[m_cSize-1];
			return TRUE;
			}
		return FALSE;
		}
	};
	
template<class T> class Vector : public VectorVoid
	{
public:
	Vector() : VectorVoid() {}
	Vector(int cSize) : VectorVoid(cSize) {}

	inline T *ElementAt(int iItem)
		{
		return (T *) ((m_rg[iItem]));
		}	
	};

template<class T> class VectorInt : public VectorVoid
	{
public:
	VectorInt() : VectorVoid() {}
	VectorInt(int cSize) : VectorVoid(cSize) {}

	inline T ElementAt(int iItem)
		{
		return (T) ((m_rg[iItem]));
		}

	inline int AddElement(T item)
		{
		return VectorVoid::AddElement((void *)item);
		}

	int IndexOf(int pvItem)
		{
		int i;

		if (m_rg == NULL)
			return -1;

		for (i=0;i<m_cSize;i++)
			{
			if ((void *)pvItem == m_rg[i])
				return i;
			}
		return -1;
		}

	inline void RemoveElement(int pvItem)
		{
		if (!m_rg)
			return;

		int i = IndexOf(pvItem);
		if (i>=0)
			RemoveElementAt(i);
		}
	};

typedef Vector<void> VectorPV;
typedef VectorPV * PVectorPV;

template<class T> 
class VecClean : public Vector<T>
	{
public:
	VecClean(){};
	~VecClean()
		{
		int cSize = m_cSize;
		while (--cSize>=0)
			{
			T * pElem = ElementAt(cSize);
			delete pElem;
			}
		}
	};

#endif





