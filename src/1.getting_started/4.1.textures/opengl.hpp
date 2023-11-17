#pragma once


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
    virtual ~Tex() { glDeleteTextures(1, &id); }
};

class Buffer {
public:
    unsigned int id;
    int target;
    static Buffer *create(int target, const void *bytes, uint64_t size, GLenum usage) {
        Buffer *buf = new Buffer(target);
        buf->bind();
        buf->data(size, bytes, usage);
        return buf;
    }
    Buffer(int target) {
        this->target = target;
        glGenBuffers(1, &id);
    }
    ~Buffer() { glDeleteBuffers(1, &id); }
    void bind() { glBindBuffer(target, id); }
    void data(uint64_t size, const void *data, GLenum usage) {
        bind();
        glBufferData(target, size, data, usage);
    }
};

class VertexArray {
public:
    unsigned int id;
    static VertexArray *create() {
        VertexArray *obj = new VertexArray();
        obj->bind();
        return obj;
    }
    VertexArray() { glGenVertexArrays(1, &id); }
    ~VertexArray() { glDeleteVertexArrays(1, &id); }
    void bind() { glBindVertexArray(id); }
    VertexArray *attrib(int index, GLint size, GLsizei stride, uint64_t pointer) {
        auto floatSize = sizeof(float);
        stride *= floatSize;
        pointer *= floatSize;
        glVertexAttribPointer(index, size, GL_FLOAT, GL_FALSE, stride, (const GLvoid *)pointer);
        glEnableVertexAttribArray(index);
        return this;
    }
};

