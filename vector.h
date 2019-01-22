#pragma once

#define START_SIZE 32
#define GROW_SIZE  32

template<class T> class VectorProtected  // keeps only -pointers- of elements and does -not- free them afterwards! AND with a critical section
{
private:
   int		m_cMax;		// Number of elements allocated
   int		m_cSize;	// Last element used
   void 	**m_rg;		// Data buffer

public:

   // essentially a debug-only method to tell the Vector to toss its heap mem
   inline void Reset()
   {
      if (m_rg)
      {
         free(m_rg);
         m_rg = NULL;
         m_cSize = 0;
         m_cMax = 0;
      }
   }

   inline void Empty()
   {
      if (m_rg)
         free(m_rg);
      m_cSize = 0;
      m_rg = (void **)malloc(sizeof(void *) * START_SIZE);
      m_cMax = (!m_rg) ? 0 : START_SIZE; // if !m_rg, we're actually OOM, but this will get caught the next time anything gets added to the Vector
   }

   inline int Size() const { return m_cSize; }
   inline int size() const { return m_cSize; } // for compatibility with std::vector

   inline int Capacity() const
   {
      return m_cMax;
   }

   inline bool SetSize(const int i)
   {
      if (i > m_cMax)
      {
         void **m_rgNew;

         if (m_rg)
         {
            m_rgNew = (void **)realloc((void *)m_rg, sizeof(void *) * i);
         }
         else
         {
            m_rgNew = (void **)malloc(sizeof(void *) * i);
         }

         if (m_rgNew == NULL)
         {
            return false;
         }

         m_rg = m_rgNew;
         m_cMax = i;
      }

      return true;
   }

   inline bool Extend(const int cNewSize)
   {
      if (cNewSize > m_cSize)
      {
         if (!SetSize(cNewSize))
            return false;

         ZeroMemory(&m_rg[m_cSize], sizeof(void *) * (cNewSize - m_cSize));
         m_cSize = cNewSize;
      }

      return true;
   }

   inline bool Clone(VectorProtected * const pvector) const
   {
      pvector->Reset();

      if (m_rg)
      {
         if ((pvector->m_rg = (void **)malloc(sizeof(void *) * m_cMax)) == NULL)
            return false;  // OOM

         pvector->m_cMax = m_cMax;
         pvector->m_cSize = m_cSize;

         for (int i = 0; i < m_cSize; ++i) // We need this for smart pointers - they need to be ref counted
            pvector->m_rg[i] = m_rg[i];
      }

      return true;
   }

   inline int IndexOf(const void * const pvItem) const
   {
      if (m_rg)
         for (int i = 0; i < m_cSize; ++i)
            if (pvItem == m_rg[i])
               return i;

      return -1;
   }

   // returns current size of the Vector, or -1 on failure (OOM)
   inline int AddElement(void * const pItem)
   {
      if (m_cSize == m_cMax)
      {
         if (!SetSize(m_cSize + GROW_SIZE))
            return -1;
      }

      const int oldsize = m_cSize;
      m_rg[m_cSize++] = pItem;

      return oldsize;
   }

   // true for success, false if failure (OOM)
   inline bool InsertElementAt(void * const pItem, const int iPos)
   {
      if (m_cSize == m_cMax)
      {
         if (!SetSize(m_cSize + GROW_SIZE))
            return false;
      }

      if (m_cSize != iPos)
      {
         memmove(m_rg + iPos + 1, m_rg + iPos, sizeof(void *) * (m_cSize - iPos));
      }

      m_rg[iPos] = pItem;
      ++m_cSize;
      return true;
   }

   inline void ReplaceElementAt(void * const pItem, const int iPos)
   {
      if (!m_rg)
         return;

      m_rg[iPos] = pItem;
   }

   inline void RemoveElementAt(const int iItem)
   {
      if (!m_rg)
         return;

      memmove(m_rg + iItem, m_rg + iItem + 1, sizeof(void *) * (m_cSize - iItem - 1));
      --m_cSize;
   }

   inline void RemoveElement(void * const pvItem)
   {
      if (!m_rg)
         return;

      const int i = IndexOf(pvItem);
      if (i >= 0)
         RemoveElementAt(i);
   }

   inline void RemoveAllElements()
   {
      if (!m_rg)
         return;
      //for (int i=0; i<m_cSize; ++i) 	// We need this for smart pointers - they need to be ref counted
      //{
      //m_rg[i] = NULL;
      //}
      m_cSize = 0;
   }

   // LIFO stack support
   inline int Push(void * const pItem)
   {
      return AddElement(pItem);
   }

   inline bool Pop(void ** const ppItem)
   {
      if (m_cSize)
      {
         *ppItem = m_rg[--m_cSize];
         m_rg[m_cSize] = NULL;
         return true;
      }

      return false;
   }

   inline bool Top(void ** const ppItem) const
   {
      if (m_cSize)
      {
         *ppItem = m_rg[m_cSize - 1];
         return true;
      }

      return false;
   }

   //
   //
   //

    VectorProtected()
    {
        InitializeCriticalSection((LPCRITICAL_SECTION)&hCriticalSection);

        m_cSize = 0;
        m_rg = (void **)malloc(sizeof(void *) * START_SIZE);
        m_cMax = (!m_rg) ? 0 : START_SIZE;  // if !m_rg, we're actually OOM, but this will get caught the next time anything gets added to the Vector
    }

    VectorProtected(const int reservedSize)
    {
        InitializeCriticalSection((LPCRITICAL_SECTION)&hCriticalSection);

        m_cSize = 0;
        m_rg = (void **)malloc(sizeof(void *) * reservedSize);
        m_cMax = (!m_rg) ? 0 : reservedSize;
    }

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

       if (m_rg)
          free(m_rg);
    }

    typedef T value_type;

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
