/*
 * Portions of this code was derived from MFC reference material:
 *
 * https://learn.microsoft.com/en-us/cpp/mfc/reference/cexception-class
 */

class CException : public std::exception {
public:
   LPCTSTR GetText() const { return ""; };
};