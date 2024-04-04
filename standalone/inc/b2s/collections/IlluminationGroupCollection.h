#pragma once

#include <map>

class B2SPictureBox;

struct CaseInsensitiveComparatorIlluminationGroupCollection {
    bool operator() (const string& a, const string& b) const noexcept {
        return std::lexicographical_compare(a.begin(), a.end(), b.begin(), b.end(), 
            [](unsigned char c1, unsigned char c2) { return std::tolower(c1) < std::tolower(c2); });
    }
};

class IlluminationGroupCollection : public std::map<string, vector<B2SPictureBox*>, CaseInsensitiveComparatorIlluminationGroupCollection>
{
public:
   void Add(B2SPictureBox* pPictureBox);
};