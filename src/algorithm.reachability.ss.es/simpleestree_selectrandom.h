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

#ifndef SIMPLEESTREESELECTRANDOM_H
#define SIMPLEESTREESELECTRANDOM_H

#include "algorithm.reachability.ss/dynamicsinglesourcereachabilityalgorithm.h"
#include "property/propertymap.h"
#include "property/fastpropertymap.h"
#include "sesvertexdata.h"
#include <sstream>
#include <random>
#include <utility>
#include <boost/circular_buffer.hpp>

namespace Algora {

class ArcPool
{
public:
    void insert(std::pair<Arc*, SESVertexData*> arc)
    {
        if (arraySize == nArcs)
        {
            arraySize *= 2;
            std::pair<Arc*, SESVertexData*>* newData = new std::pair<Arc*, SESVertexData*>[arraySize];
            for (size_t arcIndex = 0; arcIndex < nArcs; arcIndex++)
            {
                newData[arcIndex] = data[arcIndex];
            }
            delete[] data;
            data = newData;
        }
        data[nArcs] = arc;
        nArcs++;
    }

    void clear()
    {
        nArcs = 0;
    }

    ~ArcPool()
    {
        delete[] data;
    }

    bool empty() const
    {
        return nArcs == 0;
    }

    size_t size() const
    {
        return nArcs;
    }

    const std::pair<Arc*, SESVertexData*>& at(size_t index) const
    {
        return data[index];
    }

private:
    std::pair<Arc*, SESVertexData*>* data = new std::pair<Arc*, SESVertexData*>[5]{};
    size_t nArcs = 0;
    size_t arraySize = 5;
};


/*
 * The same as SimpleESTree with the difference that, if the parent of a node in the bfs tree
 * is removed from the graph, a new parent for that node in the bfs tree is selected at random
 * and not by 
*/
template<bool reverseArcDirection = false>
class SimpleESTreeSelectRandom : public DynamicSingleSourceReachabilityAlgorithm
{
public:
    // requeueLimit, maxAffectedRatio
    typedef std::tuple<unsigned int, double> ParameterSet;

    explicit SimpleESTreeSelectRandom(unsigned int requeueLimit = 5, double maxAffectedRatio = .5);
    explicit SimpleESTreeSelectRandom(const ParameterSet &params);
    virtual ~SimpleESTreeSelectRandom() override;
    void setRequeueLimit(unsigned int limit) {
        requeueLimit = limit;
    }
    void setMaxAffectedRatio(double ratio) {
        maxAffectedRatio = ratio;
    }
		DiGraph::size_type getDepthOfBFSTree() const;
		DiGraph::size_type getNumReachable() const;

    // DiGraphAlgorithm interface
public:
    virtual void run() override;
        virtual std::string getName() const noexcept override {
            std::stringstream ss;
            if (reverseArcDirection) {
                ss << "Simple ES-Tree Single-Sink Reachability Algorithm (";
            } else {
                ss << "Simple ES-Tree Single-Source Reachability Algorithm (";
            }
            ss << requeueLimit << "/" << maxAffectedRatio << ")";
            return ss.str();
        }
        virtual std::string getShortName() const noexcept override {
            std::stringstream ss;
            if (reverseArcDirection) {
                ss << "Reverse-Simple-EST-DSSR(";
            } else {
                ss << "Simple-EST-DSSR(";
            }
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
    typedef boost::circular_buffer<SESVertexData*> PriorityQueue;

    FastPropertyMap<SESVertexData*> data;
    FastPropertyMap<bool> reachable;
    FastPropertyMap<unsigned int> timesInQueue;
    PriorityQueue queue;

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
    ArcPool potentialTreeArcs;
    std::subtract_with_carry_engine<std::vector<Algora::Arc*>::size_type, 48, 5, 12> generateRandomNumber;

    void restoreTree(SESVertexData *rd);
    std::pair<Algora::Arc*, SESVertexData*> selectRandomTreeArc(const ArcPool& potentialTreeArcs);
    void cleanup(bool freeSpace);
    void dumpTree(std::ostream &os);
    bool checkTree();
    void rerun();
    DiGraph::size_type process(SESVertexData *vd, bool &limitReached);
};


}

#endif // SIMPLEESTREE_H
