#include "Renderer.h"
#include <iostream>

void GLClearError()
{
    while (glGetError() != GL_NO_ERROR);  //GL_NO_ERROR = 0

}

bool GLLogCall(const char* function, const char* file, int line)
{
    while (GLenum error = glGetError()) {
        std::cout << "OpenGL Error: (0x" << std::hex << error
            << std::dec << ")" << "\nFUNCTION: " << function << "\nFILE: " << file
            << "\nLINE: " << line << std::endl;
        return false;
    }
    return true;
    //check glew.h for hex errors
}