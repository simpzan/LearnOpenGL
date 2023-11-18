#pragma once
#include <memory>
#include "log.h"

class Tex {
public:
    unsigned int id;
    int target;
public:
    Tex(int target, int wrapS, int wrapT, int filterMin, int filterMag) {
        this->target = target;
        glGenTextures(1, &id);
        glBindTexture(target, id);
        glTexParameteri(target, GL_TEXTURE_WRAP_S, wrapS);
        glTexParameteri(target, GL_TEXTURE_WRAP_T, wrapT);
        glTexParameteri(target, GL_TEXTURE_MIN_FILTER, filterMin);
        glTexParameteri(target, GL_TEXTURE_MAG_FILTER, filterMag);
        TRO();
    }
    void bind() { glBindTexture(target, id); }
    virtual void image2D(int level, int format, int width, int height, int type, void *data) {
        bind();
        glTexImage2D(target, level, format, width, height, 0, format, type, data);
    }
    void compressedImage2D(GLint level, GLenum format, GLsizei width, GLsizei height,
            GLsizei size, const GLvoid * data) {
        glCompressedTexImage2D(target, level, format, width, height, 0, size, data);
    }
    void setWrapParam(int key, int value) {
        bind();
        glTexParameteri(target, key, value);
    }
    void setLevel(int base, int max) {
        setWrapParam(GL_TEXTURE_BASE_LEVEL, base);
        setWrapParam(GL_TEXTURE_MAX_LEVEL, max);
    }
    void generateMipmap() {
        bind();
        glGenerateMipmap(target);
    }
    virtual ~Tex() { glDeleteTextures(1, &id); TRO(); }
};
std::unique_ptr<Tex> texFromFile(const char *filename, int format) {
    int width, height, nrChannels;
    auto data = stbi_load(filename, &width, &height, &nrChannels, 0);
    if (!data) {
        std::cout << "Failed to load texture" << std::endl;
        return nullptr;
    }
    std::unique_ptr<Tex> tex { new Tex(GL_TEXTURE_2D, GL_REPEAT, GL_REPEAT, GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR) };
    tex->image2D(0, format, width, height, GL_UNSIGNED_BYTE, data);
    tex->generateMipmap();

    stbi_image_free(data);
    return tex;
}

class Buffer {
public:
    unsigned int id;
    int target;
    static std::unique_ptr<Buffer> create(int target, const void *bytes, uint64_t size, GLenum usage) {
        std::unique_ptr<Buffer> buf { new Buffer(target) };
        buf->bind();
        buf->data(size, bytes, usage);
        return buf;
    }
    Buffer(int target) {
        this->target = target;
        glGenBuffers(1, &id);
        TRO();
    }
    ~Buffer() { glDeleteBuffers(1, &id); TRO(); }
    void bind() { glBindBuffer(target, id); }
    void data(uint64_t size, const void *data, GLenum usage) {
        bind();
        glBufferData(target, size, data, usage);
    }
};

class VertexArray {
public:
    unsigned int id;
    VertexArray() { glGenVertexArrays(1, &id); TRO(); }
    ~VertexArray() { glDeleteVertexArrays(1, &id); TRO(); }
    VertexArray &bind() {
        glBindVertexArray(id);
        return *this;
    }
    VertexArray &attrib(int index, GLint size, GLsizei stride, uint64_t pointer) {
        auto floatSize = sizeof(float);
        stride *= floatSize;
        pointer *= floatSize;
        glVertexAttribPointer(index, size, GL_FLOAT, GL_FALSE, stride, (const GLvoid *)pointer);
        glEnableVertexAttribArray(index);
        return *this;
    }
};


class Shader {
public:
    unsigned int id;
    GLenum type;
    static std::unique_ptr<Shader> create(GLenum type, const char *source) {
        std::unique_ptr<Shader> shader { new Shader(type) };
        bool ok = shader->compile(source);
        if (ok) return shader;
        return nullptr;
    }
    Shader(GLenum type) {
        this->type = type;
        id = glCreateShader(type);
        TRO();
    }
    ~Shader() { glDeleteShader(id); TRO(); }
    bool compile(const char *source) {
        glShaderSource(id, 1, &source, NULL);
        glCompileShader(id);
        return checkCompileResult();
    }
private:
    bool checkCompileResult() {
        int success;
        char infoLog[1024];
        glGetShaderiv(id, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(id, 1024, NULL, infoLog);
            std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n"
                << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
            return false;
        }
        return true;
    }
};
class Program {
public:
    unsigned int id;
    static std::unique_ptr<Program> create(const char *vs, const char *fs) {
        auto vertex = Shader::create(GL_VERTEX_SHADER, vs);
        if (!vertex) return nullptr;
        auto fragment = Shader::create(GL_FRAGMENT_SHADER, fs);
        if (!fragment) return nullptr;
        std::unique_ptr<Program> program { new Program() };
        program->link({ vertex.get(), fragment.get() });
        return program;
    }
    Program() { id = glCreateProgram(); TRO(); }
    ~Program() { glDeleteProgram(id); TRO(); }
    bool link(std::vector<Shader *>shaders) {
        for (auto shader: shaders) glAttachShader(id, shader->id);
        glLinkProgram(id);
        return checkLinkResult();
    }
    void use() { glUseProgram(id); }

    void setUniform(const char *name, bool value) const {
        glUniform1i(glGetUniformLocation(id, name), (int)value);
    }
    void setUniform(const char *name, int value) const {
        glUniform1i(glGetUniformLocation(id, name), value);
    }
    void setUniform(const char *name, float value) const {
        glUniform1f(glGetUniformLocation(id, name), value);
    }

private:
    bool checkLinkResult() {
        int success;
        char infoLog[1024];
        glGetProgramiv(id, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(id, 1024, NULL, infoLog);
            std::cout << "ERROR::PROGRAM_LINKING_ERROR of type: " << "program" << "\n"
                    << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
            return false;
        }
        return true;
    }
};
