#include "reservoirsampler.h"

#include <cmath>
#include <cstdlib>


template<typename RandomEngine> 
Algora::ReservoirSampler<RandomEngine>::ReservoirSampler() : randomEngine{std::random_device{}()}, w{generateRandomNumber(randomEngine)}
{
}

template<typename RandomEngine> 
void Algora::ReservoirSampler<RandomEngine>::provide(ArcParentPair ap)
{
    if (currentValueIndex == nextReservoirValueIndex)
    {
        reservoirValue = ap;
        w = w * generateRandomNumber(randomEngine);
        nextReservoirValueIndex += std::floor(std::log(generateRandomNumber(randomEngine)) / std::log(1 - w)) + 1;
    }
    currentValueIndex++;
}

template<typename RandomEngine> 
const typename Algora::ReservoirSampler<RandomEngine>::ArcParentPair &Algora::ReservoirSampler<RandomEngine>::sample() const
{
    return reservoirValue;
}

template<typename RandomEngine> 
void Algora::ReservoirSampler<RandomEngine>::reset()
{
    currentValueIndex = 0;
    nextReservoirValueIndex = 0;
    reservoirValue = {};
    w = generateRandomNumber(randomEngine);
}