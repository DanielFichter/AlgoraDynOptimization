#pragma once

#include <array>
#include <cstddef>
#include <stddef.h>


namespace Algora {

template <size_t bufferSize> class RandomSimulator {

private:
std::array<size_t, bufferSize> randomNumbers;
size_t rnIndex = 0;

public:
    void initialize();
    size_t sample();
};

extern template class RandomSimulator<1'000>;
} // namespace Algora