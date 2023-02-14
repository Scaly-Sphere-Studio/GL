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

/** Abstractization of glfw contexts, inspired by std::lock.
 *  Make given context current in scope.
 *  @usage
 *  @code
 *  // Context is set to something
 *  
 *  void func(Window::Shared window)
 *  {
 *      // Set context to given window
 *      Context const context(window);
 *      
 *      // OpenGL operations ...
 *  }
 * 
 *  // Context is back to previous
 *  @endcode
 */

class Context final {
private:
    void _init(GLFWwindow* ptr);
public:
    /** Make given context current if needed.*/
    Context(std::weak_ptr<Window> ptr);
    /** Make given context current if needed.*/
    Context(GLFWwindow* ptr);
    /** Swap to previous context if it was changed in constructor.*/
    ~Context();
    /** @cond INTERNAL*/
    Context()                           = delete;   // Default constructor
    Context(const Context&)             = delete;   // Copy constructor
    Context(Context&&)                  = delete;   // Move constructor
    Context& operator=(const Context&)  = delete;   // Copy assignment
    Context& operator=(Context&&)       = delete;   // Move assignment
    /** @endcond*/
private:
    GLFWwindow* _given{ nullptr };
    GLFWwindow* _previous{ nullptr };
    bool _equal{ true };
};

class Input {
public:
    inline int count() const noexcept { return _count; }
    
    inline bool is_held() const noexcept { return _count != 0; };
    inline bool is_held(int min_count) const noexcept { return min_count <= _count; };
    inline operator bool() const noexcept { return is_held(); };
    
    inline bool is_pressed() const noexcept { return _is_pressed && is_held(); };
    inline bool is_pressed(int min_count) const noexcept { return _is_pressed && is_held(min_count); };
    
    inline bool is_released() const noexcept { return _is_pressed && !is_held(); };

    inline void increment(std::chrono::milliseconds const& input_stack_time) noexcept {
        auto const now = std::chrono::steady_clock::now();
        if ((now - _last_pressed) <= input_stack_time) {
            _count = _old_count;
        }
        ++_count;
        _is_pressed = true;
        _last_pressed = now;
    }
    inline void reset() noexcept {
        _old_count = _count;
        _count = 0;
        _is_pressed = true;
    }
    inline void handled() noexcept {
        _is_pressed = false;
    };
private:
    int _count{ 0 };
    int _old_count{ 0 };
    bool _is_pressed{ false };
    std::chrono::steady_clock::time_point _last_pressed;
};


INTERNAL_BEGIN;

// This class is to be inherited in all classes whose instances are
// bound to a specific Window instance.
class WindowObject {
public:
    WindowObject() = delete;
    std::shared_ptr<Window> const getWindow() const;
    char const* getWindowTitle() const;
    Context const getContext() const;
protected:
    WindowObject(std::shared_ptr<Window> window);
    void _changeWindow(std::shared_ptr<Window> window);
    static std::shared_ptr<Window> _getFirstWindow();
private:
    std::weak_ptr<Window> _window;
    GLFWwindow* _glfw_window;
};

template<class T>
class SharedWindowObject : public WindowObject, public std::enable_shared_from_this<T> {
public:
    SharedWindowObject() = delete;
    using Shared = std::shared_ptr<T>;
    using Vector = std::vector<Shared>;

protected:
    SharedWindowObject(std::shared_ptr<Window> window) : WindowObject(window) {};

public:
    static Shared create(std::shared_ptr<Window> window)
    {
        if (!window) {
            window = _getFirstWindow();
        }
        return Shared(new T(window));
    }
    static Shared create() { return create(std::shared_ptr<Window>(nullptr)); }
    static Shared create(GLFWwindow* window) { return create(Window::get(window)); }
};

template<class T>
class InstancedWindowObject : public SharedWindowObject<T>
{
public:
    InstancedWindowObject() = delete;
    ~InstancedWindowObject() { cleanWeakPtrVector(_instances); };
protected:
    InstancedWindowObject(std::shared_ptr<Window> window) : SharedWindowObject<T>(window) {};
    using Weak = std::weak_ptr<T>;
    static std::vector<Weak> _instances;

public:
    static auto create(std::shared_ptr<Window> window)
    {
        auto shared = SharedWindowObject<T>::create(window);
        _instances.emplace_back(shared);
        return shared;
    }
    static auto create() { return create(std::shared_ptr<Window>(nullptr)); }
    static auto create(GLFWwindow* window) { return create(Window::get(window)); }

    static auto getInstances(std::shared_ptr<Window> window) noexcept {
        SharedWindowObject<T>::template Vector vec;
        for (Weak const& weak : _instances) {
            auto const instance = weak.lock();
            if (instance && (!window || window == instance->getWindow()))
                vec.emplace_back(instance);
        }
        return vec;
    }
    static auto getInstances() noexcept { return getInstances(nullptr); }

    static auto get(T* raw_ptr) {
        for (auto const& weak_ptr : _instances) {
            auto shared_ptr = weak_ptr.lock();
            if (shared_ptr && shared_ptr.get() == raw_ptr) {
                return shared_ptr;
            }
        }
        return SharedWindowObject<T>::template Shared(nullptr);
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