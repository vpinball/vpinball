#pragma once

#include <map>

#include "common.h"


namespace B2SLegacy {

struct CaseInsensitiveComparatorGenericDictionaryIgnoreCase {
    bool operator() (const string& a, const string& b) const noexcept {
        return std::ranges::lexicographical_compare(a.begin(), a.end(), b.begin(), b.end(),
            [](char c1, char c2) { return cLower(c1) < cLower(c2); });
    }
};

template<typename T>
class GenericDictionaryIgnoreCase final : public std::map<string, T, CaseInsensitiveComparatorGenericDictionaryIgnoreCase> {
};

}
