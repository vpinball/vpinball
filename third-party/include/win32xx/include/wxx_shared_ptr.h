// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF
// ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED
// TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
// PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT
// SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR
// ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
// ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE
// OR OTHER DEALINGS IN THE SOFTWARE.
//
// This software was developed from code available in the public domain
// and has no copyright.


// About Shared_Ptr:
// Shared_Ptr wraps a reference-counted smart pointer around a dynamically
// allocated object. Unlike auto_ptr, the Shared_Ptr can be used as a smart
// pointer for objects stored in containers like std::vector. Do not use
// Shared_Ptr (or shared_ptr or auto_ptr) for dynamically allocated arrays.
// See below for advice on how to wrap dynamically allocated arrays in a
// vector.
//
// The next standard of C++ will also contain a shared_ptr. Some modern
// compilers already have a shared_ptr available as std::tr1::shared_ptr. If
// your compiler already provides a shared_ptr, or if you have Boost, you
// should use that smart pointer instead. This class has been provided for
// those users who don't have easy access to an "official" shared_ptr.
// Note that this class is "Shared_Ptr", a slightly different name to the
// future "shared_ptr" to avoid naming conflicts.

// Advantages of Shared_Ptr (or shared_ptr where available):
//  - Shared_Ptr can be safely copied. This makes then suitable for containers.
//  - Shared_Ptr automatically calls delete for the wrapped pointer when
//     its last copy goes out of scope.
//  - Shared_Ptr simplifies exception safety.
//
// Without smart pointers, it can be quite challenging to ensure that every
// dynamically allocated pointer (i.e. use of new) is deleted in the event of
// all possible exceptions. In addition to the exceptions we throw ourselves,
// "new" itself will throw an exception it it fails, as does the STL (Standard
// Template Library which includes vector and string). Without smart pointers
// we often need to resort to additional try/catch blocks simply to avoid
// memory leaks when exceptions occur.

// Examples:
//  Shared_Ptr<CWnd> w1(new CWnd);
//   or
//  Shared_Ptr<CWnd> w1 = new CWnd;
//   or
//  typedef Shared_Ptr<CWnd> CWndPtr;
//  CWndPtr w1 = new CWnd;
//   or
//  typedef Shared_Ptr<CWnd> CWndPtr;
//  CWndPtr w1(new CWnd);
//
//  And with a vector
//  typedef Shared_Ptr<CWnd> CWndPtr;
//  std::vector<CWndPtr> MyVector;
//  MyVector.push_back(new CWnd);
//   or
//  typedef Shared_Ptr<CWnd> CWndPtr;
//  CWnd* pWnd = new CWnd;
//  std::vector<CWndPtr> MyVector;
//  MyVector.push_back(pWnd);
//

// How to handle dynamically allocated arrays:
// While we could create a smart pointer for arrays, we don't need to because
// std::vector already handles this for us. Consider the following example:
//    int length = ::GetWindowTextLength(wnd);
//    pTChar = new TCHAR[length+1];
//    ZeroMemory(pTChar, (length+1)*sizeof(TCHAR));
//    ::GetWindowText(wnd, m_pTChar, length);
//    ....
//    delete[] pTChar;
//
// This can be improved by using a vector instead of an array
// This works because the memory in a vector is always contiguous.
// Note that this is NOT always true of std::string.
//    int length = ::GetWindowTextLength(wnd);
//    std::vector<TCHAR> vTChar( length+1, _T('\0') );
//    TCHAR* pTCharArray = &vTChar.front();
//    ::GetWindowText(wnd, pTCharArray, length+1);
//
// Alternatively we could use a CString.
//    int length = ::GetWindowTextLength(wnd);
//    CString str;
//    ::GetWindowText(wnd, str.GetBuffer(length), length);
//    str.ReleaseBuffer();
//


// Summing up:
// Ideally, calls to "new" should be wrapped in some sort of smart pointer
// wherever possible. This eliminates the possibility of memory leaks,
// particularly in the event of exceptions. It also eliminates the
// need for delete in user's code.


#ifndef _WIN32XX_SHARED_PTR_
#define _WIN32XX_SHARED_PTR_


#include <cassert>
#include <algorithm>        // For std::swap
#include <WinSock2.h>       // must include before windows.h
#include <Windows.h>        // For InterlockedIncrement and InterlockedDecrement

#ifdef __BORLANDC__
  #pragma option -w-8027    // function not expanded inline
#endif


namespace Win32xx
{

    // Shared_Ptr is a smart pointer suitable for elements in a vector.
    // Shared_Ptr behaves much like the shared_ptr introduced in C++11
    template <class T>
    class Shared_Ptr
    {
    public:
        Shared_Ptr() : m_ptr(0), m_count(0) { }
        Shared_Ptr(T* p) : m_ptr(p), m_count(0)
        {
            try
            {
                if (m_ptr) m_count = new long(0);
                inc_ref();
            }
            // catch the unlikely event of 'new long(0)' throwing an exception
            catch (const std::bad_alloc&)
            {
                delete m_ptr;
                throw;
            }
        }
        Shared_Ptr(const Shared_Ptr& rhs) : m_ptr(rhs.m_ptr), m_count(rhs.m_count) { inc_ref(); }
        ~Shared_Ptr()
        {
            if (m_count && dec_ref() == 0)
            {
                // Note: This code doesn't handle a pointer to an array.
                //  We would need delete[] m_ptr to handle that.
                delete m_ptr;
                delete m_count;
            }
        }

        T* get() const { return m_ptr; }
        long use_count() const { return m_count? *m_count : 0; }
        bool unique() const { return (m_count && (*m_count == 1)); }

        void swap(Shared_Ptr& rhs)
        {
           std::swap(m_ptr, rhs.m_ptr);
           std::swap(m_count, rhs.m_count);
        }

        Shared_Ptr& operator=(const Shared_Ptr& rhs)
        {
             Shared_Ptr tmp(rhs);
             this->swap(tmp);
             return *this;
        }

        T* operator->() const
        {
            assert(m_ptr);
            return m_ptr;
        }

        T& operator*() const
        {
            assert (m_ptr);
            return *m_ptr;
        }

        bool operator== (const Shared_Ptr& rhs) const
        {
            return ( *m_ptr == *rhs.m_ptr);
        }

        bool operator!= (const Shared_Ptr& rhs) const
        {
            return ( *m_ptr != *rhs.m_ptr);
        }

        bool operator< (const Shared_Ptr& rhs) const
        {
            return ( *m_ptr < *rhs.m_ptr );
        }

        bool operator> (const Shared_Ptr& rhs) const
        {
            return ( *m_ptr > *rhs.m_ptr );
        }

    private:
        void inc_ref()
        {
            if (m_count)
                InterlockedIncrement(m_count);
        }

        int  dec_ref()
        {
            assert (m_count);
            return InterlockedDecrement(m_count);
        }

        T* m_ptr;
        long* m_count;
    };

}

#endif  // _WIN32XX_SHARED_PTR_
