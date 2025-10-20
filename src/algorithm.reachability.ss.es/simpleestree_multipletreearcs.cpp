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

#include "simpleestree_multipletreearcs.h"
#include "graph/vertex.h"
#include "graph/arc.h"
#include "algorithm.basic.traversal/breadthfirstsearch.h"
#include "algorithm.basic.traversal/graphtraversal.h"
#include "algorithm/digraphalgorithmexception.h"
#include "io/printvector.h"

#include <vector>
#include <climits>
#include <cassert>

// #define DEBUG_SIMPLEESTREE

#include <iostream>
#ifdef DEBUG_SIMPLEESTREE
#define PRINT_DEBUG(msg) std::cerr << msg << std::endl;
#define IF_DEBUG(cmd) cmd;
#else
#define PRINT_DEBUG(msg) ((void)0);
#define IF_DEBUG(cmd)
#endif

namespace Algora
{

#ifdef DEBUG_SIMPLEESTREE
    void printQueue(boost::circular_buffer<SESVertexDataMultipleParents<maxNTreeArcs> *> q)
    {
        std::cerr << "PriorityQueue: ";
        while (!q.empty())
        {
            std::cerr << q.front()->vertex << "[" << q.front()->level << "]" << ", ";
            q.pop_front();
        }
        std::cerr << std::endl;
    }
#endif

    template <bool reverseArcDirection, unsigned maxNTreeArcs>
    SimpleESTreeMultipleTreeArcs<reverseArcDirection, maxNTreeArcs>::SimpleESTreeMultipleTreeArcs(unsigned int requeueLimit, double maxAffectedRatio)
        : SimpleESTreeMultipleTreeArcs<reverseArcDirection, maxNTreeArcs>(std::make_tuple(requeueLimit, maxAffectedRatio))
    {
    }

    template <bool reverseArcDirection, unsigned maxNTreeArcs>
    SimpleESTreeMultipleTreeArcs<reverseArcDirection, maxNTreeArcs>::SimpleESTreeMultipleTreeArcs(const SimpleESTreeMultipleTreeArcs<reverseArcDirection, maxNTreeArcs>::ParameterSet &params)
        : DynamicSingleSourceReachabilityAlgorithm(), root(nullptr),
          initialized(false), requeueLimit(std::get<0>(params)),
          maxAffectedRatio(std::get<1>(params)),
          movesDown(0U), movesUp(0U),
          levelIncrease(0U), levelDecrease(0U),
          maxLevelIncrease(0U), maxLevelDecrease(0U),
          decUnreachableHead(0U), decNonTreeArc(0U),
          incUnreachableTail(0U), incNonTreeArc(0U),
          reruns(0U), maxReQueued(0U),
          maxAffected(0U), totalAffected(0U),
          rerunRequeued(0U), rerunNumAffected(0U)
    {
        data.setDefaultValue(nullptr);
        reachable.setDefaultValue(false);

        timesInQueue.setDefaultValue(0U);
    }

    template <bool reverseArcDirection, unsigned maxNTreeArcs>
    SimpleESTreeMultipleTreeArcs<reverseArcDirection, maxNTreeArcs>::~SimpleESTreeMultipleTreeArcs()
    {
        cleanup(true);
    }

    template <bool reverseArcDirection, unsigned maxNTreeArcs>
    DiGraph::size_type SimpleESTreeMultipleTreeArcs<reverseArcDirection, maxNTreeArcs>::getDepthOfBFSTree() const
    {
        DiGraph::size_type maxLevel = 0U;
        diGraph->mapVertices([&](Vertex *v)
                             {
         if (reachable(v) && data(v)->level > maxLevel) {
             maxLevel = data(v)->level;
         } });
        return maxLevel;
    }

    template <bool reverseArcDirection, unsigned maxNTreeArcs>
    DiGraph::size_type SimpleESTreeMultipleTreeArcs<reverseArcDirection, maxNTreeArcs>::getNumReachable() const
    {
        DiGraph::size_type numR = 0U;
        diGraph->mapVertices([&](Vertex *v)
                             {
         if (reachable(v)) {
             numR++;
         } });
        return numR;
    }

    template <bool reverseArcDirection, unsigned maxNTreeArcs>
    void SimpleESTreeMultipleTreeArcs<reverseArcDirection, maxNTreeArcs>::run()
    {
        if (initialized)
        {
            return;
        }

        PRINT_DEBUG("Initializing SimpleESTreeMultipleTreeArcs...");

        if (reachable.size() < diGraph->getSize())
        {
            reachable.resetAll(diGraph->getSize());
        }
        else
        {
            reachable.resetAll();
        }

        BreadthFirstSearch<FastPropertyMap, false, reverseArcDirection> bfs(false);
        root = source;
        if (root == nullptr)
        {
            root = diGraph->getAnyVertex();
        }
        bfs.setStartVertex(root);
        if (data[root] == nullptr)
        {
            data[root] = new SESVertexDataMultipleParents<maxNTreeArcs>(root, {}, {}, 0);
        }
        else
        {
            data[root]->reset({}, {}, 0);
        }
        reachable[root] = true;

        bfs.onArcDiscover([this](const Arc *a)
                          {
#ifdef COLLECT_PR_DATA
                              prVertexConsidered();
                              prArcConsidered();
#endif
                              Arc* mutableA = const_cast<Arc*>(a);
                              Vertex *t;
                              Vertex *h;
                              if (reverseArcDirection)
                              {
                                  t = a->getHead();
                                  h = a->getTail();
                              }
                              else
                              {
                                  t = a->getTail();
                                  h = a->getHead();
                              }
                              auto* hd = data[h];
                              auto* td = data[t]; 

                              if (hd == nullptr)
                              {
                                  data[h] = new SESVertexDataMultipleParents<maxNTreeArcs>(h, std::array<SESVertexDataMultipleParents<maxNTreeArcs>*, maxNTreeArcs>{td},
                                     std::array<Arc*, maxNTreeArcs>{mutableA});
                              }
                              else
                              {
                                  if (hd->hasAnyParent())
                                  {
                                      // TODO: also consider that multiarcs can exist, i.e. the parent can be the same for multiple arcs
                                      if (td->level == hd->getParentData(0)->level && !hd->isParent(td))
                                      {
                                        hd->tryAddParent(td, mutableA);
                                      }
                                  }
                                  else
                                  {
                                      hd->reset(std::array<SESVertexDataMultipleParents<maxNTreeArcs>*, maxNTreeArcs>{td}, std::array<Arc*, maxNTreeArcs>{mutableA});
                                  }
                              }
                              reachable[h] = true;
                              PRINT_DEBUG("(" << a->getTail() << ", " << a->getHead() << ")" << " is a tree arc.")
#ifdef COLLECT_PR_DATA
                              prArcConsidered();
#endif
                              return true; });
        runAlgorithm(bfs, diGraph);

        diGraph->mapVertices([this](Vertex *v)
                             {
#ifdef COLLECT_PR_DATA
         prVertexConsidered();
#endif
        if (data(v) == nullptr) {
            data[v] = new SESVertexDataMultipleParents<maxNTreeArcs>(v);
            PRINT_DEBUG( v << " is a unreachable.")
        } });

        initialized = true;
        PRINT_DEBUG("Initializing completed.");

        IF_DEBUG(
            if (!checkTree()) {
                std::cerr.flush();
                dumpTree(std::cerr);
                std::cerr.flush();
            });
        assert(checkTree());
    }

    template <bool reverseArcDirection, unsigned maxNTreeArcs>
    std::string SimpleESTreeMultipleTreeArcs<reverseArcDirection, maxNTreeArcs>::getProfilingInfo() const
    {
        std::stringstream ss;
#ifdef COLLECT_PR_DATA
        ss << DynamicSingleSourceReachabilityAlgorithm::getProfilingInfo();
        ss << "#moves down (level increase): " << movesDown << std::endl;
        ss << "#moves up (level decrease): " << movesUp << std::endl;
        ss << "total level increase: " << levelIncrease << std::endl;
        ss << "total level decrease: " << levelDecrease << std::endl;
        ss << "maximum level increase: " << maxLevelIncrease << std::endl;
        ss << "maximum level decrease: " << maxLevelDecrease << std::endl;
        ss << "#unreachable head (dec): " << decUnreachableHead << std::endl;
        ss << "#non-tree arcs (dec): " << decNonTreeArc << std::endl;
        ss << "#unreachable tail (inc): " << incUnreachableTail << std::endl;
        ss << "#non-tree arcs (inc): " << incNonTreeArc << std::endl;
        ss << "requeue limit: " << requeueLimit << std::endl;
        ss << "maximum #requeuings: " << maxReQueued << std::endl;
        ss << "maximum ratio of affected vertices: " << maxAffectedRatio << std::endl;
        ss << "total affected vertices: " << totalAffected << std::endl;
        ss << "maximum number of affected vertices: " << maxAffected << std::endl;
        ss << "#reruns: " << reruns << std::endl;
        ss << "#reruns because requeue limit reached: " << rerunRequeued << std::endl;
        ss << "#reruns because max. number of affected vertices reached: " << rerunNumAffected << std::endl;
#endif
        return ss.str();
    }

    template <bool reverseArcDirection, unsigned maxNTreeArcs>
    DynamicSingleSourceReachabilityAlgorithm::Profile SimpleESTreeMultipleTreeArcs<reverseArcDirection, maxNTreeArcs>::getProfile() const
    {
        auto profile = DynamicSingleSourceReachabilityAlgorithm::getProfile();
        profile.push_back(std::make_pair(std::string("vertices_moved_down"), movesDown));
        profile.push_back(std::make_pair(std::string("vertices_moved_up"), movesUp));
        profile.push_back(std::make_pair(std::string("total_level_increase"), levelIncrease));
        profile.push_back(std::make_pair(std::string("total_level_decrease"), levelDecrease));
        profile.push_back(std::make_pair(std::string("max_level_increase"), maxLevelIncrease));
        profile.push_back(std::make_pair(std::string("max_level_decrease"), maxLevelDecrease));
        profile.push_back(std::make_pair(std::string("dec_head_unreachable"), decUnreachableHead));
        profile.push_back(std::make_pair(std::string("dec_nontree"), decNonTreeArc));
        profile.push_back(std::make_pair(std::string("inc_tail_unreachable"), incUnreachableTail));
        profile.push_back(std::make_pair(std::string("inc_nontree"), incNonTreeArc));
        profile.push_back(std::make_pair(std::string("requeue_limit"), requeueLimit));
        profile.push_back(std::make_pair(std::string("max_affected"), maxAffectedRatio));
        profile.push_back(std::make_pair(std::string("max_requeued"), maxReQueued));
        profile.push_back(std::make_pair(std::string("total_affected"), totalAffected));
        profile.push_back(std::make_pair(std::string("max_affected"), maxAffected));
        profile.push_back(std::make_pair(std::string("rerun"), reruns));
        profile.push_back(std::make_pair(std::string("rerun_requeue_limit"), rerunRequeued));
        profile.push_back(std::make_pair(std::string("rerun_max_affected"), rerunNumAffected));
        return profile;
    }

    template <bool reverseArcDirection, unsigned maxNTreeArcs>
    void SimpleESTreeMultipleTreeArcs<reverseArcDirection, maxNTreeArcs>::onDiGraphSet()
    {
        DynamicSingleSourceReachabilityAlgorithm::onDiGraphSet();
        cleanup(false);

        movesDown = 0U;
        movesUp = 0U;
        levelIncrease = 0U;
        levelDecrease = 0U;
        decUnreachableHead = 0U;
        decNonTreeArc = 0U;
        incUnreachableTail = 0U;
        incNonTreeArc = 0U;
        reruns = 0U;
        maxReQueued = 0U;
        maxAffected = 0U;
        totalAffected = 0U;
        rerunRequeued = 0U;
        rerunNumAffected = 0U;
        // called by cleanup
    }

    template <bool reverseArcDirection, unsigned maxNTreeArcs>
    void SimpleESTreeMultipleTreeArcs<reverseArcDirection, maxNTreeArcs>::onDiGraphUnset()
    {
        DynamicSingleSourceReachabilityAlgorithm::onDiGraphUnset();
        cleanup(true);
    }

    template <bool reverseArcDirection, unsigned maxNTreeArcs>
    void SimpleESTreeMultipleTreeArcs<reverseArcDirection, maxNTreeArcs>::onVertexAdd(Vertex *v)
    {
        if (!initialized)
        {
            return;
        }

        assert(data(v) == nullptr);

        data[v] = new SESVertexDataMultipleParents<maxNTreeArcs>(v);
    }

    template <bool reverseArcDirection, unsigned maxNTreeArcs>
    void SimpleESTreeMultipleTreeArcs<reverseArcDirection, maxNTreeArcs>::onArcAdd(Arc *a)
    {
        if (!initialized)
        {
            return;
        }

        PRINT_DEBUG("An arc has been added: (" << a->getTail() << ", " << a->getHead() << ")")

        if (a->isLoop())
        {
            PRINT_DEBUG("Arc is a loop.")
            return;
        }

        Vertex *tail;
        Vertex *head;
        if (reverseArcDirection)
        {
            tail = a->getHead();
            head = a->getTail();
        }
        else
        {
            tail = a->getTail();
            head = a->getHead();
        }

        if (head == source)
        {
            PRINT_DEBUG("Head is source.")
            return;
        }

        auto td = data(tail);
        auto hd = data(head);

        assert(td != nullptr);
        assert(hd != nullptr);

        if (!td->isReachable())
        {
            PRINT_DEBUG("Tail is unreachable.")
#ifdef COLLECT_PR_DATA
            incUnreachableTail++;
#endif
            return;
        }

#ifdef COLLECT_PR_DATA
        auto n = diGraph->getSize();
#endif

        // update...

        if (hd->level < td->level + 1)
        {
            // arc does not change anything
#ifdef COLLECT_PR_DATA
            incNonTreeArc++;
#endif
            PRINT_DEBUG("Not a tree arc.")
            return;
        }
        if (hd->level == td->level + 1)
        {
            PRINT_DEBUG("Is a new tree arc.")
            hd->tryAddParent(td, a);
        }
        else
        {
            PRINT_DEBUG("Is a new tree arc.")
#ifdef COLLECT_PR_DATA
            movesUp++;
            if (!hd->isReachable())
            {
                levelDecrease += (n - (td->level + 1));
            }
            else
            {
                levelDecrease += (hd->level - (td->level + 1));
            }
#endif
            hd->replaceParents(td, a);
            reachable[head] = true;

            BreadthFirstSearch<FastPropertyMap, false, reverseArcDirection> bfs(false);
            bfs.setStartVertex(head);
#ifdef COLLECT_PR_DATA
            bfs.onArcDiscover([this, n](const Arc *a)
                              {
#else
            bfs.onArcDiscover([this](const Arc *a)
                              {
#endif
            PRINT_DEBUG( "Discovering arc (" << a->getTail() << ", " << a->getHead() << ")...");
#ifdef COLLECT_PR_DATA
         prArcConsidered();
#endif
            if (a->isLoop()) {
                PRINT_DEBUG( "Loop ignored.");
                return false;
            }
            Vertex *at;
            Vertex *ah;
            if (reverseArcDirection) {
                at = a->getHead();
                ah = a->getTail();
            } else {
                at = a->getTail();
                ah = a->getHead();
            }
            auto atd = data(at);
            auto ahd = data(ah);

#ifdef COLLECT_PR_DATA
         prVertexConsidered();
#endif
            if (!ahd->isReachable() || atd->level + 1 < ahd->level) {
#ifdef COLLECT_PR_DATA
             movesUp++;
             auto newLevel = atd->level + 1;
             auto dec = n - newLevel;
             if (ahd->isReachable()) {
                 dec = ahd->level - newLevel;
             }
             levelDecrease += dec;
             if (dec > maxLevelDecrease) {
                 maxLevelDecrease = dec;
             }
#endif
             ahd->replaceParents(atd, const_cast<Arc*>(a));
             reachable[ah] = true;
             PRINT_DEBUG( "(" << a->getTail() << ", " << a->getHead() << ")" << " is a new tree arc.");
             return true;
         }
         else if (atd->level + 1 == ahd->level && !ahd->isParent(atd)) 
         {
            PRINT_DEBUG( "(" << a->getTail() << ", " << a->getHead() << ")" << " is a new tree arc.");
            ahd->tryAddParent(atd, const_cast<Arc*>(a));
         }
         else
         {
             PRINT_DEBUG( "(" << a->getTail() << ", " << a->getHead() << ")" << " is a non-tree arc.")
         }
         return false; });
            runAlgorithm(bfs, diGraph);
        }

        IF_DEBUG(
            if (!checkTree()) {
                std::cerr.flush();
                dumpTree(std::cerr);
                std::cerr.flush();
            });
        assert(checkTree());
    }

    template <bool reverseArcDirection, unsigned maxNTreeArcs>
    void SimpleESTreeMultipleTreeArcs<reverseArcDirection, maxNTreeArcs>::onVertexRemove(Vertex *v)
    {
        if (!initialized)
        {
            return;
        }

        auto vd = data(v);
        if (vd != nullptr)
        {
            delete vd;
            data.resetToDefault(v);
            reachable.resetToDefault(v);
        }
    }

    template <bool reverseArcDirection, unsigned maxNTreeArcs>
    void SimpleESTreeMultipleTreeArcs<reverseArcDirection, maxNTreeArcs>::onArcRemove(Arc *a)
    {
        if (!initialized)
        {
            return;
        }

        PRINT_DEBUG("An arc is about to be removed: (" << a->getTail() << ", " << a->getHead() << ")");

        if (a->isLoop())
        {
            PRINT_DEBUG("Arc is a loop.")
            return;
        }

        auto head = reverseArcDirection ? a->getTail() : a->getHead();

        if (head == source)
        {
            PRINT_DEBUG("Head is source.")
            return;
        }

        PRINT_DEBUG("Stored data of tail: " << data(a->getTail()));
        PRINT_DEBUG("Stored data of head: " << data(a->getHead()));

        auto hd = data(head);

        if (hd == nullptr)
        {
            if (reverseArcDirection)
            {
                PRINT_DEBUG("Tail of arc is unreachable (and never was). Nothing to do.")
            }
            else
            {
                PRINT_DEBUG("Head of arc is unreachable (and never was). Nothing to do.")
            }
            throw std::logic_error("Should not happen");
        }

        if (!hd->isReachable())
        {
            if (reverseArcDirection)
            {
                PRINT_DEBUG("Tail of arc is already unreachable. Nothing to do.")
            }
            else
            {
                PRINT_DEBUG("Head of arc is already unreachable. Nothing to do.")
            }
#ifdef COLLECT_PR_DATA
            decUnreachableHead++;
#endif
            return;
        }

        auto tail = reverseArcDirection ? a->getHead() : a->getTail();
        auto td = data(tail);
        if (hd->tryRemoveParent(td) && !hd->hasAnyParent())
        {
            restoreTree(hd);
        }
        else
        {
            PRINT_DEBUG("Arc is not a tree arc. Nothing to do.")
#ifdef COLLECT_PR_DATA
            decNonTreeArc++;
#endif
        }

        IF_DEBUG(
            if (!checkTree()) {
                std::cerr.flush();
                dumpTree(std::cerr);
                std::cerr.flush();
            });
        assert(checkTree());
    }

    template <bool reverseArcDirection, unsigned maxNTreeArcs>
    void SimpleESTreeMultipleTreeArcs<reverseArcDirection, maxNTreeArcs>::onSourceSet()
    {
        cleanup(false);
    }

    template <bool reverseArcDirection, unsigned maxNTreeArcs>
    bool SimpleESTreeMultipleTreeArcs<reverseArcDirection, maxNTreeArcs>::query(const Vertex *t)
    {
        PRINT_DEBUG("Querying reachability of " << t);
        if (t == source)
        {
            PRINT_DEBUG("TRUE");
            return true;
        }

        if (!initialized)
        {
            PRINT_DEBUG("Query in uninitialized state.");
            run();
        }
        PRINT_DEBUG((reachable[t] ? "TRUE" : "FALSE"));
        return reachable(t);
    }

    template <bool reverseArcDirection, unsigned maxNTreeArcs>
    std::vector<Arc *> SimpleESTreeMultipleTreeArcs<reverseArcDirection, maxNTreeArcs>::queryPath(const Vertex *t)
    {
        std::vector<Arc *> path;
        if (!query(t) || t == source)
        {
            return path;
        }

        while (t != source)
        {
            auto *a = data(t)->getTreeArc(0);
            std::cout << "t: " << data(t) << std::endl;
            std::cout << "tree arc: " << a << std::endl;
            std::cout << "nParents: " << data(t)->nParents << std::endl;
            path.push_back(a);
            t = reverseArcDirection ? a->getHead() : a->getTail();
        }
        assert(!path.empty());

        if (!reverseArcDirection)
        {
            std::reverse(path.begin(), path.end());
        }

        return path;
    }

    template <bool reverseArcDirection, unsigned maxNTreeArcs>
    void SimpleESTreeMultipleTreeArcs<reverseArcDirection, maxNTreeArcs>::dumpData(std::ostream &os) const
    {
        if (!initialized)
        {
            os << "uninitialized" << std::endl;
        }
        else
        {
            for (auto i = data.cbegin(); i != data.cend(); i++)
            {
                os << (*i) << std::endl;
            }

            os << "Tree in dot format:\ndigraph SESTree {\n";
            for (const auto vd : data)
            {
                const auto [treeArcs, nTreeArcs] = vd->getExistingTreeArcs();
                for (size_t treeArcIndex = 0; treeArcIndex < nTreeArcs; treeArcIndex++)
                {
                    const auto *currentTreeArc = treeArcs[treeArcIndex];
                    os << currentTreeArc->getTail()->getName() << " -> "
                       << currentTreeArc->getHead()->getName() << ";\n";
                }
            }
            os << "}" << std::endl;
        }
    }

    template <bool reverseArcDirection, unsigned maxNTreeArcs>
    void SimpleESTreeMultipleTreeArcs<reverseArcDirection, maxNTreeArcs>::dumpTree(std::ostream &os)
    {
        if (!initialized)
        {
            os << "uninitialized" << std::endl;
        }
        else
        {
            diGraph->mapVertices([&](Vertex *v)
                                 {
                auto vd = data[v];
                const auto [treeArcs, nTreeArcs] = vd->getExistingTreeArcs();

                os << v << ": L " << vd->level << ", A [";
                bool first = true;
                for (size_t treeArcIndex = 0; treeArcIndex < nTreeArcs; treeArcIndex++)
                {
                    const auto* currentTreeArc = treeArcs[treeArcIndex];
                    if (first)
                    {
                        first = false;
                    }
                    else
                    {
                        os << ", ";
                    }
                    os << currentTreeArc;
                }
                os << "], P [";
                first = true;

                const auto& parents = vd->getParents();
                for (size_t parentIndex = 0; parentIndex < nTreeArcs; parentIndex++)
                {
                    const auto* currentParent = parents[parentIndex];
                    if (first)
                    {
                        first = false;
                    }
                    else
                    {
                        os << ", ";
                    }
                    os << currentParent;
                }
                os << "]\n"; });
        }
    }

    template <bool reverseArcDirection, unsigned maxNTreeArcs>
    bool SimpleESTreeMultipleTreeArcs<reverseArcDirection, maxNTreeArcs>::checkTree()
    {
        BreadthFirstSearch<FastPropertyMap, true, reverseArcDirection> bfs;
        bfs.setStartVertex(source);
        bfs.levelAsValues(true);
        FastPropertyMap<DiGraph::size_type> levels(bfs.INF);
        levels.resetAll(diGraph->getSize());
        bfs.useModifiableProperty(&levels);
        runAlgorithm(bfs, diGraph);

        bool ok = true;
        diGraph->mapVertices([&](Vertex *v)
                             {
        auto bfsLevel = levels[v] == bfs.INF ? SESVertexDataMultipleParents<maxNTreeArcs>::UNREACHABLE : levels[v];
        if (data[v]->level != bfsLevel) {
            std::cerr << "Level mismatch for vertex " << data[v]
                         << ": expected level " << bfsLevel << std::endl;
            ok = false;
        } });
        return ok;
    }

    template <bool reverseArcDirection, unsigned maxNTreeArcs>
    void SimpleESTreeMultipleTreeArcs<reverseArcDirection, maxNTreeArcs>::rerun()
    {
#ifdef COLLECT_PR_DATA
        reruns++;
#endif
        diGraph->mapVertices([&](Vertex *v)
                             { data[v]->reset(); });
        initialized = false;
        run();
    }

    template <bool reverseArcDirection, unsigned maxNTreeArcs>
    DiGraph::size_type SimpleESTreeMultipleTreeArcs<reverseArcDirection, maxNTreeArcs>::process(SESVertexDataMultipleParents<maxNTreeArcs> *vd, bool &limitReached)
    {

        if (vd->level == 0UL)
        {
            PRINT_DEBUG("No need to process source vertex " << vd << ".");
            return 0U;
        }

#ifdef COLLECT_PR_DATA
        auto verticesConsidered = 0U;
        auto arcsConsidered = 0U;
#endif

        PRINT_DEBUG("Processing vertex " << vd << ".");
        const Vertex *v = vd->getVertex();

        if (vd->discardInvalidParents())
        {
            PRINT_DEBUG("Node still has valid parents. No further processing required.");
            return 0U;
        }
        else if (!vd->isReachable())
        {
            PRINT_DEBUG("Vertex is already unreachable.");
            return 0U;
        }

        // Note that parents can have different level at this point
        vd->discardHigherLevelParents();

        auto [oldParents, nOldParents] = vd->getExistingParents();

        auto [parents, nParents] = std::tie(oldParents, nOldParents);
        auto oldVLevel = vd->level;
        auto minParentLevel = nParents == 0 ? SESVertexDataMultipleParents<maxNTreeArcs>::UNREACHABLE : parents[0]->level;
        auto treeArcs = vd->getExistingTreeArcs().first;

        PRINT_DEBUG("Min parent level is " << minParentLevel << ".");

        auto findParent = [this, &parents, &nParents, &minParentLevel, &oldVLevel, &treeArcs](Arc *a)
        {
#ifdef COLLECT_PR_DATA
            prArcConsidered();
#endif
            if (a->isLoop())
            {
                PRINT_DEBUG("Loop ignored.");
                return;
            }
            auto pd = data(reverseArcDirection ? a->getHead() : a->getTail());
#ifdef COLLECT_PR_DATA
            prVertexConsidered();
#endif
            auto pLevel = pd->level;
            // TODO: check if parent level is unreachable!
            if (pLevel == minParentLevel && pLevel != SESVertexDataMultipleParents<maxNTreeArcs>::UNREACHABLE)
            {
                if (nParents < maxNTreeArcs)
                {
                    parents[nParents] = pd;
                    treeArcs[nParents] = a;
                    nParents++;
                }
            }
            else if (pLevel < minParentLevel)
            {
                minParentLevel = pLevel;

                std::tie(parents, nParents) = std::make_pair(std::array<SESVertexDataMultipleParents<maxNTreeArcs> *, maxNTreeArcs>{pd}, 1);
                treeArcs = {a};
                PRINT_DEBUG("Update: Min parent level now is " << minParentLevel
                                                               << ", parent " << parent);
                assert(minParentLevel + 1 >= oldVLevel);
            }
        };

        if (reverseArcDirection)
        {
            diGraph->mapOutgoingArcs(v, findParent);
        }
        else
        {
            diGraph->mapIncomingArcs(v, findParent);
        }

        DiGraph::size_type levelDiff = 0U;
        auto n = diGraph->getSize();

        if ((nParents == 0 || minParentLevel >= n - 1) && vd->isReachable())
        {
            vd->setUnreachable();
            reachable.resetToDefault(v);
            levelDiff = n - oldVLevel;
            PRINT_DEBUG("No parent or parent is unreachable. Vertex is unreachable. Level diff "
                        << levelDiff);
        }
        else if (parents != oldParents || oldVLevel <= minParentLevel)
        {
            assert(std::all_of(parents.begin(), parents.begin() + nParents, [](const auto *parent)
                               { return parent->isReachable(); }));
            assert(minParentLevel != SESVertexDataMultipleParents<maxNTreeArcs>::UNREACHABLE);

            vd->reset(parents, treeArcs);

            assert(vd->level == minParentLevel + 1);
            assert(vd->getExistingParents().second == nParents);
            assert(vd->level >= oldVLevel);
            levelDiff = vd->level - oldVLevel;
            PRINT_DEBUG("Parents have changed, new parents are " << parents);
        }

        if (levelDiff > 0U)
        {
            PRINT_DEBUG("Updating children...");
            auto updateChildren = [this, &limitReached](Arc *a)
            {
#ifdef COLLECT_PR_DATA
                prArcConsidered();
#endif
                if (a->isLoop())
                {
                    return;
                }
                Vertex *head = reverseArcDirection ? a->getTail() : a->getHead();
                auto *hd = data(head);
#ifdef COLLECT_PR_DATA
                prVertexConsidered();
#endif
                // TODO: if is lowest level treearc, I think
                if (hd->isTreeArc(a))
                {
                    if (timesInQueue[head] < requeueLimit)
                    {
                        PRINT_DEBUG("    Adding child " << hd << " to queue...");
                        timesInQueue[head]++;
                        if (timesInQueue[head] > maxReQueued)
                        {
                            maxReQueued = timesInQueue[head];
                        }
                        queue.push_back(hd);
                    }
                    else
                    {
                        timesInQueue[head]++;
                        limitReached = true;
                        PRINT_DEBUG("Limit reached for vertex " << head << ".");
                    }
                }
            };
            auto updateUntil = [&limitReached](const Arc *)
            { return limitReached; };
            if (reverseArcDirection)
            {
                diGraph->mapIncomingArcsUntil(vd->vertex, updateChildren, updateUntil);
            }
            else
            {
                diGraph->mapOutgoingArcsUntil(vd->vertex, updateChildren, updateUntil);
            }
        }

#ifdef COLLECT_PR_DATA
        prVerticesConsidered(verticesConsidered);
        prArcsConsidered(arcsConsidered);
#endif

        return levelDiff;
    }

    template <bool reverseArcDirection, unsigned maxNTreeArcs>
    void SimpleESTreeMultipleTreeArcs<reverseArcDirection, maxNTreeArcs>::restoreTree(SESVertexDataMultipleParents<maxNTreeArcs> *rd)
    {
        auto n = diGraph->getSize();
        DiGraph::size_type affectedLimit = maxAffectedRatio < 1.0
                                               ? static_cast<DiGraph::size_type>(floor(maxAffectedRatio * n))
                                               : n;
        queue.set_capacity(affectedLimit);
        timesInQueue.resetAll(n);
        timesInQueue[rd->getVertex()]++;
        queue.clear();
        queue.push_back(rd);
        if (maxReQueued == 0U)
        {
            maxReQueued = 1U;
        }
        PRINT_DEBUG("Initialized queue with " << rd << ".")
        bool limitReached = false;
        auto processed = 0ULL;

        while (!queue.empty())
        {
            IF_DEBUG(printQueue(queue))
            auto vd = queue.front();
            queue.pop_front();
#ifdef COLLECT_PR_DATA
            prVertexConsidered();
            auto levels =
#endif
                process(vd, limitReached);
            processed++;

            if (limitReached || ((processed + queue.size() > affectedLimit) && !queue.empty()))
            {
#ifdef COLLECT_PR_DATA
                if (limitReached)
                {
                    rerunRequeued++;
                }
                if ((processed + queue.size() > affectedLimit) && !queue.empty())
                {
                    rerunNumAffected++;
                }
#endif
                queue.clear();
                rerun();
                break;
#ifdef COLLECT_PR_DATA
            }
            else if (levels > 0U)
            {
                movesDown++;
                levelIncrease += levels;
                if (levels > maxLevelIncrease)
                {
                    maxLevelIncrease = levels;
                }
#endif
            }
        }
#ifdef COLLECT_PR_DATA
        totalAffected += processed;
        if (processed > maxAffected)
        {
            maxAffected = processed;
        }
#endif
    }

    template <bool reverseArcDirection, unsigned maxNTreeArcs>
    void SimpleESTreeMultipleTreeArcs<reverseArcDirection, maxNTreeArcs>::cleanup(bool freeSpace)
    {
        if (initialized)
        {
            for (auto i = data.cbegin(); i != data.cend(); i++)
            {
                if ((*i))
                {
                    delete (*i);
                }
            }
        }

        queue.clear();

        if (freeSpace || !diGraph)
        {
            data.resetAll(0);
            reachable.resetAll(0);
            timesInQueue.resetAll(0);
            queue.set_capacity(0);
        }
        else
        {
            data.resetAll(diGraph->getSize());
            reachable.resetAll(diGraph->getSize());
            timesInQueue.resetAll(diGraph->getSize());
        }

        initialized = false;
    }

    template class SimpleESTreeMultipleTreeArcs<false, 2>;
    template class SimpleESTreeMultipleTreeArcs<true, 2>;
    template class SimpleESTreeMultipleTreeArcs<false, 3>;
    template class SimpleESTreeMultipleTreeArcs<true, 3>;
}
