//
//  utils.hpp
//  Forge
//
//  Created by Aaron Ruel on 4/6/19.
//

#ifndef utils_h
#define utils_h

#include <chrono>
#include <algorithm>
#include <iostream>

#define BENCHMARK(code) \
auto __start = std::chrono::high_resolution_clock::now(); \
code \
auto __end = std::chrono::high_resolution_clock::now(); \
auto __result = std::chrono::duration_cast<std::chrono::duration<double>>(__end - __start); \
std::cout << "BENCH: " << __result.count() << " seconds" << std::endl;

// modified https://stackoverflow.com/a/8765699
template<size_t Size>
struct membuf: public std::streambuf {
    membuf(char * array) {
        this->setp(array, array + Size - 1);
        std::fill_n(array, Size, 0);
    }
};

template <size_t Size>
struct omemstream: virtual membuf<Size>, std::ostream {
    omemstream(char * array):
        membuf<Size>(array),
        std::ostream(this)
    {
    }
};

#endif /* utils_h */
