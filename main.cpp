#include <algorithm>
#include <iostream>
#include <string>
#include <vector>

#include "suffix_array.hpp"

int main() {
    size_t key;
    std::string str;
    std::cin >> key >> str;

    std::vector<size_t> vec_str;
    vec_str.reserve(str.size());
    for (char c : str) {
        vec_str.push_back(static_cast<size_t>(c - 'a' + 1));
    }

    std::vector<size_t> answer;
    answer.reserve(str.size());

    for (size_t i = 0; i < str.size(); ++i) {
        std::vector<size_t> current(vec_str.begin() + i, vec_str.begin() + std::min(i + key, str.size()));
        if (i + key > str.size()) {
            current.insert(current.end(), vec_str.begin(), vec_str.begin() + (i + key - str.size()));
        }

        auto sa = build_suffix_array(current, 26);
        auto lcp = build_lcp_array(current, sa);
        size_t result = 0;
        for (size_t j = 0; j + 1 < current.size(); ++j) {
            result += current.size() - sa[j] - lcp[j];
        }
        result += current.size() - sa.back();

        answer.push_back(result);
    }

    for (size_t i : answer) {
        std::cout << i << ' ';
    }

    return 0;
}