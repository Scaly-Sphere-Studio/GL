#ifndef SSS_GL_BASIC_HPP
#define SSS_GL_BASIC_HPP

#include <SSS/Commons.hpp>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

/** @file
 *  Defines namespace SSS::GL::Basic and subsequent classes.
 */

 /** Declares the SSS::GL namespace.
  *  Further code will be nested in the SSS::GL namespace.\n
  *  Should be used in pair with with #SSS_GL_END.
  */
#define SSS_GL_BEGIN SSS_BEGIN; namespace GL {
  /** Closes the SSS::GL namespace declaration.
   *  Further code will no longer be nested in the SSS::GL namespace.\n
   *  Should be used in pair with with #SSS_GL_BEGIN.
   */
#define SSS_GL_END SSS_END; }

   /** \cond INTERNAL*/

   /** Logs the given message with "SSS/GL: " prepended to it.*/
#define LOG_GL_MSG(X) LOG_CTX_MSG("SSS/GL", X)

/** Logs the given message with "SSS/GL: %window_name%: " prepended to it.*/
#define LOG_WNDW_MSG(win, X) LOG_GL_MSG(win->getTitle(), X)

/** \endcond*/

/** Holds all SSS::GL related log flags.*/
namespace SSS::Log::GL {
    LOG_NAMESPACE_BASICS(Log);
}

SSS_GL_BEGIN;

class Window; // Pre-declaration of Window class.
class Context; // Pre-declaration of Context class.

INTERNAL_BEGIN;

// This class is to be inherited in all classes whose instances are
// bound to a specific Window instance.
class WindowObject {
public:
    WindowObject() = delete;
protected:
    WindowObject(std::shared_ptr<Window> window);
    void _changeWindow(std::shared_ptr<Window> window);
    std::shared_ptr<Window> const _get_window() const;
    Context const _get_context() const;
private:
    std::weak_ptr<Window> _window;
    GLFWwindow* _glfw_window;
};

// This class is to be inherited in all classes whose instances are
// bounded (with an ID) to a specific Window instance.
class WindowObjectWithID : public WindowObject {
public:
    WindowObjectWithID() = delete;
    uint32_t getID() const noexcept { return _id; };
protected:
    WindowObjectWithID(std::shared_ptr<Window> window, uint32_t id)
        : WindowObject(window), _id(id) {};
    uint32_t const _id;
};

template<class T>
class SharedWindowObject : public WindowObject {
public:
    SharedWindowObject() = delete;
    ~SharedWindowObject() { cleanWeakPtrVector(_instances); };
    using Shared = std::shared_ptr<T>;
    using Vector = std::vector<Shared>;

protected:
    SharedWindowObject(std::shared_ptr<Window> window) : WindowObject(window) {};
    using Weak = std::weak_ptr<T>;
    static std::vector<Weak> _instances;

public:
    static Shared create(std::shared_ptr<Window> window)
    {
        if (!window) {
            window = Window::getFirst();
        }
        Shared shared(new T(window));
        _instances.emplace_back(shared);
        return shared;
    }
    static Shared create() { return create(nullptr); }

    static Vector getInstances(std::shared_ptr<Window> window) noexcept {
        Vector vec;
        for (Weak const& weak : _instances) {
            Shared const instance = weak.lock();
            if (instance && (!window || window == instance->_get_window()))
                vec.emplace_back(instance);
        }
        return vec;
    }
    static Vector getInstances() noexcept { return getInstances(nullptr); }

    static Shared get(T* raw_ptr) {
        for (auto const& weak_ptr : _instances) {
            Shared shared_ptr = weak_ptr.lock();
            if (shared_ptr && shared_ptr.get() == raw_ptr) {
                return shared_ptr;
            }
        }
        return nullptr;
    }
};

INTERNAL_END;

/** %Basic abstractisation of \b OpenGL objects.*/
namespace Basic {

    /** Abstractisation of OpenGL \b textures and their
     *  creation, deletion, settings and editing.
     */
    struct Texture final : public _internal::WindowObject {
        /** Constructor, creates an \b OpenGL and sets #id accordingly.
         *  Forces to be bound to a Window instance.
         *  @sa ~Texture()
         */
        Texture(std::shared_ptr<Window> window, GLenum given_target);
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

        void setTarget(GLenum new_target);
        inline GLenum getTarget() const noexcept { return _target; };

        /** Sets corresponding parameters.
         *  Effectively calls \c glTexParameteri() with #target and
         *  given arguments.
         * 
         *  Implicitly calls bind().
         */
        void parameteri(GLenum pname, GLint param);

        void editSettings(int width, int height, int depth = 1);
        /** Edits pixel storage.
         *  Effectively calls \c glTexImage2D() with #target and
         *  given arguments.
         * 
         *  Implicitly calls bind().
         */
        void editPixels(const GLvoid* pixels, int z_offset = 0);

        /** %Texture ID generated by \b OpenGL.*/
        GLuint id;

    private:
        /** The target specified in the constructor.*/
        GLenum _target;
        int _width{ 0 }, _height{ 0 }, _depth{ 0 };
    };

    /** Abstractisation of OpenGL vertex array objects (\b %VAO) and
     *  their creation, deletion, and binding.
     */
    struct VAO final : public _internal::WindowObject {
        /** Constructor, creates an \b OpenGL vertex array and
         *  sets #id accordingly.
         *  Forces to be bound to a Window instance.
         *  @sa ~VAO()
         */
        VAO(std::shared_ptr<Window> window);
        /** Destructor, deletes the \b OpenGL vertex array
         *  of corresponding #id.
         *  @sa VAO()
         */
        ~VAO();
        /** Binds the vertex array to the context in which it was created.
         *  Effectively calls \c glBindVertexArray() with #id.
         */
        void bind() const;
        /** Unbinds any vertex array bound to the context in which this was created.
         *  Effectively calls \c glBindVertexArray() with \c 0.
         */
        void unbind() const;
        /** Vertex array ID generated by \b OpenGL.*/
        GLuint const id;
    };

    /** Abstractisation of OpenGL vertex buffer objects (\b %VBO) and
     *  their creation, deletion, and editing.
     */
    struct VBO final : public _internal::WindowObject {
        /** Constructor, creates an \b OpenGL buffer object and
         *  sets #id accordingly.
         *  Forces to be bound to a Window instance.
         *  @sa ~VBO()
         */
        VBO(std::shared_ptr<Window> window);
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
    struct IBO final : public _internal::WindowObject {
        /** Constructor, creates an \b OpenGL buffer object and
         *  sets #id accordingly.
         *  Forces to be bound to a Window instance.
         *  @sa ~IBO()
         */
        IBO(std::shared_ptr<Window> window);
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

#endif // SSS_GL_BASIC_HPP