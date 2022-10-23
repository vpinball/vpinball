#pragma once

template<class T> class VectorProtected  // keeps only -pointers- of elements and does -not- free them afterwards! AND with a critical section for some operations (see end of file)
{
private:
   vector<void*> m_rg; // Data buffer
   CRITICAL_SECTION m_CriticalSection;

public:

   inline bool empty() { return m_rg.empty(); }
   inline void clear() { m_rg.clear(); }
   inline int size() const { return (int)m_rg.size(); }
   inline int capacity() const { return (int)m_rg.capacity(); }
   inline void resize(const size_t i) { m_rg.resize(i); }
   inline void push_back(void* const pItem) { m_rg.push_back(pItem); }

   inline int find(const void * const pvItem) const
   {
      const auto i = std::find(m_rg.begin(), m_rg.end(), pvItem);
      if (i != m_rg.end())
         return (int)(i - m_rg.begin());
      else
         return -1;
   }

   inline void insert(void * const pItem, const int iPos)
   {
      assert(iPos <= (int)m_rg.size());
      if (iPos <= (int)m_rg.size())
         m_rg.insert(m_rg.begin()+iPos,pItem);
   }

   inline void erase(const int iPos)
   {
      assert(iPos < (int)m_rg.size());
      if (iPos < (int)m_rg.size())
         m_rg.erase(m_rg.begin()+iPos);
   }

   inline void find_erase(const void * const pvItem)
   {
      const auto i = std::find(m_rg.begin(), m_rg.end(), pvItem);
      if (i != m_rg.end())
         m_rg.erase(i);
      else
      {
         assert(!"Element not found");
      }
   }

   //
   //
   //

    VectorProtected()
    {
        InitializeCriticalSection((LPCRITICAL_SECTION)&m_CriticalSection);
    }

    ~VectorProtected()
    {
       unsigned long counter = 0;
       //try to enter the critical section. If it's used by another thread try again up to 1 second
       while ((TryEnterCriticalSection((LPCRITICAL_SECTION)&m_CriticalSection) == 0) && (counter < 10))
       {
          Sleep(100);
          counter++;
       }
       if (counter < 10)
       {
           //critical section is now blocked by us leave and delete is
           //if counter=10 don't do anything
           LeaveCriticalSection((LPCRITICAL_SECTION)&m_CriticalSection);
           DeleteCriticalSection((LPCRITICAL_SECTION)&m_CriticalSection);
       }
    }

    T *ElementAt(const int iPos) const
    {
        EnterCriticalSection((LPCRITICAL_SECTION)&m_CriticalSection);
        T * const value = (T *)(m_rg[iPos]);
        LeaveCriticalSection((LPCRITICAL_SECTION)&m_CriticalSection);
        return value;
    }

    T& operator[](const int iPos)
    {
        EnterCriticalSection((LPCRITICAL_SECTION)&m_CriticalSection);
        T &value = *((T *)(m_rg[iPos]));
        LeaveCriticalSection((LPCRITICAL_SECTION)&m_CriticalSection);
        return value;
    }
    const T& operator[](const int iPos) const
    {
        EnterCriticalSection((LPCRITICAL_SECTION)&m_CriticalSection);
        const T &value = *((T *)(m_rg[iPos]));
        LeaveCriticalSection((LPCRITICAL_SECTION)&m_CriticalSection);
        return value;
    }
};
