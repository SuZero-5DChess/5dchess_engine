#ifndef GENERATOR_H
#define GENERATOR_H

// to replace std::generator since it is not supported by GCC/Clang for the moment

#include <coroutine>
#include <concepts>
#include <iterator>
#include <optional>

template<std::movable T>
class generator
{
public:
    struct promise_type
    {
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
        [[noreturn]]
        static void unhandled_exception() { throw; }
 
        std::optional<T> current_value;
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
