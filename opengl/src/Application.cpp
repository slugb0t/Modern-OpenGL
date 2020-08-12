//include Glew
#include <GL/glew.h>
//Include GLFW
#include <GLFW/glfw3.h>

#include <iostream>
#include <string>
#include <fstream>
#include <sstream>

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

    //opens the file using fstream
    std::ifstream stream(filepath);
    std::string line;
    std::stringstream ss[2];
    ShaderType type = ShaderType::NONE;

    //sstream needed for getline as well
    while (getline(stream, line))
    {
        if (line.find("#shader") != std::string::npos)
        {
            if (line.find("vertex") != std::string::npos)
            {
                //set more to vertex
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
    unsigned int id = glCreateShader(type);
    const char* src = source.c_str();
    glShaderSource(id, 1, &src, nullptr);
    glCompileShader(id);
   
    //glGetShaderiv(id, GL_COMPILE_STATUS, &result);
    //if(result)

    //TODO:ERROR HANDLING
    //query to check if ID is good
    int result;
    glGetShaderiv(id, GL_COMPILE_STATUS, &result);
    //std::cout << (type == GL_VERTEX_SHADER ? "vertex" : "fragment") << "shader compile status" << result << std::endl;
    if (result == GL_FALSE)
    {
        //print the error message
        int length;
        glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);
        //char message[length];

        char* message = (char*)alloca(length * sizeof(char));
        //allocate on the stack dynamically
        glGetShaderInfoLog(id, length, &length, message);
        std::cout << "FAILED TO COMPILE SHADER:" << 
            (type == GL_VERTEX_SHADER ? "vertex" : "fragment") << " shader" << std::endl;
        std::cout << message << std::endl;
        glDeleteShader(id);
        return 0;
    }

    return id;
}


static int CreateShader(const std::string& vertexShader, const std::string& fragmentShader)
{
    unsigned int program = glCreateProgram();
    unsigned int vs = CompileShader(GL_VERTEX_SHADER, vertexShader);
    unsigned int fs = CompileShader(GL_FRAGMENT_SHADER, fragmentShader);

    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);

    GLint program_linked;

    glGetProgramiv(program, GL_LINK_STATUS, &program_linked);
    std::cout << "Program link status: " << program_linked << std::endl;
    if (program_linked != GL_TRUE)
    {
        GLsizei log_length = 0;
        GLchar message[1024];
        glGetProgramInfoLog(program, 1024, &log_length, message);
        std::cout << "Failed to link program" << std::endl;
        std::cout << message << std::endl;
    }

    glValidateProgram(program);

    glDeleteShader(vs);
    glDeleteShader(fs);

    return program;
}

int main(void)
{
    GLFWwindow* window;

    /* Initialize the library */
    if (!glfwInit())
    {
        fprintf(stderr, "Failed to initialize GLFW\n");
        getchar();
        return -1;
    }

    //to run on all OS
    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
 
 
    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(1024, 768, "Hello World", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    /* Make the window's context current */
    glfwMakeContextCurrent(window);


    //Initialize glew here after making opengl window
    glewExperimental = true; //needed for core profile
    if (glewInit() != GLEW_OK)
        std::cout << "Error" << std::endl;

    std::cout << glGetString(GL_VERSION) << std::endl;

    //Ensure we can capture the escape key being pressed below
    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

    //ADDEDFROMGIT
    unsigned int VertexArrayID;
    glGenVertexArrays(1, &VertexArrayID);
    glBindVertexArray(VertexArrayID);

    
    //HERE WE CREATED THE BUFFER TO HOLD THE DATA OF THE TRIANGLE POSITIONS
    //data that will be passed to our buffer
    float positions[6] = {
        -0.5f, -0.5f,
         0.0f,  0.5f,
         0.5f, -0.5f
    };

    //create buffer and copy data
    unsigned int buffer;
    glGenBuffers(1, &buffer);   //sending the address of buffer to fill with and ID of 1
    glBindBuffer(GL_ARRAY_BUFFER, buffer);  //buffer of memory
    //now put data in buffer
    glBufferData(GL_ARRAY_BUFFER, 6 * sizeof(float), positions, GL_STATIC_DRAW);  //2nd paramter in bytes
    

    //define vertex layout here
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 2, 0);
    glEnableVertexAttribArray(0);
    //start of indice, then how big the vec is (vec2),
    //stride is the amount to get to the next vertex in bytes

    ShaderProgramSource source = ParseShader("res/shader/Basic.shader");
    std::cout << "VERTEX\n" << source.VertexSource << std::endl;
    std::cout << "FRAGMENT\n" << source.FragmentSouce << std::endl;
    
    unsigned int shader = CreateShader(source.VertexSource, source.FragmentSouce);
    glUseProgram(shader);
    
    //glBindBuffer(GL_ARRAY_BUFFER, 0);

    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window) || glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS)
    {
        /* Render here */
        glClear(GL_COLOR_BUFFER_BIT);

        //ISSUE A DRAW CALL
        glDrawArrays(GL_TRIANGLES, 0, 3);   //drawing a triangle starting at indice 0 with 3 rows of data

        /* Swap front and back buffers */
        glfwSwapBuffers(window);

        /* Poll for and process events */
        glfwPollEvents();
    }
    glDeleteProgram(shader);
    glfwTerminate();
    return 0;
}