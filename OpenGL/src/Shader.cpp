#include "Shader.h"
#include "Render.h"
#include <GL/glew.h>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>



Shader::Shader(const std::string& filepath)
	: m_FilePath(filepath), m_RendererID(0)
{
    ShaderProgramSource source = ParseShader(filepath);
    m_RendererID = CreateShader(source.VertexSource, source.FragmentSource);

}

Shader::~Shader()
{
    GLCall(glDeleteProgram(m_RendererID));
}

void Shader::Bind() const
{
    GLCall(glUseProgram(m_RendererID));
}

void Shader::Unbind() const
{
    GLCall(glUseProgram(0));
}

void Shader::SetUnifrom4f(const std::string& name, float v0, float v1, float v2, float v3)
{
    GLCall(glUniform4f(GetUniformLocation(name), v0, v1, v2, v3));
}

unsigned int Shader::GetUniformLocation(const std::string& name)
{
    if (m_UniformLocationCache.find(name) != m_UniformLocationCache.end())
    {
        return m_UniformLocationCache[name];
    }

    GLCall(int location = glGetUniformLocation(m_RendererID, name.c_str()));
    if (location == -1)
        std::cout << "Warning: uniform " << name << " doesn't exist!" << std::endl;

    m_UniformLocationCache[name] = location;
    return location;
}

unsigned int Shader::CompileShader(unsigned int type, const std::string& source)
{
    unsigned int id = glCreateShader(type);                                             // create the shaders of the type
    const char* src = source.c_str();
    glShaderSource(id, 1, &src, nullptr);                                               // give opengl the shader code
    glCompileShader(id);                                                                // tell opengl to compile the shader code

    // TODO: Error Handling


    int result;
    glGetShaderiv(id, GL_COMPILE_STATUS, &result);                                      // get the shader compile status
    if (result == GL_FALSE)
    {
        int length;
        glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);                                 // get the log info length
        char* message = (char*) alloca(length * sizeof(char));
        glGetShaderInfoLog(id, length, &length, message);                               // get the log message

        std::cout << "Failed to compile " << (type == GL_VERTEX_SHADER ? "vertex" : "fragment") << " shader!" << std::endl;
        std::cout << message << std::endl;
        GLCall(glDeleteShader(id));                                                             // delete the shader because it was bad

        return 0;
    }

    return id;
}

unsigned int Shader::CreateShader(const std::string& vertexShader, const std::string& fragmentShader)
{
    unsigned int program;
    GLCall(program = glCreateProgram());                                                        // create a empty program object to attach the shaders to
    unsigned int vs = CompileShader(GL_VERTEX_SHADER, vertexShader);
    unsigned int fs = CompileShader(GL_FRAGMENT_SHADER, fragmentShader);

    (glAttachShader(program, vs));                                                        // attach the vertex shader to the program
    (glAttachShader(program, fs));                                                        // attach the fragment shader to the program
    GLCall(glLinkProgram(program));                                                             // link the program 
    ProgramInfo(program, GL_LINK_STATUS);
    GLCall(glValidateProgram(program));                                                         // check to see if the "executables" in program can excute in the given opengl state
    ProgramInfo(program, GL_VALIDATE_STATUS);

    GLCall(glDeleteShader(vs));                                                                 // free the memory and invalidate the names associated with the shader object (vertex)
    GLCall(glDeleteShader(fs));                                                                 // free the memory and invalidate the names associated with the shader object (fragment)

    return program;

}


ShaderProgramSource Shader::ParseShader(const std::string& filepath)
{
    std::ifstream stream(filepath);

    enum class ShaderType
    {
        NONE = -1, VERTEX = 0, FRAGMENT = 1
    };

    std::string line;
    std::stringstream ss[2];
    ShaderType type = ShaderType::NONE;
    while (getline(stream, line))
    {
        if (line.find("#shader") != std::string::npos)
        {
            if (line.find("vertex") != std::string::npos)
            {
                // set mode to vertex
                type = ShaderType::VERTEX;
            }
            else if (line.find("fragment") != std::string::npos)
            {
                // set mode to fragment
                type = ShaderType::FRAGMENT;
            }

        }
        else
        {
            ss[(int)type] << line << "\n";
        }
    }

    return { ss[0].str(), ss[1].str() };
}

void Shader::ProgramInfo(unsigned int program, unsigned int parameter)
{
    int result;
    GLCall(glGetProgramiv(program, parameter, &result));

    if (result == GL_FALSE)
    {
        int length;
        GLCall(glGetProgramiv(program, GL_INFO_LOG_LENGTH, &length));
        char* message = (char*) alloca(length * sizeof(char));
        GLCall(glGetProgramInfoLog(program, length, &length, message));

        std::cout << " Program: " << program << " (shader) Error!" << std::endl;
        std::cout << message << std::endl;
    }
}
