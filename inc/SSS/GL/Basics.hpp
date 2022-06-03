#pragma once

#include "_internal/_includes.hpp"

/** @file
 *  Defines namespace SSS::GL::Basic and subsequent classes.
 */

SSS_GL_BEGIN;

class Window; // Pre-declaration of Window class.

INTERNAL_BEGIN;

// This class is to be inherited in all classes whose instances are
// bounded (without ID) to a specific Window instance.
class WindowObject {
public:
    WindowObject() = delete;
protected:
    WindowObject(std::weak_ptr<Window> window) : _window(window) {};
    std::weak_ptr<Window> _window;
};

// This class is to be inherited in all classes whose instances are
// bounded (with an ID) to a specific Window instance.
class WindowObjectWithID {
public:
    WindowObjectWithID() = delete;
    uint32_t getID() const noexcept { return _id; };
protected:
    WindowObjectWithID(std::weak_ptr<Window> window, uint32_t id)
        : _window(window), _id(id) {};
    std::weak_ptr<Window> _window;
    uint32_t const _id;
};

INTERNAL_END;

/** %Basic abstractisation of \b OpenGL objects.*/
namespace Basic {

    /** Abstractisation of OpenGL \b textures and their
     *  creation, deletion, settings and editing.
     */
    struct Texture : public _internal::WindowObject {
        /** Unique instance pointer.*/
        using Ptr = std::unique_ptr<Texture>;
        /** Constructor, creates an \b OpenGL and sets #id accordingly.
         *  Forces to be bound to a Window instance.
         *  @sa ~Texture()
         */
        Texture(std::weak_ptr<Window> window, GLenum given_target);
        /** Destructor, deletes the \b OpenGL texture of corresponding #id.
         *  @sa Texture()
         */
        ~Texture();
        /** Binds the texture to the context in which it was created.
         *  Effectively calls \c glBindTexture() with #target and #id.
         * 
         *  Implicitly called in parameteri() and edit().
         */
        void bind() const;
        /** Sets corresponding parameters.
         *  Effectively calls \c glTexParameteri() with #target and
         *  given arguments.
         * 
         *  Implicitly calls bind().
         */
        void parameteri(GLenum pname, GLint param);
        /** Edits pixel storage.
         *  Effectively calls \c glTexImage2D() with #target and
         *  given arguments.
         * 
         *  Implicitly calls bind().
         */
        void edit(const GLvoid* pixels, GLsizei width, GLsizei height,
            GLenum format = GL_RGBA, GLint internalformat = GL_RGBA,
            GLenum type = GL_UNSIGNED_BYTE, GLint level = 0);
        /** The target specified in the constructor.*/
        GLenum const target;
        /** %Texture ID generated by \b OpenGL.*/
        GLuint const id;
    };

    /** Abstractisation of OpenGL vertex array objects (\b %VAO) and
     *  their creation, deletion, and binding.
     */
    struct VAO : public _internal::WindowObject {
        /** Unique instance pointer.*/
        using Ptr = std::unique_ptr<VAO>;
        /** Constructor, creates an \b OpenGL vertex array and
         *  sets #id accordingly.
         *  Forces to be bound to a Window instance.
         *  @sa ~VAO()
         */
        VAO(std::weak_ptr<Window> window);
        /** Destructor, deletes the \b OpenGL vertex array
         *  of corresponding #id.
         *  @sa VAO()
         */
        ~VAO();
        /** Binds the vertex array to the context in which it was created.
         *  Effectively calls \c glBindVertexArray() with #id.
         */
        void bind() const;
        /** Vertex array ID generated by \b OpenGL.*/
        GLuint const id;
    };

    /** Abstractisation of OpenGL vertex buffer objects (\b %VBO) and
     *  their creation, deletion, and editing.
     */
    struct VBO : public _internal::WindowObject {
        /** Unique instance pointer.*/
        using Ptr = std::unique_ptr<VBO>;
        /** Constructor, creates an \b OpenGL buffer object and
         *  sets #id accordingly.
         *  Forces to be bound to a Window instance.
         *  @sa ~VBO()
         */
        VBO(std::weak_ptr<Window> window);
        /** Destructor, deletes the \b OpenGL buffer object
         *  of corresponding #id.
         *  @sa VBO()
         */
        ~VBO();
        /** Binds the buffer object to the context in which it was created.
         *  Effectively calls \c glBindBuffer() with \c GL_ARRAY_BUFFER and #id.
         *
         *  Implicitly called in edit().
         */
        void bind() const;
        /** Edits buffer object.
         *  Effectively calls \c glBufferData() with GL_ARRAY_BUFFER and
         *  given arguments.
         * 
         *  Implicitly calls bind().
         */
        void edit(GLsizeiptr size, const void* data, GLenum usage);
        /** Buffer object ID generated by \b OpenGL.*/
        GLuint const id;
    };

    /** Abstractisation of OpenGL index buffer objects (\b %IBO) and
     *  their creation, deletion, and editing.
     */
    struct IBO : public _internal::WindowObject {
        /** Unique instance pointer.*/
        using Ptr = std::unique_ptr<IBO>;
        /** Constructor, creates an \b OpenGL buffer object and
         *  sets #id accordingly.
         *  Forces to be bound to a Window instance.
         *  @sa ~IBO()
         */
        IBO(std::weak_ptr<Window> window);
         /** Destructor, deletes the \b OpenGL buffer object
          *  of corresponding #id.
          *  @sa VBO()
          */
        ~IBO();
        /** Binds the buffer object to the context in which it was created.
         *  Effectively calls \c glBindBuffer() with \c GL_ELEMENT_ARRAY_BUFFER
         *  and #id.
         *
         *  Implicitly called in edit().
         */
        void bind() const;
        /** Edits buffer object.
         *  Effectively calls \c glBufferData() with GL_ELEMENT_ARRAY_BUFFER
         *  and given arguments.
         *
         *  Implicitly calls bind().
         */
        void edit(GLsizeiptr size, const void* data, GLenum usage);
        /** Buffer object ID generated by \b OpenGL.*/
        GLuint const id;
    };
}

SSS_GL_END;