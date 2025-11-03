#include "random_simulator.h"
#include <cstddef>
#include <random>

template <size_t bufferSize>
void Algora::RandomSimulator<bufferSize>::initialize() {
  std::mt19937 generateRandom;
  for (size_t index = 0; index < bufferSize; index++) {
    randomNumbers[index] = generateRandom();
  }
}

template <size_t bufferSize>
size_t Algora::RandomSimulator<bufferSize>::sample() {
  if (rnIndex == bufferSize - 1) {
    rnIndex = 0;
  } else {
    rnIndex++;
  }

  return randomNumbers[rnIndex];
}

namespace Algora {

template class RandomSimulator<1'000>;
}