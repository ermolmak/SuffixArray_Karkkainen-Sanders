#pragma once

#include <algorithm>
#include <cstddef>
#include <stdexcept>
#include <tuple>
#include <utility>
#include <vector>

std::vector<size_t> build_suffix_array(const std::vector<size_t> &string, size_t max_symbol);

std::vector<size_t> build_lcp_array(const std::vector<size_t> &string, const std::vector<size_t> &suffix_array);