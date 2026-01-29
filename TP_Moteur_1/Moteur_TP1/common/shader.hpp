#ifndef SHADER_HPP
#define SHADER_HPP

#include <glm/glm.hpp>
#include <string>

class Shader{
public:
    //程序ID
    unsigned int m_ID;

    //构造器读取并构建着色器
    Shader(const char* vertexPath, const char* fragmentPath);

    //使用/激活程序
    void use();
    //uniform工具函数
    void setBool(const std::string &name,bool value) const;
    void setInt(const std::string &name,int value) const;
    void setFloat(const std::string &name,float value) const;
    void setMat4(const std::string &name, const glm::mat4 &mat) const;

private:
    void checkCompileErrors(unsigned int shader, std::string type);

};


#endif
