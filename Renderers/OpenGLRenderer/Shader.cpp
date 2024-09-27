#include "Shader.h"
#include <fstream>

static std::string get_file_contents(const char* filename)
{
    std::ifstream in(filename, std::ios::binary);
    if (in)
    {
        std::string contents;
        in.seekg(0, std::ios::end);
        contents.resize(in.tellg());
        in.seekg(0, std::ios::beg);
        in.read(contents.data(), contents.size());
        in.close();
        return(contents);
    }
    throw(errno);
}

Shader::Shader() {

}

Shader::~Shader()
{
}

void Shader::Init(const char* VertexShader, const char* FragmentShader, const char* GeomtryShader)
{

    std::string s = get_file_contents(VertexShader);
    std::string f = get_file_contents(FragmentShader);
    std::string g;
    const char* geom_source = nullptr;
    const char* vertex_source = s.c_str();
    const char* frag_source = f.c_str();

    // Vertex
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertex_source, NULL);
    glCompileShader(vertexShader);
    checkCompileErrors(vertexShader, "VERTEX");

    // fragment Shader
    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &frag_source, NULL);
    glCompileShader(fragmentShader);
    checkCompileErrors(fragmentShader, "FRAGMENT");
    unsigned int geometryShader;
    
    // shader Program
    ID = glCreateProgram();
    glAttachShader(ID, vertexShader);
    if (GeomtryShader != nullptr) {
        g = get_file_contents(GeomtryShader);
        const char* geom_source = g.c_str();
        geometryShader = glCreateShader(GL_GEOMETRY_SHADER);
        glShaderSource(geometryShader, 1, &geom_source, NULL);
        glCompileShader(geometryShader);
        checkCompileErrors(geometryShader, "GEOMTRY");
        glAttachShader(ID, geometryShader);
    }
    glAttachShader(ID, fragmentShader);
    glLinkProgram(ID);
    checkCompileErrors(ID, "PROGRAM");
    // delete the shaders as they're linked into our program now and no longer necessary
    glDeleteShader(vertexShader);
    glDeleteShader(geometryShader);
    glDeleteShader(fragmentShader);
}

void Shader::use()
{
    glUseProgram(ID);
}
void Shader::checkCompileErrors(unsigned int shader, std::string type)
{
    int success;
    char infoLog[1024];
    if (type != "PROGRAM")
    {
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(shader, 1024, NULL, infoLog);
            std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
        }
    }
    else
    {
        glGetProgramiv(shader, GL_LINK_STATUS, &success);
        if (!success)
        {
            glGetProgramInfoLog(shader, 1024, NULL, infoLog);
            std::cout << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
        }
    }
}

void Shader::setBool(const std::string& name, bool value) const 
{
    glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value);
}
// ------------------------------------------------------------------------
void Shader::setInt(const std::string& name, int value) const
{
    glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
}
// ------------------------------------------------------------------------
void Shader::setFloat(const std::string& name, float value) const
{
    glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
}
void Shader::setVec2(const std::string& name, float x, float y) const
{
    glUniform2f(glGetUniformLocation(ID, name.c_str()), x, y);
}
void Shader::setVec3(const std::string& name,glm::vec3 v) const
{
    glUniform3f(glGetUniformLocation(ID, name.c_str()), v.x, v.y, v.z);
}
void Shader::setVec4(const std::string& name, float x, float y, float z, float w) const
{
    glUniform4f(glGetUniformLocation(ID, name.c_str()), x, y, z, w);
}
// ------------------------------------------------------------------------
void Shader::setMat2(const std::string& name, const glm::mat2 mat) const
{
    glUniformMatrix2fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, glm::value_ptr(mat));
}
// ------------------------------------------------------------------------
void Shader::setMat3(const std::string& name, const glm::mat3 mat) const
{
    glUniformMatrix3fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, glm::value_ptr(mat));
}
// ------------------------------------------------------------------------
void Shader::setMat4(const std::string& name, const glm::mat4 mat) const
{
    glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, glm::value_ptr(mat));
}