/**
 * Copyright (C) 2013 - 2019 : Kathrin Hanauer
 *
 * This file is part of Algora.
 *
 * Algora is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Algora is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Algora.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Contact information:
 *   http://algora.xaikal.org
 */

#ifndef ESTREEMLTIMESTAMPSFPM_H
#define ESTREEMLTIMESTAMPSFPM_H

#include "algorithm.reachability.ss/dynamicsinglesourcereachabilityalgorithm.h"
#include "esvertexdata.h"
#include "property/propertymap.h"
#include "property/fastpropertymap.h"
#include "simpleestree_timestamps.h"
#include <sstream>
#include <boost/circular_buffer.hpp>

namespace Algora {

template<bool reverseArcDirection = false, bool preferOlder = true>
class ESTreeMLTimeStampsFPM : public DynamicSingleSourceReachabilityAlgorithm
{
public:
    typedef std::tuple<unsigned int, double> ParameterSet;

    explicit ESTreeMLTimeStampsFPM(unsigned int requeueLimit = 5, double maxAffectedRatio = 0.5);
    explicit ESTreeMLTimeStampsFPM(const ParameterSet &params);
    virtual ~ESTreeMLTimeStampsFPM() override;
    void setRequeueLimit(unsigned int limit) {
        requeueLimit = limit;
    }
    void setMaxAffectedRatio(double ratio) {
        maxAffectedRatio = ratio;
    }
    void setDyDiGraph(const DynamicDiGraph*);

    // DiGraphAlgorithm interface
public:
    virtual void run() override;
    virtual std::string getName() const noexcept override {
      std::stringstream ss;
            ss << "Multilevel ES-Tree Single-Source Reachability Algorithm (";
      ss << requeueLimit << "/" << maxAffectedRatio << ")";
      return ss.str();
		}
    virtual std::string getShortName() const noexcept override {
      std::stringstream ss;
            ss << "ML-EST-DSSR(";
      ss << requeueLimit << "/" << maxAffectedRatio << ")";
      return ss.str();
		}
    virtual std::string getProfilingInfo() const override;
    virtual Profile getProfile() const override;

    // DynamicDiGraphAlgorithm interface
public:
    virtual void onVertexAdd(Vertex *v) override;
    virtual void onArcAdd(Arc *a) override;
    virtual void onVertexRemove(Vertex *v) override;
    virtual void onArcRemove(Arc *a) override;

protected:
    virtual void onDiGraphSet() override;
    virtual void onDiGraphUnset() override;

    // DynamicSSReachAlgorithm interface
protected:
    virtual void onSourceSet() override;

    // DynamicSSReachAlgorithm interface
public:
    virtual bool query(const Vertex *t) override;
    virtual std::vector<Arc*> queryPath(const Vertex *t) override;
    virtual void dumpData(std::ostream &os) const override;

private:
    typedef boost::circular_buffer<ESVertexData*> PriorityQueue;

    FastPropertyMap<ESVertexData*> data;
    FastPropertyMap<DiGraph::size_type> inNeighborIndices;
    FastPropertyMap<bool> reachable;
    FastPropertyMap<unsigned int> timesInQueue;
		PriorityQueue queue;
    const DynamicDiGraph* dyDiGraph;

    Vertex *root;
    bool initialized;
    unsigned int requeueLimit;
    double maxAffectedRatio;

    profiling_counter movesDown;
    profiling_counter movesUp;
    profiling_counter levelIncrease;
    profiling_counter levelDecrease;
    DiGraph::size_type maxLevelIncrease;
    DiGraph::size_type maxLevelDecrease;
    profiling_counter decUnreachableHead;
    profiling_counter decNonTreeArc;
    profiling_counter incUnreachableTail;
    profiling_counter incNonTreeArc;
    profiling_counter reruns;
    unsigned int maxReQueued;
    DiGraph::size_type maxAffected;
    profiling_counter totalAffected;
    profiling_counter rerunRequeued;
    profiling_counter rerunNumAffected;

    void restoreTree(ESVertexData *rd);
    void cleanup(bool freeSpace);
    void dumpTree(std::ostream &os);
    bool checkTree();
    void rerun();
    DiGraph::size_type process(ESVertexData *vd, bool &limitReached);
};

// explicit instantiation declaration
extern template class ESTreeMLTimeStampsFPM<false>;
extern template class ESTreeMLTimeStampsFPM<true>;
}

#endif // ESTREEML_H
