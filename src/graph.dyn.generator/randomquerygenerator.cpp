#include "randomquerygenerator.h"

#include <functional>

namespace Algora {

RandomQueryGenerator::RandomQueryGenerator()
    : absoluteQueries(0ULL), relativeQueries(0.0),
      relateTo(NUM_QUERY_RELATION::OPS_IN_DELTA), seed(0ULL), initialized(false)
{

}

RandomQueryGenerator::VertexQueryList RandomQueryGenerator::generateVertexQueries(const DynamicDiGraph *dyGraph)
{
    init();

    auto numQueries = computeNumQueries(dyGraph);
    VertexQueryList queries;
    // assuming that vertex ids are consecutive
    std::uniform_int_distribution<DynamicDiGraph::VertexIdentifier> distVertex(0, dyGraph->getCurrentGraphSize() - 1);
    auto randomVertex = std::bind(distVertex, std::ref(gen));

    for (auto i = numQueries; i > 0; i--) {
        queries.push_back(randomVertex());
    }

    return queries;
}

std::vector<RandomQueryGenerator::VertexQueryList> RandomQueryGenerator::generateAllVertexQueries(DynamicDiGraph *dyGraph)
{
    init();
    std::vector<VertexQueryList> queriesSet;

    // assuming that vertex ids are consecutive
    std::uniform_int_distribution<DynamicDiGraph::VertexIdentifier> distVertex(0, dyGraph->getCurrentGraphSize() - 1);
    auto randomVertex = std::bind(distVertex, std::ref(gen));

    dyGraph->resetToBigBang();
    auto sumQueries = 0Ull;

    while (dyGraph->applyNextDelta()) {
        queriesSet.emplace_back();
        auto &queries = queriesSet.back();
        auto numQueries = computeNumQueries(dyGraph);
        sumQueries += numQueries;
        for (auto i = 0Ull; i < numQueries; i++) {
            queries.push_back(randomVertex());
        }
    }

    // Special treatment for NOOPs at end
    if (dyGraph->lastOpWasNoop() && queriesSet.back().empty()) {
        queriesSet.pop_back();
    }

    return queriesSet;

}

void RandomQueryGenerator::init()
{
    if (initialized) {
        return;
    }

    if (seed == 0U) {
        std::random_device rd;
        seed = rd();
    }

    gen.seed(seed);
    initialized = true;
}

unsigned long long RandomQueryGenerator::computeNumQueries(const DynamicDiGraph *dyGraph) const
{
    auto numQueries = absoluteQueries;
    if (relativeQueries > 0.0) {
        auto tsCur = dyGraph->getCurrentTime();
        auto tsNext = dyGraph->getTimeOfXthNextDelta(1, true);
        if (tsNext == tsCur) {
            return 0ULL;
        }
        switch (relateTo) {
        case NUM_QUERY_RELATION::TIMEDIFF_IN_DELTA:
            numQueries = tsNext - tsCur;
            break;
        case NUM_QUERY_RELATION::OPS_IN_DELTA:
            numQueries = dyGraph->countArcAdditions(tsCur, tsCur) + dyGraph->countArcRemovals(tsCur, tsCur);
            break;
        }
    }
    return numQueries;
}

}
