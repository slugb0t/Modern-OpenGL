//include Glew
#include <GL/glew.h>
//Include GLFW
#include <GLFW/glfw3.h>

#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <chrono>

#include "Renderer.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"

struct ShaderProgramSource
{
    std::string VertexSource;
    std::string FragmentSouce;
};

static ShaderProgramSource ParseShader(const std::string& filepath) 
{
    enum class ShaderType
    {
        NONE = -1, VERTEX = 0, FRAGMENT = 1
    };
    //CHANGE CLASS TO STATIC TO BE ABLE TO CALL FUNCTIONS OUTSIDE OF SCOPE(?)

    //opens the file using fstream
    //sstream needed for getline as well
    std::ifstream stream(filepath);
    std::string line;
    std::stringstream ss[2];
    ShaderType type {ShaderType::NONE};

    while (getline(stream, line))
    {
        if (line.find("#shader") != std::string::npos)
        {
            if (line.find("vertex") != std::string::npos)
            {
                //set mode to vertex
                type = ShaderType::VERTEX;
            }
            else if (line.find("fragment") != std::string::npos)
            {
                //set mode to fragment
                type = ShaderType::FRAGMENT;
            }
        }
        else
        {
            ss[(int)type] << line << '\n';
        }
    }

    return { ss[0].str(), ss[1].str() };
}

static unsigned int CompileShader(unsigned int type, const std::string& source)
{
    unsigned int id { glCreateShader(type) };
    const char* src { source.c_str() };
    GLCall(glShaderSource(id, 1, &src, nullptr));
    GLCall(glCompileShader(id));
   
    //glGetShaderiv(id, GL_COMPILE_STATUS, &result);
    //if(result)

    //TODO:ERROR HANDLING
    //query to check if ID is good
    int result;
    GLCall(glGetShaderiv(id, GL_COMPILE_STATUS, &result));
    //std::cout << (type == GL_VERTEX_SHADER ? "vertex" : "fragment") << "shader compile status" << result << std::endl;
    if (result == GL_FALSE)
    {
        //print the error message
        int length;
        GLCall(glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length));
        //char message[length];

        char* message{ (char*)_malloca(length * sizeof(char)) };
        //allocate on the stack dynamically
        GLCall(glGetShaderInfoLog(id, length, &length, message));
        std::cout << "FAILED TO COMPILE SHADER:" << 
            (type == GL_VERTEX_SHADER ? "vertex" : "fragment") << " shader" << std::endl;
        std::cout << message << std::endl;
        GLCall(glDeleteShader(id));
        return 0;
    }

    return id;
}

static int CreateShader(const std::string& vertexShader, const std::string& fragmentShader)
{
    unsigned int program{ glCreateProgram() };
    unsigned int vs{ CompileShader(GL_VERTEX_SHADER, vertexShader) };
    unsigned int fs{ CompileShader(GL_FRAGMENT_SHADER, fragmentShader) };

    GLCall(glAttachShader(program, vs));
    GLCall(glAttachShader(program, fs));
    GLCall(glLinkProgram(program));

    GLint program_linked;

    GLCall(glGetProgramiv(program, GL_LINK_STATUS, &program_linked));
    std::cout << "Program link status: " << program_linked << std::endl;
    if (program_linked != GL_TRUE)
    {
        GLsizei log_length{ 0 };
        GLchar message[1024];
        glGetProgramInfoLog(program, 1024, &log_length, message);
        std::cout << "Failed to link program" << std::endl;
        std::cout << message << std::endl;
    }

    GLCall(glValidateProgram(program));

    GLCall(glDeleteShader(vs));
    GLCall(glDeleteShader(fs));

    return program;
}

int main(void)
{
    GLFWwindow* window;

    /* Initialize the glfw library */
    if (!glfwInit())
    {
        fprintf(stderr, "Failed to initialize GLFW\n");
        getchar();
        return -1;
    }

    //to be able to run on all OS always set these window hints
    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
 
 
    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(1024, 768, "You Little Monkey", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    /* Make the window's context current */
    glfwMakeContextCurrent(window);

    glfwSwapInterval(1); //syncs to refresh rate

    //Initialize glew here after making opengl window
    glewExperimental = true;            //needed for core profile
    if (glewInit() != GLEW_OK)          //GLEW_OK = 0
        std::cout << "Error" << std::endl;

    std::cout << glGetString(GL_VERSION) << std::endl;

    //Ensure we can capture the escape key being pressed below
    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
    
    //data that will be passed to our buffer
    float positions[] {
        -0.5f, -0.5f,   //1
         0.5f, -0.5f,   //2
         0.5f,  0.5f,   //3
        -0.5f,  0.5f    //4
    };

    //our index buffer telling which points in space to connect
    unsigned int indices[] {
        0, 1, 2,
        2, 3, 0
    };



    //VAO (vertex array object) must be set up before binding attributes                                                                     
    //needed when going into core profile mode; is created for you in compat mode
    unsigned int VertexArrayID;                                             //sending the address of vertices to fill with and ID of 1
    GLCall(glGenVertexArrays(1, &VertexArrayID));                           
    GLCall(glBindVertexArray(VertexArrayID));

    //create buffer and copy data
    VertexBuffer vertexBuff(positions, 4 * 2 * sizeof(float)); 
    
    //define vertex layout here
    GLCall(glEnableVertexAttribArray(0));
    GLCall(glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 2, 0));  
                                                                            //^^^^ == (start of indice, how big each vec is, data type, ?, amount to get to next vec in bytes, ?)
                                                                            //links vertices to our buffer
    IndexBuffer indexBuff(indices, 6);
    
    //upload shader
    ShaderProgramSource source = ParseShader("res/shader/Basic.shader");
    std::cout << "VERTEX\n" << source.VertexSource << std::endl;
    std::cout << "FRAGMENT\n" << source.FragmentSouce << std::endl;
    
    unsigned int shader = CreateShader(source.VertexSource, source.FragmentSouce);
    GLCall(glUseProgram(shader));

    //uniforms are essential for altering shader at run time (cpu computes rgba then sends to gpu) another form of sending data to the shader
    GLCall(int location = glGetUniformLocation(shader, "u_Color"));
    ASSERT(location != -1);
    GLCall(glUniform4f(location, 0.8f, 0.3f, 0.8f, 1.0f));

    float r = 0.0f;
    float increment = 0.05f;
    //DIDNT WORK
    //auto t_Start = std::chrono::high_resolution_clock::now();
    //auto t_Now = std::chrono::high_resolution_clock::now();
    //float time = std::chrono::duration_cast<std::chrono::duration<float>>(t_Now - t_Start).count();

    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window) && glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS)
    {
        /* Render here */
        glClear(GL_COLOR_BUFFER_BIT);
        GLCall(glUniform4f(location, r, 0.3f, 0.8f, 1.0f));

                                                //instead of binding vertex buffer, atrrib pointer etc. just bind vao
        GLCall(glBindVertexArray(VertexArrayID));
        indexBuff.Bind();                       //bind vertex buffer repeatedly

        if (r > 1.0f) increment = -0.5f;
        else if (r < 0.0f) increment = 0.5f;

        r += increment;

        //ISSUE A DRAW CALL'
        //Using 6 vertices using our 4 positions
        GLCall(glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr));   //drawing a triangle starting at indice 0 with 3 rows of data
        

        /* Swap front and back buffers */
        glfwSwapBuffers(window);

        /* Poll for and process events */
        glfwPollEvents();
    }
    GLCall(glDeleteProgram(shader));
    glfwTerminate();
    return 0;
}