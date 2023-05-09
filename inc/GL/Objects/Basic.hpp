#ifndef SSS_GL_BASIC_HPP
#define SSS_GL_BASIC_HPP

#ifdef SSS_GL_EXPORTS
  #define SSS_GL_API __declspec(dllexport)
#else
  #ifdef SSS_GL_DEMO
    #define SSS_GL_API
  #else
    #define SSS_GL_API __declspec(dllimport)
  #endif // SSS_GL_DEMO
#endif // SSS_GL_EXPORTS

#include <SSS/Commons.hpp>
#define GLFW_DLL
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <map>

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

/** Logs the given message with "GL: " prepended to it.*/
#define LOG_GL_MSG(X) LOG_CTX_MSG("GL", X)

/** Logs the given message with "GL: %window_name%: " prepended to it.*/
#define LOG_WNDW_MSG(win, X) LOG_GL_MSG(win->getTitle(), X)

/** \endcond*/

/** Holds all SSS::GL related log flags.*/
namespace SSS::Log::GL {
    LOG_NAMESPACE_BASICS(Log);
}

SSS_GL_BEGIN;

/** Global function which polls everything in the library.
 *  Process is as follow:
 * - Calls glfwPollEvents().
 * - Calls all TR::Area::update().
 * - Calls all Model's passive function.
 * - Edits every Texture with a pending thread (file load / text area).
 */
SSS_GL_API void pollEverything();

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

/** %Basic abstractisation of \b OpenGL objects.*/
namespace Basic {

    // Ignore warning about STL exports as they're private members
#pragma warning(push, 2)
#pragma warning(disable: 4251)
#pragma warning(disable: 4275)

    template<class T>
    class SharedBase : public ::SSS::Base, public std::enable_shared_from_this<T> {
    protected:
        SharedBase() {};
    public:
        using Shared = std::shared_ptr<T>;
        using Weak = std::weak_ptr<T>;
        using Vector = std::vector<Shared>;
        
        static Shared create()
        {
            return Shared(new T());
        }
    };

#pragma warning(pop)

    template<class T>
    class InstancedBase : public SharedBase<T>
    {
    public:
        ~InstancedBase() { cleanWeakPtrVector(_instances); };
    protected:
        InstancedBase() {};
        using Weak = std::weak_ptr<T>;
        static std::vector<Weak> _instances;

    public:
        static auto create()
        {
            auto shared = SharedBase<T>::create();
            _instances.emplace_back(shared);
            return shared;
        }

        static auto getInstances() noexcept {
            SharedBase<T>::template Vector vec;
            vec.reserve(_instances.size());
            for (Weak const& weak : _instances) {
                vec.emplace_back(weak.lock());
            }
            return vec;
        }

        static auto get(T* raw_ptr) {
            for (auto const& weak_ptr : _instances) {
                auto shared_ptr = weak_ptr.lock();
                if (shared_ptr && shared_ptr.get() == raw_ptr) {
                    return shared_ptr;
                }
            }
            return SharedBase<T>::template Shared(nullptr);
        }
    };

    template <class T>
    std::vector<std::weak_ptr<T>> InstancedBase<T>::_instances{};

    /** Abstractisation of OpenGL \b textures and their
     *  creation, deletion, settings and editing.
     */
    struct SSS_GL_API Texture {
        /** Constructor, creates an \b OpenGL and sets #id accordingly.
         *  Forces to be bound to a Window instance.
         *  @sa ~Texture()
         */
        Texture(GLenum given_target);
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

    /** Abstractisation of OpenGL vertex buffer objects (\b %VBO) and
     *  their creation, deletion, and editing.
     */
    struct SSS_GL_API VBO {
        /** Constructor, creates an \b OpenGL buffer object and
         *  sets #id accordingly.
         *  Forces to be bound to a Window instance.
         *  @sa ~VBO()
         */
        VBO();
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

        static void bind(GLuint vbo_id);

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
    struct SSS_GL_API IBO {
        /** Constructor, creates an \b OpenGL buffer object and
         *  sets #id accordingly.
         *  Forces to be bound to a Window instance.
         *  @sa ~IBO()
         */
        IBO();
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

        static void bind(GLuint ibo_id);

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

    // Ignore warning about STL exports as they're private members
#pragma warning(push, 2)
#pragma warning(disable: 4251)
#pragma warning(disable: 4275)

    /** Abstractisation of OpenGL vertex array objects (\b %VAO) and
     *  their creation, deletion, and binding.
     */
    struct SSS_GL_API VAO {
        /** Constructor, creates an \b OpenGL vertex array and
         *  sets #id accordingly.
         *  Forces to be bound to a Window instance.
         *  @sa ~VAO()
         */
        VAO();
        /** Destructor, deletes the \b OpenGL vertex array
         *  of corresponding #id.
         *  @sa VAO()
         */
        ~VAO();

        void setup(std::function<void()> f);
        
        /** Binds the vertex array to the context in which it was created.
         *  Effectively calls \c glBindVertexArray() with #id.
         */
        void bind();

        static void bind(GLuint vao_id);

        /** Unbinds any vertex array bound to the context in which this was created.
         *  Effectively calls \c glBindVertexArray() with \c 0.
         */
        void unbind() const;

    private:
        static GLuint _create();
        std::map<GLFWwindow*, GLuint> _ids;
        std::function<void()> _setup_func;
    };

#pragma warning(pop)
}

SSS_GL_END;

#endif // SSS_GL_BASIC_HPP