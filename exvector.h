#pragma once

const int xvBaseElement = -1;

class ExVectorVoid
{
private:
	void* m_pbaseobj;
	VectorVoid m_vobj;
	VectorVoid m_vindex;
	int m_searchstart; // The node to start the binary search at

public:

	inline ExVectorVoid()
	{
		m_pbaseobj = 0;
		m_searchstart = 1;
	}

	inline ExVectorVoid(void * const pobj)
	{
		m_pbaseobj = pobj;
		m_searchstart = 1;
	}

	inline ExVectorVoid(ExVectorVoid * const xv)
	{
		m_pbaseobj = xv->m_pbaseobj;
		m_vobj = VectorVoid(xv->m_vobj);
		m_vindex = VectorVoid(xv->m_vindex);
		xv->m_searchstart=m_searchstart;
	}

	inline void Clone(ExVectorVoid * const xv)
	{
		xv->m_pbaseobj=m_pbaseobj;
		m_vobj.Clone(&(xv->m_vobj));
		m_vindex.Clone(&(xv->m_vindex));
		xv->m_searchstart=m_searchstart;
	}

	inline void RecomputeSearchStart()
	{
		const unsigned int size = m_vindex.Size();
		unsigned int i = 1u<<31;
		while ((!(i & size)) && (i>1))
			{
			i >>= 1;
			}
		m_searchstart = i;
	}

	inline void SetBase(void * const pobj)
	{
		m_pbaseobj = pobj;
	}

	// return fTrue for REPLACING, fFalse for ADDING
	bool AddElement(void * const pobj, const int iex)
	{
		bool fFound = false;

		if (iex == xvBaseElement)
		{
			SetBase(pobj);
			return true;
		}

		int currentnode = m_searchstart-1;  // Zero based
		int jumpnode = m_searchstart >> 1;
		
		while(1)
		{
			//Assert(currentnode >= 0);

			int currentvalue;
			if (currentnode >= m_vindex.Size())
			{
				currentvalue = 0x7fffffff;
			}
			else
			{
				currentvalue = (int)m_vindex.ElementAt(currentnode);
				if (currentvalue == iex)
				{
					m_vobj.ReplaceElementAt(pobj, currentnode);
					fFound = true;
					break;
				}
			}

			if (jumpnode == 0)
			{
				if (currentvalue < iex)
				{
					++currentnode; // insert new node after this one
				}
				break;
			}

			if (currentvalue < iex)
			{
				currentnode += jumpnode;
			}
			else
			{
				currentnode -= jumpnode;
			}
			jumpnode >>= 1;
		}

		if (!fFound)
		{
			m_vobj.InsertElementAt(pobj, currentnode);
			m_vindex.InsertElementAt((void *)iex, currentnode);
			RecomputeSearchStart();
		}
		return fFound;
	}

	inline void RemoveAbsoluteElementAt(const int iex)
	{
		m_vobj.RemoveElementAt(iex);
		m_vindex.RemoveElementAt(iex);
		RecomputeSearchStart();
	}
		
	inline void RemoveElementAt(void * const pv)
	{
		const int index = m_vindex.IndexOf(pv);
		if (index != -1)
		{
			RemoveAbsoluteElementAt(index);
		}
	}

	void *ElementAtVoid(const int m) const
	{
		if ((m == xvBaseElement) || (m_vindex.Size() == 0))
			return m_pbaseobj;
			
		int currentnode = m_searchstart-1;  // Zero based
		int jumpnode = m_searchstart >> 1;
		
		while(1)
		{
			//Assert(currentnode >= 0);

			int currentvalue;
			if (currentnode >= m_vindex.Size())
			{
				currentvalue = 0x7fffffff;
			}
			else
			{
				currentvalue = (int)m_vindex.ElementAt(currentnode);
				if (currentvalue == m)
				{
					return m_vobj.ElementAt(currentnode);
				}
			}
			
			if (jumpnode == 0)
				break;

			if (currentvalue < m)
			{
				currentnode += jumpnode;
			}
			else
			{
				currentnode -= jumpnode;
			}
			jumpnode >>= 1;
		}

		return m_pbaseobj;
	}

	// returns true iff index has an exception record
	inline bool FIsExceptionItem(const int j) const
	{
		return (ElementAtVoid(j) != m_pbaseobj);
	}

	inline void RemoveExceptions()
	{
		m_vindex.RemoveAllElements();
		m_vobj.RemoveAllElements();
		RecomputeSearchStart();
	}

	// number of exceptions
	inline int AbsoluteSize() const
	{
		return m_vobj.Size();
	}

	// provides direct access to exceptions
	inline void *AbsoluteElementAtVoid(const int item) const
	{
		return m_vobj.ElementAt(item);
	}
     	
	// allows direct replacement of exceptions
	inline void ReplaceAbsoluteElementAt(void * const pT, const int item)
	{
		m_vobj.ReplaceElementAt(pT, item);
	}

	// Get the Exception index from absolute index
	inline int RelPosFromAbsPos(const int item) const
	{
		return (int)m_vindex.ElementAt(item);
	}

	// a debug-thing:  free heap-alloced memory otherwise our quit-time mem checking will report leaks
	inline void Reset()
	{
		m_vobj.Reset();
		m_vindex.Reset();
		RecomputeSearchStart();		
	}
};
	
template<class T> class ExVector : public ExVectorVoid
{
public:
	inline ExVector() : ExVectorVoid() {}
	inline ExVector(void *pobj) : ExVectorVoid(void *pobj) {}
	//inline ExVector(ExVector *xv) : ExVectorVoid(ExVector *xv)
	
	inline T *ElementAt(const int m) const {return (T *)ElementAtVoid(m);}
	inline T *AbsoluteElementAt(const int item) const {return (T *)AbsoluteElementAtVoid(item);}
};
