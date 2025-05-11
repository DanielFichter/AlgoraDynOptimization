#include <iostream>
#include <vector>

template <typename T>
std::ostream &operator<<(std::ostream &o, const std::vector<T> &vector)
{
    o << "[";
    for (size_t index = 0; index < vector.size(); index++)
    {
        if (index)
        {
            o << ", ";
        }
        o << vector[index];
    }

    o << "]";

    return o;
}