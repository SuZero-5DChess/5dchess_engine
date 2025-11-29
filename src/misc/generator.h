#ifndef GENERATOR_H
#define GENERATOR_H

// to replace std::generator since it is not supported by GCC/Clang for the moment

#include <coroutine>
#include <concepts>
#include <iterator>
#include <optional>
#include <exception>

// for debugging exceptions in coroutine
#include <iostream>
#include <sstream>
// Detect C++23 <stacktrace>
#if __has_include(<stacktrace>) && defined(__cpp_lib_stacktrace) && __cpp_lib_stacktrace >= 202011
    #include <stacktrace>
    #define HAS_STD_STACKTRACE 1
#else
    #define HAS_STD_STACKTRACE 0
#endif

// Detect <execinfo.h>
#if __has_include(<execinfo.h>)
    #include <execinfo.h>
    #define HAS_EXECINFO 1
#else
    #define HAS_EXECINFO 0
#endif


// ==================================== //
//  Capture stack trace implementation  //
// ==================================== //
static inline std::string capture_stacktrace()
{
#if HAS_STD_STACKTRACE
    // -------- C++23 version ---------
    std::ostringstream oss;
    oss << std::stacktrace::current();
    return oss.str();

#elif HAS_EXECINFO
    // -------- execinfo.h version (Linux/macOS) ---------
    void* buf[64];
    int n = ::backtrace(buf, 64);
    char** symbols = ::backtrace_symbols(buf, n);

    std::ostringstream oss;
    for (int i = 0; i < n; ++i)
        oss << symbols[i] << '\n';

    free(symbols);
    return oss.str();

#else
    // -------- Fallback ---------
    return "<stacktrace unavailable>";
#endif
}


template<std::movable T>
class generator
{
public:
    struct promise_type
    {
        //std::exception_ptr exception_;
        std::optional<T> current_value;
        
        generator<T> get_return_object()
        {
            return generator{Handle::from_promise(*this)};
        }
        static std::suspend_always initial_suspend() noexcept
        {
            return {};
        }
        static std::suspend_always final_suspend() noexcept
        {
            return {};
        }
        std::suspend_always yield_value(T value) noexcept
        {
            current_value = std::move(value);
            return {};
        }
        void return_void() noexcept {}
        // Disallow co_await in generator coroutines.
        void await_transform() = delete;
        void unhandled_exception() {
            //exception_ = std::current_exception();
            std::string stack = capture_stacktrace();
            std::cerr << "\n" << stack << std::endl;
            throw;
        }
    };
 
    using Handle = std::coroutine_handle<promise_type>;
 
    explicit generator(const Handle coroutine) :
        m_coroutine{coroutine}
    {}
 
    generator() = default;
    ~generator()
    {
        if (m_coroutine)
            m_coroutine.destroy();
    }
 
    generator(const generator&) = delete;
    generator& operator=(const generator&) = delete;
 
    generator(generator&& other) noexcept :
        m_coroutine{other.m_coroutine}
    {
        other.m_coroutine = {};
    }
    generator& operator=(generator&& other) noexcept
    {
        if (this != &other)
        {
            if (m_coroutine)
                m_coroutine.destroy();
            m_coroutine = other.m_coroutine;
            other.m_coroutine = {};
        }
        return *this;
    }
 
    // Range-based for loop support.
    class iterator
    {
        Handle m_coroutine;
    public:
        void operator++()
        {
            m_coroutine.resume();
        }
        const T& operator*() const
        {
            return *m_coroutine.promise().current_value;
        }
        bool operator==(std::default_sentinel_t) const
        {
            return !m_coroutine || m_coroutine.done();
        }
 
        explicit iterator(const Handle coroutine) :
            m_coroutine{coroutine}
        {}
    };
 
    iterator begin()
    {
        if (m_coroutine)
            m_coroutine.resume();
        return iterator{m_coroutine};
    }
 
    std::default_sentinel_t end() { return {}; }
    
    std::optional<T> first()
    {
        if (m_coroutine)
        {
            m_coroutine.resume();
            if (!m_coroutine.done() && m_coroutine.promise().current_value)
            {
                return std::move(m_coroutine.promise().current_value);
            }
        }
        return std::nullopt;
    }

    // return the first generated value such that p(value) is true
    // or std::nullopt if nothing is found
    // important: the generator does not rewind
    template<std::predicate<T> P>
    std::optional<T> find(P &&p)
    {
        if (m_coroutine)
        {
            // resume if not started yet
            if (!m_coroutine.done())
            {
                // check if we haven't started iterating yet
                if (!m_coroutine.promise().current_value)
                {
                    m_coroutine.resume();
                }
                while (!m_coroutine.done())
                {
                    if (m_coroutine.promise().current_value &&
                        p(*m_coroutine.promise().current_value))
                    {
                        return std::move(m_coroutine.promise().current_value);
                    }
                    m_coroutine.resume();
                }
            }
        }
        return std::nullopt;
    }
private:
    Handle m_coroutine;
};

#endif // GENERATOR_H
