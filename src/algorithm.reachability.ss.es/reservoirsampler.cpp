#include "reservoirsampler.h"

#include <cmath>

Algora::ReservoirSampler::ReservoirSampler() : mersenne_twister_engine{std::random_device{}()}, w{generateRandomNumber(mersenne_twister_engine)}
{
}

void Algora::ReservoirSampler::provide(ArcParentPair ap)
{
    if (currentValueIndex == nextReservoirValueIndex)
    {
        reservoirValue = ap;
        w = w * generateRandomNumber(mersenne_twister_engine);
        nextReservoirValueIndex += std::floor(std::log(generateRandomNumber(mersenne_twister_engine)) / std::log(1 - w)) + 1;
    }
    currentValueIndex++;
}

const Algora::ReservoirSampler::ArcParentPair &Algora::ReservoirSampler::sample() const
{
    return reservoirValue;
}

void Algora::ReservoirSampler::reset()
{
    currentValueIndex = 0;
    nextReservoirValueIndex = 0;
    reservoirValue = {};
    w = generateRandomNumber(mersenne_twister_engine);
}