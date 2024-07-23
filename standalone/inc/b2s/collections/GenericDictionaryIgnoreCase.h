#pragma once

#include <map>

struct CaseInsensitiveComparatorGenericDictionaryIgnoreCase {
    bool operator() (const string& a, const string& b) const noexcept {
        return std::lexicographical_compare(a.begin(), a.end(), b.begin(), b.end(), 
            [](unsigned char c1, unsigned char c2) { return std::tolower(c1) < std::tolower(c2); });
    }
};

template<typename T>
class GenericDictionaryIgnoreCase : public std::map<string, T, CaseInsensitiveComparatorGenericDictionaryIgnoreCase> {
};