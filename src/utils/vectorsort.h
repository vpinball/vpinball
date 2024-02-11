#pragma once

template<class T>
class VectorSortString : public vector<T>
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
      const unsigned int s = (unsigned int)vector<T>::size();
      unsigned int i = 1u << 31;
      while (!(i & s) && (i > 1))
      {
         i >>= 1;
      }
      m_searchstart = i;
   }

   inline void AddSortedString(T const pT)
   {
      int currentnode = m_searchstart - 1;  // Zero based
      int jumpnode = m_searchstart >> 1;

      while (true)
      {
         //assert(currentnode >= 0);

         const int strcmp = (currentnode >= (int)vector<T>::size()) ? 1 : pT->SortAgainst(vector<T>::data()[currentnode]);

         if (jumpnode == 0)
         {
            if (strcmp < 0)
            {
               ++currentnode; // insert new node after this one
            }
            break;
         }

         if (strcmp > 0)
         {
            currentnode -= jumpnode;
         }
         else
         {
            currentnode += jumpnode;
         }
         jumpnode >>= 1;
      }

      vector<T>::insert(vector<T>::begin() + currentnode, pT);
      RecomputeSearchStart();
   }

   inline int GetSortedIndex(const void * const pvoid) const
   {
      int currentnode = m_searchstart - 1;  // Zero based
      int jumpnode = m_searchstart >> 1;

      while (true)
      {
         //assert(currentnode >= 0);
         int strcmp;
         if (currentnode >= (int)vector<T>::size())
         {
            strcmp = -1;
         }
         else
         {
            strcmp = vector<T>::data()[currentnode]->SortAgainstValue((WCHAR*)pvoid);

            if (strcmp == 0)
            {
               return currentnode;
            }
         }

         if (jumpnode == 0)
            break;

         if (strcmp > 0)
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

   inline T GetSortedElement(const void * const pvoid) const
   {
      const int i = GetSortedIndex(pvoid);
      if (i != -1)
      {
         return vector<T>::data()[i];
      }

      return nullptr;
   }

   inline void RemoveElementAt(const int iItem)
   {
      vector<T>::erase(vector<T>::begin()+iItem);
      RecomputeSearchStart();
   }
};
