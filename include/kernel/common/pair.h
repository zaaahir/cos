#ifndef PAIR_H
#define PAIR_H
namespace Common {
    template<typename T, typename U>
    class Pair
    {
    public:
        Pair(const T& first, const U& last) : first(first), last(last) {}
        T first;
        U last;
    };
}
#endif
