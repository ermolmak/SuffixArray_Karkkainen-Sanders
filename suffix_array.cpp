#include "suffix_array.hpp"

bool check_string_(const std::vector<size_t> &string, size_t max_symbol) {
    return std::all_of(string.begin(),
                       string.end(),
                       [max_symbol](size_t x) -> bool { return x >= 1 && x <= max_symbol; });
}

inline size_t get_(const std::vector<size_t> &vec, size_t n) {
    return n < vec.size() ? vec[n] : 0;
}

void radix_sort_(const std::vector<size_t> &source,
                 std::vector<size_t> &target,
                 const std::vector<size_t> &string,
                 size_t shift,
                 size_t max_symbol) {
    std::vector<size_t> counters(max_symbol + 1);
    for (size_t i : source) {
        ++counters[get_(string, i + shift)];
    }

    for (size_t i = 0, sum = 0; i < counters.size(); ++i) {
        std::swap(sum, counters[i]);
        sum += counters[i];
    }

    target.resize(source.size());
    for (size_t i : source) {
        target[counters[get_(string, i + shift)]++] = i;
    }
}

struct length_ {
    const size_t length;
    const size_t length_0;          // number of i mod 3 = 0 suffixes
    const size_t length_1;          // number of i mod 3 = 1 suffixes
    const size_t length_2;          // number of i mod 3 = 2 suffixes
    const size_t fake;              // the last "fake" i mod 3 = 1 suffix
    const size_t length_12;         // number of i mod 3 != 0 suffixes including "fake"

    length_(size_t size): length(size), length_0((size + 2) / 3), length_1((size + 1) / 3), length_2(size / 3),
                          fake(length_0 - length_1), length_12(length_0 + length_2) {
    }
};

typedef std::tuple<size_t, size_t, size_t> triple_;
typedef std::pair<std::vector<size_t>, std::vector<size_t>> pair_vector_;

std::vector<size_t> suffix_array_(const std::vector<size_t> &string, const size_t max_symbol);

void lexicographic_names_(const length_ &len,
                          const std::vector<size_t> &string,
                          const size_t max_symbol,
                          std::vector<size_t> &string_12,
                          std::vector<size_t> &suffix_array_12) {
    size_t name = 0;
    triple_ last_named(max_symbol + 1, max_symbol + 1, max_symbol + 1);
    for (size_t i : suffix_array_12) {
        triple_ current(get_(string, i), get_(string, i + 1), get_(string, i + 2));

        if (current != last_named) {
            ++name;
            last_named = current;
        }

        if (i % 3 == 1) {
            string_12[i / 3] = name;
        } else {
            string_12[i / 3 + len.length_0] = name;
        }
    }

    if (name < len.length_12) {
        suffix_array_12 = suffix_array_(string_12, name);

        for (size_t i = 0; i < len.length_12; ++i) {
            string_12[suffix_array_12[i]] = i + 1;
        }
    } else {
        for (size_t i = 0; i < len.length_12; ++i) {
            suffix_array_12[string_12[i] - 1] = i;
        }
    }
}

pair_vector_ suffix_array_12_(const length_ &len, const std::vector<size_t> &string, const size_t max_symbol) {
    std::vector<size_t> string_12;
    string_12.reserve(len.length_12);
    for (size_t i = 0; i < len.length + len.fake; ++i) {
        if (i % 3 != 0) {
            string_12.push_back(i);
        }
    }

    std::vector<size_t> suffix_array_12;
    radix_sort_(string_12, suffix_array_12, string, 2, max_symbol);
    radix_sort_(suffix_array_12, string_12, string, 1, max_symbol);
    radix_sort_(string_12, suffix_array_12, string, 0, max_symbol);

    lexicographic_names_(len, string, max_symbol, string_12, suffix_array_12);

    return std::make_pair(std::move(string_12), std::move(suffix_array_12));
}

std::vector<size_t> suffix_array_0_(const length_ &len,
                                    const std::vector<size_t> &string,
                                    const size_t max_symbol,
                                    const std::vector<size_t> &suffix_array_12) {
    std::vector<size_t> string_0;
    string_0.reserve(len.length_0);
    for (size_t i : suffix_array_12) {
        if (i < len.length_0) {
            string_0.push_back(i * 3);
        }
    }

    std::vector<size_t> suffix_array_0;
    radix_sort_(string_0, suffix_array_0, string, 0, max_symbol);

    return suffix_array_0;
}

std::vector<size_t> suffix_array_merge_(const length_ &len,
                                        const std::vector<size_t> &string,
                                        const std::vector<size_t> &string_12,
                                        const std::vector<size_t> &suffix_array_0,
                                        const std::vector<size_t> &suffix_array_12) {
    std::vector<size_t> suffix_array;
    suffix_array.reserve(len.length);

    const auto
            get_pos_12 =
            [&len](size_t i) -> size_t { return i < len.length_0 ? i * 3 + 1 : (i - len.length_0) * 3 + 2; };

    const auto pair_comp = [&string, &string_12, &len](size_t pos_0, size_t pos_12, size_t val_sa12) -> bool {
        return std::make_pair(string[pos_12], get_(string_12, val_sa12 + len.length_0))
               <= std::make_pair(string[pos_0], get_(string_12, pos_0 / 3));
    };

    const auto triple_comp = [&string, &string_12, &len](size_t pos_0, size_t pos_12, size_t val_sa12) -> bool {
        return std::make_tuple(string[pos_12], get_(string, pos_12 + 1), get_(string_12, val_sa12 - len.length_0 + 1))
               <= std::make_tuple(string[pos_0], get_(string, pos_0 + 1), get_(string_12, pos_0 / 3 + len.length_0));
    };
    size_t i0, i12;
    for (i0 = 0, i12 = len.fake; i0 < len.length_0 && i12 < len.length_12;) {
        size_t pos_0 = suffix_array_0[i0];
        size_t pos_12 = get_pos_12(get_(suffix_array_12, i12));

        if (suffix_array_12[i12] < len.length_0 ? pair_comp(pos_0, pos_12, suffix_array_12[i12]) :
            triple_comp(pos_0, pos_12, suffix_array_12[i12])) {

            suffix_array.push_back(pos_12);
            ++i12;
        } else {
            suffix_array.push_back(pos_0);
            ++i0;
        }
    }

    if (i12 == len.length_12) {
        for (; i0 < len.length_0; ++i0) {
            suffix_array.push_back(suffix_array_0[i0]);
        }
    } else if (i0 == len.length_0) {
        for (; i12 < len.length_12; ++i12) {
            suffix_array.push_back(get_pos_12(suffix_array_12[i12]));
        }
    }

    return suffix_array;
}

std::vector<size_t> suffix_array_(const std::vector<size_t> &string, const size_t max_symbol) {
    typedef std::tuple<size_t, size_t, size_t> triple;
    length_ len(string.size());

    pair_vector_ string_sa_12(suffix_array_12_(len, string, max_symbol));
    std::vector<size_t> string_12(std::move(string_sa_12.first));
    std::vector<size_t> suffix_array_12(std::move(string_sa_12.second));

    std::vector<size_t> suffix_array_0(suffix_array_0_(len, string, max_symbol, suffix_array_12));

    return suffix_array_merge_(len, string, string_12, suffix_array_0, suffix_array_12);
}

std::vector<size_t> build_suffix_array(const std::vector<size_t> &string, size_t max_symbol) {
    if (!check_string_(string, max_symbol)) {
        throw std::invalid_argument("There is a bad symbol in the string.");
    }

    return suffix_array_(string, max_symbol);
}

std::vector<size_t> build_lcp_array(const std::vector<size_t> &string, const std::vector<size_t> &suffix_array) {
    if (string.size() != suffix_array.size()) {
        throw std::invalid_argument("string and suffix_array have different sizes");
    }

    if (string.size() == 0) {
        throw std::invalid_argument("string has 0 size");
    }

    size_t size = string.size();
    std::vector<size_t> suffix_array_pos(size);
    for (size_t i = 0; i < size; ++i) {
        suffix_array_pos[suffix_array[i]] = i;
    }

    std::vector<size_t> lcp(size - 1);
    for (size_t i = 0, result = 0; i < size; ++i) {
        size_t pos = suffix_array_pos[i];

        if (pos + 1 == size) {
            continue;
        }

        if (result > 0) {
            --result;
        }

        for (; i + result < size && suffix_array[pos + 1] + result < size
               && string[i + result] == string[suffix_array[pos + 1] + result]; ++result) { }

        lcp[pos] = result;
    }

    return lcp;
}
