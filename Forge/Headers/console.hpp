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
#include <sol/sol.hpp>

using Buffer = std::unique_ptr<char[]>;
inline auto make_buffer = std::make_unique<char[]>;
constexpr size_t BufferSize = 256;

template<size_t SIZE = BufferSize>
class Console {
private:
    Buffer buffer = make_buffer(SIZE);
    omemstream<SIZE> bufOut;
    
public:
    Console() : bufOut(buffer.get()) {
        lua.open_libraries(sol::lib::base, sol::lib::package);
        
        /// Sanity check
        lua.set_function("world", [this]{
            bufOut << "it says hello" << std::endl;
        });
        
        /// Override the print function to redirect output
        lua.set_function("print", [this](sol::variadic_args va){
            for (auto v : va) {
                std::string arg = v;
                bufOut << arg << std::endl;
            }
        });
    }
    
    ~Console() {}
    
    /// Buffer getter
    const Buffer& getConsoleBuffer() const { return buffer; }
    
    /// Pass code buffer to be evaluated
    void executeLua(Buffer& input) {
        // Since language errors are exceptions, might as well catch them
        try {
            lua.script(std::string(input.get()));
        }
        catch(const std::exception& e) {
            std::cout << e.what() << std::endl;
        }
    };
    
private:
    sol::state lua;
};

template<size_t SIZE = BufferSize>
struct ConsoleService : kgr::single_service<Console<SIZE>> {};

#endif /* console_h */
