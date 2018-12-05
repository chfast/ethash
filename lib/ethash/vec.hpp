#include <initializer_list>

template <typename T, unsigned N>
struct vec
{
    T elems[N];

    constexpr vec() = default;

    vec(std::initializer_list<T> init) noexcept
    {
        int i = 0;
        for (const auto elem : init)
            elems[i++] = elem;
    }

    T& operator[](std::size_t index) noexcept { return elems[index]; }
    T operator[](std::size_t index) const noexcept { return elems[index]; }
};

template <typename T, unsigned N, typename Fn>
vec<T, N> transform(const vec<T, N>& u, const vec<T, N>& v, Fn fn) noexcept
{
    vec<T, N> z;
    for (std::size_t i = 0; i < N; ++i)
        z[i] = fn(u[i], v[i]);
    return z;
}

template <typename T, unsigned N, typename Fn>
vec<T, N> transform(const vec<T, N>& u, T c, Fn fn) noexcept
{
    vec<T, N> z;
    for (std::size_t i = 0; i < N; ++i)
        z[i] = fn(u[i], c);
    return z;
}

template <typename T, unsigned N, typename Fn>
vec<T, N> transform(const vec<T, N>& u, Fn fn) noexcept
{
    vec<T, N> z;
    for (std::size_t i = 0; i < N; ++i)
        z[i] = fn(u[i]);
    return z;
}

template <typename T, unsigned N>
vec<T, N> operator+(const vec<T, N>& u, const vec<T, N>& v) noexcept
{
    return transform(u, v, [](T x, T y) { return x + y; });
}

template <typename T, unsigned N>
vec<T, N> operator-(const vec<T, N>& u, const vec<T, N>& v) noexcept
{
    return transform(u, v, [](T x, T y) { return x - y; });
}

template <typename T, unsigned N>
vec<T, N> operator*(const vec<T, N>& u, const vec<T, N>& v) noexcept
{
    return transform(u, v, [](T x, T y) { return x * y; });
}

template <typename T, unsigned N>
vec<T, N> operator&(const vec<T, N>& u, const vec<T, N>& v) noexcept
{
    return transform(u, v, [](T x, T y) { return x & y; });
}

template <typename T, unsigned N>
vec<T, N> operator|(const vec<T, N>& u, const vec<T, N>& v) noexcept
{
    return transform(u, v, [](T x, T y) { return x | y; });
}

template <typename T, unsigned N>
vec<T, N> operator^(const vec<T, N>& u, const vec<T, N>& v) noexcept
{
    return transform(u, v, [](T x, T y) { return x ^ y; });
}

template <typename T, unsigned N>
vec<T, N> operator<<(const vec<T, N>& u, unsigned c) noexcept
{
    return transform(u, T{c}, [](T x, T y) { return x << y; });
}

template <typename T, unsigned N>
vec<T, N> operator>>(const vec<T, N>& u, unsigned c) noexcept
{
    return transform(u, T{c}, [](T x, T y) { return x >> y; });
}

template <typename T, unsigned N>
vec<T, N> operator^(const vec<T, N>& u, T c) noexcept
{
    return transform(u, c, [](T x, T y) { return x ^ y; });
}

template <typename T, unsigned N>
vec<T, N> operator~(const vec<T, N>& u) noexcept
{
    return transform(u, [](T x) { return ~x; });
}