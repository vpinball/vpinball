template<class T>
class ISort
	{
public:
	//virtual int SortStrCmp(ISortStrings *pss) = 0;
	virtual int SortAgainst(T *pT/*void *pvoid*/) = 0;
	virtual int SortAgainstValue(void *pv) = 0;
	};

template<class T>
class VectorSortString : public Vector<T>
	{
protected:
	int m_searchstart; // The node to start the binary search at

public:

	inline VectorSortString()
		{
		m_searchstart = 1;
		}

	void RecomputeSearchStart()
		{
		// Find log base 2 of size of vector
		// Set the search start element to the middle one
		// Note that this value will be one based
		unsigned int size = m_cSize;
		unsigned int i = 1<<31;
		while ((!(i & size)) && (i>1))
			{
			i >>= 1;
			}
		m_searchstart = i;
		}

	inline int AddSortedString(T *pT)
		{
		BOOL fFound = fFalse;

		int currentnode, jumpnode;								  
		int strcmp;
		
		currentnode = m_searchstart-1;  // Zero based
		jumpnode = m_searchstart >> 1;
		
		while (1)
			{
			//Assert(currentnode >= 0);

			if (currentnode >= m_cSize)
				{
				strcmp = 1;
				}
			else
				{
				strcmp = pT->SortAgainst(ElementAt(currentnode));
				}

			if (jumpnode == 0)
				{
				if (strcmp < 0)
					{
					currentnode++; // insert new node after this one
					}
				goto EndSearch;
				}

			if (strcmp == 1)
				{
				currentnode -= jumpnode;
				}
			else
				{
				currentnode += jumpnode;
				}
			jumpnode >>= 1;
			}

EndSearch:

		if (fFound == fFalse)
			{
			InsertElementAt(pT, currentnode);
			RecomputeSearchStart();
			}
		return fFound;
		}

	inline int GetSortedIndex(void *pvoid)
		{
		int currentnode, jumpnode;
		int strcmp;
		
		currentnode = m_searchstart-1;  // Zero based
		jumpnode = m_searchstart >> 1;
		
		while (1)
			{
			//Assert(currentnode >= 0);
			if (currentnode >= m_cSize)
				{
				strcmp = -1;
				}
			else
				{
				strcmp = ElementAt(currentnode)->SortAgainstValue(pvoid);

				if (strcmp == 0)
					{
					return currentnode;
					}
				}
			
			if (jumpnode == 0)
				goto NotFound;

			if (strcmp == 1)
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
		return -1;
		}

	inline T *GetSortedElement(void *pvoid)
		{			
		int i = GetSortedIndex(pvoid);
		if (i != -1)
			{
			return ElementAt(i);
			}
		
		return NULL;
		}

	inline void RemoveElementAt(int iItem)
		{
		Vector<T>::RemoveElementAt(iItem);
		RecomputeSearchStart();
		}
	};
