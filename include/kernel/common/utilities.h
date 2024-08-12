#ifndef UTILITIES_H
#define UTILITIES_H

namespace Common
{
    template< class A > struct remove_reference      {typedef A type;};
    template< class A > struct remove_reference<A&>  {typedef A type;};
    template< class A > struct remove_reference<A&&> {typedef A type;};   
    template <typename A>
    typename remove_reference<A>::type&& move(A&& arg)
    {
        return static_cast<typename remove_reference<A>::type&&>(arg);
    };

    template<typename A>
    constexpr A&& forward(remove_reference<A>& arg)
    {
        return static_cast<A&&>(arg);
    }

    template<typename A>
    constexpr A&& forward(remove_reference<A>&& arg)
    {
        return static_cast<A&&>(arg);
    }
}

#endif
