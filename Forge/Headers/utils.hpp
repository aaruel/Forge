//
//  utils.hpp
//  Forge
//
//  Created by Aaron Ruel on 4/6/19.
//

#ifndef utils_h
#define utils_h

#include <chrono>

#define BENCHMARK(code) \
auto __start = std::chrono::high_resolution_clock::now(); \
code \
auto __end = std::chrono::high_resolution_clock::now(); \
auto __result = std::chrono::duration_cast<std::chrono::duration<double>>(__end - __start); \
std::cout << "BENCH: " << __result.count() << " seconds" << std::endl;

#endif /* utils_h */
