#ifndef NFA_HEADER
#define NFA_HEADER

#include <vector>

template<typename T> class adjacency_matrix
{
    std::vector<std::vector<T>> matrix_;

public:
    explicit adjacency_matrix(int size)
    {

    }
};

template<typename T>
using graph = adjacency_matrix<T>;

#endif
