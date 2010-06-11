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

	ExVectorVoid()
		{
		m_pbaseobj = 0;
		m_searchstart = 1;
		}

	ExVectorVoid(void *pobj)
		{
		m_pbaseobj = pobj;
		m_searchstart = 1;
		}

	ExVectorVoid(ExVectorVoid *xv)
		{
		m_pbaseobj = xv->m_pbaseobj;
		m_vobj=VectorVoid(xv->m_vobj);
		m_vindex=VectorVoid(xv->m_vindex);
		xv->m_searchstart=m_searchstart;
		}

	void Clone(ExVectorVoid *xv)
		{
		xv->m_pbaseobj=m_pbaseobj;
		m_vobj.Clone(&(xv->m_vobj));
		m_vindex.Clone(&(xv->m_vindex));
		xv->m_searchstart=m_searchstart;
		}

	void RecomputeSearchStart()
		{
		const unsigned int size = m_vindex.Size();
		unsigned int i = 1u<<31;
		while ((!(i & size)) && (i>1))
			{
			i >>= 1;
			}
		m_searchstart = i;
		}

	void SetBase(void *pobj)
		{
		m_pbaseobj = pobj;
		}

	// return fTrue for REPLACING, fFalse for ADDING
	BOOL AddElement(void *pobj, int iex)
		{
		BOOL fFound = fFalse;

		if (iex == xvBaseElement)
			{
			SetBase(pobj);
			return fTrue;
			}

		int currentnode, jumpnode;								  
		int currentvalue;
		
		currentnode = m_searchstart-1;  // Zero based
		jumpnode = m_searchstart >> 1;
		
		while (1)
			{
			//Assert(currentnode >= 0);

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
					fFound = fTrue;
					goto EndSearch;
					}
				}

			if (jumpnode == 0)
				{
				if (currentvalue < iex)
					{
					currentnode++; // insert new node after this one
					}
				goto EndSearch;
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

EndSearch:

		if (fFound == fFalse)
			{
			m_vobj.InsertElementAt(pobj, currentnode);
			m_vindex.InsertElementAt((void *)iex, currentnode);
			RecomputeSearchStart();
			}
		return fFound;
		}

	void RemoveAbsoluteElementAt(int iex)
		{
		m_vobj.RemoveElementAt(iex);
		m_vindex.RemoveElementAt(iex);
		RecomputeSearchStart();
		}
		
	void RemoveElementAt(void *pv)
		{
		int index;
		index = m_vindex.IndexOf(pv);
		if (index != -1)
			{
			RemoveAbsoluteElementAt(index);
			}
		}

	void *ElementAtVoid(int m)
		{
		if ((m == xvBaseElement) || (m_vindex.Size() == 0))
			return m_pbaseobj;
			
		int currentnode, jumpnode;
		int currentvalue;
		
		currentnode = m_searchstart-1;  // Zero based
		jumpnode = m_searchstart >> 1;
		
		while (1)
			{
			//Assert(currentnode >= 0);
			if (currentnode >= m_vindex.Size())
				{
				currentvalue = 0x7fffffff;
				}
			else
				{
				currentvalue = (int)m_vindex.ElementAt(currentnode);
				if (currentvalue == m)
					{
					return (m_vobj.ElementAt(currentnode));
					}
				}
			
			if (jumpnode == 0)
				goto NotFound;

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

NotFound:
		return m_pbaseobj;
		}

	// returns fTrue iff index has an exception record
	BOOL FIsExceptionItem(int j)
		{
		if (ElementAtVoid(j) != m_pbaseobj)
			return fTrue;
		return fFalse;
		}

	void RemoveExceptions()
		{
		m_vindex.RemoveAllElements();
		m_vobj.RemoveAllElements();
		RecomputeSearchStart();
		}


	// number of exceptions
	int AbsoluteSize()
		{
		return m_vobj.Size();
		}

	// provides direct access to exceptions
	inline void *AbsoluteElementAtVoid(int item)
		{
		return m_vobj.ElementAt(item);
		}
     	
	// allows direct replacement of exceptions
	void ReplaceAbsoluteElementAt(void * pT, int item)
		{
		m_vobj.ReplaceElementAt(pT, item);
		}

	// Get the Exception index from absolute index
	int RelPosFromAbsPos(int item)
		{
		return (int)m_vindex.ElementAt(item);
		}

	// a debug-thing:  free heap-alloced memory otherwise our quit-time mem checking will report leaks
	inline void Reset(void)
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
	
	inline T *ElementAt(int m)	{return (T *)ElementAtVoid(m);}
	inline T *AbsoluteElementAt(int item) {return (T *)AbsoluteElementAtVoid(item);}
	};


