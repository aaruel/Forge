//
//  console.hpp
//  Forge
//
//  Created by Aaron Ruel on 4/7/19.
//

#ifndef console_h
#define console_h

// System Headers
#include <glad/glad.h>
#define GLT_IMPLEMENTATION
#include <gltext.h>
#include <iostream>
#include <sstream>
#include <kangaru/kangaru.hpp>
#include <sstream>
#include "utils.hpp"
#include "sol.hpp"

using Buffer = std::unique_ptr<char[]>;
constexpr size_t BufferSize = 256;

template<size_t SIZE = BufferSize>
class Console {
private:
    Buffer buffer = std::make_unique<char[]>(SIZE);
    omemstream<SIZE> bufOut;
    
public:
    Console() : bufOut(buffer.get()) {
        lua.set_function("world", [this]{
            bufOut << "it says hello" << std::endl;
        });
    }
    
    ~Console() {}
    
    /// Buffer getter
    const Buffer& getConsoleBuffer() const { return buffer; }
    
    /// Pass code buffer to be evaluated
    void executeLua(Buffer& input) {
        lua.script(std::string(input.get()));
    };
    
private:
    sol::state lua;
};

template<size_t SIZE = BufferSize>
struct ConsoleService : kgr::single_service<Console<SIZE>> {};

#endif /* console_h */
