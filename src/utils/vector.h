// license:GPLv3+

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
       unsigned int counter = 0;
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


// can replace std::vector in some cases, improves especially the insertion performance at the price of increased mem usage
// can optionally call shrink to reduce mem usage again (at the price of increased temp mem usage)
template <class T> class fi_vector
{
public:
    /*class iterator
    {
    public:
        using self_type             = iterator;
        using iterator_category     = std::forward_iterator_tag;
        using difference_type       = std::ptrdiff_t;
        using value_type            = std::remove_cv_t<T>;
        using pointer               = T*;
        using reference             = T&;

        iterator(const std::vector<unsigned int>::const_iterator& it, const std::vector<T>& el)
            : m_elements(*const_cast<std::vector<T>*>(&el)), m_iter(it) {}

        T& operator *() const { return m_elements[*m_iter]; }
        T* operator->() const { return &m_elements[*m_iter]; }
        self_type& operator ++() { ++m_iter; return *const_cast<self_type*>(this); }
        self_type& operator --() { --m_iter; return *const_cast<self_type*>(this); }
        self_type operator +(const size_t n) const { return self_type(m_iter + n, m_elements); }
        self_type operator -(const size_t n) const { return self_type(m_iter - n, m_elements); }
        difference_type operator -(const self_type& it) const { return m_iter - it.m_iter; }
        bool operator ==(const self_type& it) const { return m_iter == it.m_iter; }
        bool operator !=(const self_type& it) const { return m_iter != it.m_iter; }

    private:
        std::vector<T>&                           m_elements;
        std::vector<unsigned int>::const_iterator m_iter;
    };*/

    class const_iterator
    {
    public:
        using self_type             = const_iterator;
        using iterator_category     = std::forward_iterator_tag;
        using difference_type       = std::ptrdiff_t;
        using value_type            = std::remove_cv_t<T>;
        using pointer               = T*;
        using reference             = T&;

        const_iterator(const std::vector<unsigned int>::const_iterator& it, const std::vector<T>& el)
            : m_elements(*const_cast<std::vector<T>*>(&el)), m_iter(it) {}

        const T& operator *() const { return m_elements[*m_iter]; }
        const T* operator->() const { return &m_elements[*m_iter]; }
        self_type& operator ++() { ++m_iter; return *const_cast<self_type*>(this); }
        self_type& operator --() { --m_iter; return *const_cast<self_type*>(this); }
        self_type operator +(const size_t n) const { return self_type(m_iter + n, m_elements); }
        self_type operator -(const size_t n) const { return self_type(m_iter - n, m_elements); }
        difference_type operator -(const self_type& it) const { return m_iter - it.m_iter; }
        bool operator ==(const self_type& it) const { return m_iter == it.m_iter; }
        bool operator !=(const self_type& it) const { return m_iter != it.m_iter; }

    private:
        std::vector<T>&                           m_elements;
        std::vector<unsigned int>::const_iterator m_iter;
    };

    fi_vector() : m_size(0)
    {
        m_elements.reserve(30); //!! ??
        m_indices.reserve(30);
    }

    void reserve(const size_t n)
    {
        m_elements.reserve(n);
        m_indices.reserve(n);
    }

    T& operator[](const size_t n)             { return m_elements[m_indices[n]]; }
    const T& operator[](const size_t n) const { return m_elements[m_indices[n]]; }
    T& at(const size_t n)                     { return m_elements[m_indices[n]]; }
    const T& at(const size_t n) const         { return m_elements[m_indices[n]]; }

    /* flawed, leads to double dtor call
    void erase(const size_t n)
    {
        const unsigned int deletedIndex = m_indices[n];

        m_indices.erase(m_indices.begin() + n);
        m_elements[n].~T();
        m_indices.push_back(deletedIndex);

        --m_size;
    }*/

    void insert(const size_t n, const T& value)
    {
        if (m_size == m_elements.size())
        {
            m_indices.insert(m_indices.begin() + n, (unsigned int)m_size);
            m_elements.push_back(value);
        }
        else
        {
           const unsigned int indexToInsert = m_indices.back();
           m_indices.pop_back();

           m_indices.insert(m_indices.begin() + n, indexToInsert);

           new (&m_elements[indexToInsert]) T(value);
        }

        ++m_size;
    }

    //void insert(const iterator& n, const T& value)       { insert(n - begin(), value); }
    void insert(const const_iterator& n, const T& value) { insert(n - begin(), value); }

    void push_back(const T& value)
    {
        if (m_size == m_elements.size())
        {
            m_indices.push_back((unsigned int)m_size);
            m_elements.push_back(value);
        }
        else
        {
           const unsigned int index = m_indices[m_size];

           new (&m_elements[index]) T(value);
        }

        ++m_size;
    }

    /* flawed, leads to double dtor call
    void pop_back()
    {
        --m_size;
        m_elements[m_indices[m_size]].~T();
    }*/

    //iterator begin()             { return       iterator(m_indices.begin(), m_elements); }
    const_iterator begin() const { return const_iterator(m_indices.begin(), m_elements); }

    //iterator end()               { return       iterator(m_indices.begin() + m_size, m_elements); }
    const_iterator end() const   { return const_iterator(m_indices.begin() + m_size, m_elements); }

    size_t size() const { return m_size; }
    bool empty() const  { return m_size == 0; }

    // could be used to afterwards access m_elements.data() directly for linear/continuous data access
    void shrink()
    {
        std::vector<T> elements;
        elements.reserve(m_size);
        for (size_t i = 0; i < m_size; ++i)
            elements.push_back(m_elements[m_indices[i]]);

        m_indices.clear();
        m_indices.reserve(m_size);
        for (size_t i = 0; i < m_size; ++i)
            m_indices.push_back(i);

        m_elements.clear();
        m_elements.swap(elements);
    }

    T& front()             { return at(0); }
    const T& front() const { return at(0); }

    T& back()              { return at(m_size - 1); }
    const T& back() const  { return at(m_size - 1); }

    void clear()
    {
        m_elements.clear();
        m_indices.clear();
        m_size = 0;
    }

private:
    std::vector<T>            m_elements;
    std::vector<unsigned int> m_indices;

    size_t m_size;
};
