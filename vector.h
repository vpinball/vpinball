#pragma once

template<class T> class VectorProtected  // keeps only -pointers- of elements and does -not- free them afterwards! AND with a critical section
{
private:
   std::vector<void*> m_rg; // Data buffer

public:

   inline bool empty() { return m_rg.empty(); }
   inline void clear() { m_rg.clear(); }
   inline int size() const { return (int)m_rg.size(); }
   inline int capacity() const { return (int)m_rg.capacity(); }
   inline void resize(const int i) { m_rg.resize(i); }
   inline void push_back(void* const pItem) { m_rg.push_back(pItem); }

   inline int IndexOf(const void * const pvItem) const
   {
      for (int i = 0; i < (int)m_rg.size(); ++i)
         if (pvItem == m_rg[i])
            return i;

      return -1;
   }

   inline void InsertElementAt(void * const pItem, const int iPos)
   {
      const int cSize = (int)m_rg.size();
      m_rg.resize(cSize + 1);
      assert(iPos < (int)m_rg.size());

      if (cSize != iPos)
         memmove(m_rg.data() + iPos + 1, m_rg.data() + iPos, sizeof(void *) * (cSize - iPos));

      m_rg[iPos] = pItem;
   }

   inline void ReplaceElementAt(void * const pItem, const int iPos)
   {
      assert(iPos < (int)m_rg.size());
      if (iPos < (int)m_rg.size())
         m_rg[iPos] = pItem;
   }

   inline void RemoveElementAt(const int iItem)
   {
      assert(iItem < (int)m_rg.size());
      if (iItem < (int)m_rg.size())
      {
         memmove(m_rg.data() + iItem, m_rg.data() + iItem + 1, sizeof(void *) * ((int)m_rg.size() - iItem - 1));
         m_rg.resize(m_rg.size() - 1);
      }
   }

   inline void RemoveElement(void * const pvItem)
   {
      const int i = IndexOf(pvItem);
      if (i >= 0)
         RemoveElementAt(i);
   }

   //
   //
   //

    VectorProtected()
    {
        InitializeCriticalSection((LPCRITICAL_SECTION)&hCriticalSection);
    }

    /*VectorProtected(const int reservedSize)
    {
        InitializeCriticalSection((LPCRITICAL_SECTION)&hCriticalSection);

        m_rg.reserve(reservedSize);
    }*/

    ~VectorProtected()
    {
       unsigned long counter = 0;
       //try to enter the critical section. If it's used by another thread try again up to 1 second
       while ((TryEnterCriticalSection((LPCRITICAL_SECTION)&hCriticalSection) == 0) && (counter<10))
       {
          Sleep(100);
          counter++;
       }
       if (counter < 10)
       {
           //critical section is now blocked by us leave and delete is
           //if counter=10 don't do anything
           LeaveCriticalSection((LPCRITICAL_SECTION)&hCriticalSection);
           DeleteCriticalSection((LPCRITICAL_SECTION)&hCriticalSection);
       }
    }

    T *ElementAt(const int iItem) const
    {
        EnterCriticalSection((LPCRITICAL_SECTION)&hCriticalSection);
        T *value = (T *)(m_rg[iItem]);
        LeaveCriticalSection((LPCRITICAL_SECTION)&hCriticalSection);
        return value;
    }

    T& operator[](const int iItem)
    {
        EnterCriticalSection((LPCRITICAL_SECTION)&hCriticalSection);
        T &value = *((T *)(m_rg[iItem]));
        LeaveCriticalSection((LPCRITICAL_SECTION)&hCriticalSection);
        return value;
    }
    const T& operator[](const int iItem) const
    {
        EnterCriticalSection((LPCRITICAL_SECTION)&hCriticalSection);
        const T &value = *((T *)(m_rg[iItem]));
        LeaveCriticalSection((LPCRITICAL_SECTION)&hCriticalSection);
        return value;
    }
private:
    CRITICAL_SECTION hCriticalSection;
};
