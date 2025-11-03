#ifndef RESERVOIRSAMPLER
#define RESERVOIRSAMPLER

#include <stddef.h>
#include <random>
#include <utility>

namespace Algora
{
    class Arc;
    class SESVertexData;

    template<typename RandomEngine>
    class ReservoirSampler
    {
    public:
        using ArcParentPair = std::pair<Arc *, SESVertexData *>;
        ReservoirSampler();
        void provide(ArcParentPair);
        const ArcParentPair &sample() const;
        void reset();

    private:
        ArcParentPair reservoirValue = {};
        RandomEngine randomEngine;
        std::uniform_real_distribution<float> generateRandomNumber{0., 1.};
        float w;
        size_t currentValueIndex = 0;
        size_t nextReservoirValueIndex = 0;
    };
}

#endif // RESERVOIRSAMPLER