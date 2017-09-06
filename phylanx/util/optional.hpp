//  Copyright (c) 2017 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_UTIL_OPTIONAL_HPP)
#define PHYLANX_UTIL_OPTIONAL_HPP

#include <exception>
#include <string>
#include <type_traits>
#include <utility>

namespace phylanx { namespace util
{
    struct nullopt_t
    {
        struct init
        {
        };
        constexpr explicit nullopt_t(nullopt_t::init) {}
    };
    constexpr nullopt_t nullopt{nullopt_t::init()};

    struct in_place_t
    {
    };
    constexpr struct in_place_t in_place{};

    class bad_optional_access : public std::logic_error
    {
    public:
        explicit bad_optional_access(std::string const& what_arg)
          : logic_error(what_arg)
        {
        }

        explicit bad_optional_access(char const* what_arg)
          : logic_error(what_arg)
        {
        }
    };

    template <typename T>
    class optional
    {
    public:
        using value_type = T;

        constexpr optional() noexcept
          : empty_(true)
        {
        }

        constexpr optional(nullopt_t) noexcept
          : empty_(true)
        {
        }

        optional(optional const& other)
          : empty_(true)
        {
            if (!other.empty_)
            {
                new (&storage_) T(other.value());
                empty_ = false;
            }
        }
        optional(optional && other)
                noexcept(std::is_nothrow_move_constructible<T>::value)
          : empty_(true)
        {
            if (!other.empty_)
            {
                new (&storage_) T(std::move(other.value()));
                empty_ = false;
            }
        }

        optional(T const& val)
          : empty_(true)
        {
            new (&storage_) T(val);
            empty_ = false;
        }
        optional(T && val)
                noexcept(std::is_nothrow_move_constructible<T>::value)
          : empty_(true)
        {
            new (&storage_) T(std::move(val));
            empty_ = false;
        }

        template <typename ... Ts>
        explicit optional(in_place_t, Ts &&... ts)
          : empty_(true)
        {
            new (&storage_) T(std::forward<Ts>(ts)...);
            empty_ = false;
        }

        template <typename U, typename ... Ts>
        explicit optional(in_place_t, std::initializer_list<U> il, Ts &&... ts)
          : empty_(true)
        {
            new (&storage_) T(il, std::forward<Ts>(ts)...);
            empty_ = false;
        }

        ~optional()
        {
            reset();
        }

        optional &operator=(optional const& other)
        {
            optional tmp(other);
            swap(tmp);

            return *this;
        }
        optional &operator=(optional && other)
            noexcept(std::is_nothrow_move_assignable<T>::value &&
                std::is_nothrow_move_constructible<T>::value)
        {
            if (empty_)
            {
                if (!other.empty_)
                {
                    new (&storage_) T(std::move(other.value()));
                    empty_ = false;
                }
            }
            else
            {
                if (other.empty_)
                {
                    reinterpret_cast<T*>(&storage_)->~T();
                    empty_ = true;
                }
                else
                {
                    **this = std::move(other.value());
                }
            }
            return *this;
        }

        optional &operator=(T const& other)
        {
            optional tmp(other);
            swap(tmp);

            return *this;
        }
        optional &operator=(T && other)
        {
            if (!empty_)
            {
                reinterpret_cast<T*>(&storage_)->~T();
                empty_ = true;
            }

            new (&storage_) T(std::move(other));
            empty_ = false;

            return *this;
        }

        optional &operator=(nullopt_t) noexcept
        {
            if (!empty_)
            {
                reinterpret_cast<T*>(&storage_)->~T();
                empty_ = true;
            }
            return *this;
        }

        ///////////////////////////////////////////////////////////////////////
        constexpr T const * operator->() const noexcept
        {
            return reinterpret_cast<T const*>(&storage_);
        }

        T* operator->() noexcept
        {
            return reinterpret_cast<T*>(&storage_);
        }

        constexpr T const& operator*() const noexcept
        {
            return *reinterpret_cast<T const*>(&storage_);
        }

        T& operator*() noexcept
        {
            return *reinterpret_cast<T*>(&storage_);
        }

        constexpr explicit operator bool() const noexcept
        {
            return !empty_;
        }

        constexpr bool has_value() const
        {
            return !empty_;
        }

        T& value()
        {
            if (empty_)
            {
                throw bad_optional_access(
                    "object is empty during call to 'value()'");
            }
            return **this;
        }

        T const& value() const
        {
            if (empty_)
            {
                throw bad_optional_access(
                    "object is empty during call to 'value()'");
            }
            return **this;
        }

        template <typename U>
        constexpr T value_or(U && value) const
        {
            if (empty_)
                return std::forward<U>(value);
            return **this;
        }

        template <typename ... Ts>
        void emplace(Ts &&... ts)
        {
            if (!empty_)
            {
                reinterpret_cast<T*>(&storage_)->~T();
                empty_ = true;
            }
            new (&storage_) T(std::forward<Ts>(ts)...);
            empty_ = false;
        }

        void swap(optional& other)
            noexcept(std::is_nothrow_move_constructible<T>::value &&
                noexcept(swap(std::declval<T&>(), std::declval<T&>())))
        {
            // do nothing if both are empty
            if (empty_ && other.empty_)
            {
                return;
            }
            // swap content if both are non-empty
            if (!empty_ && !other.empty_)
            {
                swap(**this, *other);
                return;
            }

            // move the non-empty one into the empty one and make remains empty
            optional* empty = empty_ ? this : &other;
            optional* non_empty = empty_ ? &other : this;

            new (&empty->storage_) T(std::move(**non_empty));
            reinterpret_cast<T*>(&non_empty->storage_)->~T();

            empty->empty_ = false;
            non_empty->empty_ = true;
        }

        void reset()
        {
            if (!empty_)
            {
                reinterpret_cast<T*>(&storage_)->~T();
            }
        }

    private:
        typename std::aligned_storage<sizeof(T), alignof(T)>::type storage_;
        bool empty_;
    };

    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    constexpr bool operator==(optional<T> const& lhs, optional<T> const& rhs)
    {
        if (bool(lhs) != bool(rhs))
        {
            return false;
        }
        if (!bool(lhs) && !bool(rhs))
        {
            return true;
        }
        return *lhs == *rhs;
    }

    template <typename T>
    constexpr bool operator!=(optional<T> const& lhs, optional<T> const& rhs)
    {
        return !(lhs == rhs);
    }

    template <typename T>
    constexpr bool operator<(optional<T> const& lhs, optional<T> const& rhs)
    {
        if (!bool(rhs))
        {
            return false;
        }
        if (!bool(lhs))
        {
            return true;
        }
        return *rhs < *lhs;
    }

    template <typename T>
    constexpr bool operator>=(optional<T> const& lhs, optional<T> const& rhs)
    {
        return !(lhs < rhs);
    }

    template <typename T>
    constexpr bool operator>(optional<T> const& lhs, optional<T> const& rhs)
    {
        if (!bool(lhs))
        {
            return false;
        }
        if (!bool(rhs))
        {
            return true;
        }
        return *rhs > *lhs;
    }

    template <typename T>
    constexpr bool operator<=(optional<T> const& lhs, optional<T> const& rhs)
    {
        return !(lhs > rhs);
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    constexpr bool operator==(optional<T> const& opt, nullopt_t) noexcept
    {
        return !bool(opt);
    }

    template <typename T>
    constexpr bool operator==(nullopt_t, optional<T> const& opt) noexcept
    {
        return !bool(opt);
    }

    template <typename T>
    constexpr bool operator!=(optional<T> const& opt, nullopt_t) noexcept
    {
        return bool(opt);
    }

    template <typename T>
    constexpr bool operator!=(nullopt_t, optional<T> const& opt) noexcept
    {
        return bool(opt);
    }

    template <typename T>
    constexpr bool operator<(optional<T> const& opt, nullopt_t) noexcept
    {
        return false;
    }

    template <typename T>
    constexpr bool operator<(nullopt_t, optional<T> const& opt) noexcept
    {
        return bool(opt);
    }

    template <typename T>
    constexpr bool operator>=(optional<T> const& opt, nullopt_t) noexcept
    {
        return true;
    }

    template <typename T>
    constexpr bool operator>=(nullopt_t, optional<T> const& opt) noexcept
    {
        return !bool(opt);
    }

    template <typename T>
    constexpr bool operator>(optional<T> const& opt, nullopt_t) noexcept
    {
        return bool(opt);
    }

    template <typename T>
    constexpr bool operator>(nullopt_t, optional<T> const& opt) noexcept
    {
        return false;
    }

    template <typename T>
    constexpr bool operator<=(optional<T> const& opt, nullopt_t) noexcept
    {
        return !bool(opt);
    }

    template <typename T>
    constexpr bool operator<=(nullopt_t, optional<T> const& opt) noexcept
    {
        return true;
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    constexpr bool operator==(optional<T> const& opt, T const& value)
    {
        return bool(opt) ? (*opt == value) : false;
    }

    template <typename T>
    constexpr bool operator==(T const& value, optional<T> const& opt)
    {
        return bool(opt) ? (value == *opt) : false;
    }

    template <typename T>
    constexpr bool operator!=(optional<T> const& opt, T const& value)
    {
        return !(opt == value);
    }

    template <typename T>
    constexpr bool operator!=(T const& value, optional<T> const& opt)
    {
        return !(value == *opt);
    }

    template <typename T>
    constexpr bool operator<(optional<T> const& opt, T const& value)
    {
        return bool(opt) ? (*opt < value) : true;
    }

    template <typename T>
    constexpr bool operator<(T const& value, optional<T> const& opt)
    {
        return bool(opt) ? (value < *opt) : false;
    }

    template <typename T>
    constexpr bool operator>=(optional<T> const& opt, T const& value)
    {
        return !(*opt < value);
    }

    template <typename T>
    constexpr bool operator>=(T const& value, optional<T> const& opt)
    {
        return !(value < *opt);
    }

    template <typename T>
    constexpr bool operator>(optional<T> const& opt, T const& value)
    {
        return bool(opt) ? (*opt > value) : false;
    }

    template <typename T>
    constexpr bool operator>(T const& value, optional<T> const& opt)
    {
        return bool(opt) ? (value > *opt) : true;
    }

    template <typename T>
    constexpr bool operator<=(optional<T> const& opt, T const& value)
    {
        return !(*opt > value);
    }

    template <typename T>
    constexpr bool operator<=(T const& value, optional<T> const& opt)
    {
        return !(value > *opt);
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    void swap(optional<T>& x, optional<T>& y) noexcept(noexcept(x.swap(y)))
    {
        x.swap(y);
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    constexpr optional<std::decay_t<T>> make_optional(T && v)
    {
        return optional<std::decay_t<T>>(std::forward<T>(v));
    }

    template <typename T, typename ... Ts>
    constexpr optional<T> make_optional(Ts &&... ts)
    {
        return optional<T>(in_place, std::forward<Ts>(ts)...);
    }

    template <typename T, typename U, typename ... Ts>
    constexpr optional<T> make_optional(std::initializer_list<U> il, Ts &&... ts)
    {
        return optional<T>(in_place, il, std::forward<Ts>(ts)...);
    }
}}

///////////////////////////////////////////////////////////////////////////////
namespace std
{
    template <typename T>
    struct hash<phylanx::util::optional<T>>
    {
        typedef typename hash<T>::result_type result_type;
        typedef phylanx::util::optional<T> argument_type;

        constexpr result_type operator()(argument_type const& arg) const
        {
            return arg ? std::hash<T>{}(*arg) : result_type{};
        }
    };
}

#endif