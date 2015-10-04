#pragma once

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

   inline void RecomputeSearchStart()
   {
      // Find log base 2 of size of vector
      // Set the search start element to the middle one
      // Note that this value will be one based
      const unsigned int size = m_cSize;
      unsigned int i = 1u << 31;
      while ((!(i & size)) && (i > 1))
      {
         i >>= 1;
      }
      m_searchstart = i;
   }

   inline void AddSortedString(T * const pT)
   {
      int currentnode = m_searchstart - 1;  // Zero based
      int jumpnode = m_searchstart >> 1;

      while (1)
      {
         //assert(currentnode >= 0);

         const int strcmp = (currentnode >= m_cSize) ? 1 : pT->SortAgainst(ElementAt(currentnode));

         if (jumpnode == 0)
         {
            if (strcmp < 0)
            {
               ++currentnode; // insert new node after this one
            }
            break;
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

      InsertElementAt(pT, currentnode);
      RecomputeSearchStart();
   }

   inline int GetSortedIndex(void * const pvoid) const
   {
      int currentnode = m_searchstart - 1;  // Zero based
      int jumpnode = m_searchstart >> 1;

      while (1)
      {
         //assert(currentnode >= 0);
         int strcmp;
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
            break;

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

      return -1;
   }

   inline T *GetSortedElement(void * const pvoid) const
   {
      const int i = GetSortedIndex(pvoid);
      if (i != -1)
      {
         return ElementAt(i);
      }

      return NULL;
   }

   inline void RemoveElementAt(const int iItem)
   {
      Vector<T>::RemoveElementAt(iItem);
      RecomputeSearchStart();
   }
};
