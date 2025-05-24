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

#ifndef SESVERTEXDATAMULTIPLEPARENTS_H
#define SESVERTEXDATAMULTIPLEPARENTS_H

#include <climits>
#include <iostream>
#include <cassert>
#include <limits>
#include <array>
#include <algorithm>
#include "graph/digraph.h"

namespace Algora
{
    class Vertex;
    class Arc;

    template <unsigned maxNParents = 2>
    class SESVertexDataMultipleParents
    {
        friend std::ostream &operator<<(std::ostream &os, const SESVertexDataMultipleParents<maxNParents> *vd)
        {
            if (vd == nullptr)
            {
                os << " null ";
                return os;
            }

            os << vd->vertex << ": ";
            // os << "parent: [" << vd->parent << "] ; level: " << vd->level;
            os << "parents: [";
            for (size_t parentIndex = 0; parentIndex < vd->nParents; parentIndex++)
            {
                const auto* currentParent = vd->parents[parentIndex];
                if (currentParent)
                {
                    os << "{" << currentParent->vertex << ", tree arcs: [";
                    const auto& parentNTreeArcs = currentParent->nParents;
                    for (size_t treeArcIndex = 0; treeArcIndex < parentNTreeArcs; treeArcIndex++)
                    {
                        const auto& currentTreeArc = currentParent->treeArcs[treeArcIndex];
                        os << currentTreeArc << " ";
                    }
                    os << "], level: " << currentParent->level << "}";
                }
                else
                {
                    os << "null";
                }
            }
            os << "] ; level: " << vd->level;

            return os;
        }

    public:
        typedef DiGraph::size_type level_type;
        static constexpr level_type UNREACHABLE = std::numeric_limits<level_type>::max();

        SESVertexDataMultipleParents(Vertex *v, const std::array<SESVertexDataMultipleParents<maxNParents> *, maxNParents> &p = {}, const std::array<Arc *, maxNParents> &a = {},
                                     level_type l = UNREACHABLE)
            : vertex(v), parents(p), treeArcs(a), level(l)
        {

            if (parents[0])
            {
                level = p[0]->level + 1;
            }
            nParents = calculateNParents();
        }

        unsigned calculateNParents() const
        {
            unsigned parentIndex = 0;
            for (; parentIndex < maxNParents && parents[parentIndex]; parentIndex++)
            {
                assert(treeArcs[parentIndex]);
            }

            return parentIndex;
        }

        void reset(const std::array<SESVertexDataMultipleParents<maxNParents> *, maxNParents> &p = {}, const std::array<Arc *, maxNParents> &a = {}, level_type l = UNREACHABLE)
        {
            parents = p;
            level = l;
            treeArcs = a;
            if (p[0] != nullptr)
            {
                level = p[0]->level + 1;
            }
            nParents = calculateNParents();
        }

        level_type getLevel() const
        {
            return level;
        }

        Vertex *getVertex() const { return vertex; }
        const std::array<SESVertexDataMultipleParents<maxNParents> *, maxNParents> &getParentsData() const { return parents; }
        Arc *getTreeArc(size_t index) const { return treeArcs[index]; }

        std::pair<std::array<Arc *, maxNParents>, size_t> getExistingTreeArcs() const
        {
            return {treeArcs, nParents};
        }

        std::pair<std::array<SESVertexDataMultipleParents<maxNParents> *, maxNParents>, size_t> getExistingParents() const
        {
            return {parents, nParents};
        }

        void discardHigherLevelParents()
        {
            if (nParents == 0)
            {
                return;
            }

            const auto lowestLevel = getLowestLevel();

            std::array<SESVertexDataMultipleParents<maxNParents> *, maxNParents> lowestLevelParents{};
            std::array<Arc *, maxNParents> lowestLevelArcs{};
            unsigned lowestLevelParentIndex = 0;
            for (unsigned parentIndex = 0; parentIndex < nParents; parentIndex++)
            {
                auto *currentParent = parents[parentIndex];
                if (currentParent->level == lowestLevel)
                {
                    lowestLevelParents[lowestLevelParentIndex] = currentParent;
                    lowestLevelArcs[lowestLevelParentIndex] = treeArcs[parentIndex];
                    lowestLevelParentIndex++;
                }
            }

            parents = lowestLevelParents;
            treeArcs = lowestLevelArcs;
            nParents = lowestLevelParentIndex;
            assert(nParents <= maxNParents);
        }

        void setUnreachable()
        {
            parents = {};
            treeArcs = {};
            level = UNREACHABLE;
        }

        // add parent with same level as existing parents
        void tryAddParent(SESVertexDataMultipleParents<maxNParents> *pd, Arc *a)
        {
            if (nParents < maxNParents)
            {
                parents[nParents] = pd;
                treeArcs[nParents] = a;
                nParents++;
            }
        }

        // replace existing parents with new parent with lower level
        void replaceParents(SESVertexDataMultipleParents<maxNParents> *pd, Arc *a)
        {
            for (size_t parentIndex = 1; parentIndex < nParents; parentIndex++)
            {
                parents[parentIndex] = nullptr;
            }
            nParents = 1;
            parents[0] = pd;
            treeArcs[0] = a;
            level = pd->getLevel() + 1U;
        }

        bool isReachable() const
        {
            return level != UNREACHABLE;
        }

        bool isParent(const SESVertexDataMultipleParents<maxNParents> *p) const
        {
            for (unsigned parentIndex = 0; parentIndex < nParents; parentIndex++)
            {
                if (parents[parentIndex] == p)
                {
                    return true;
                }
            }
            return false;
        }
        
        // returns wether there are valid parents left
        bool discardInvalidParents()
        {
            std::array<SESVertexDataMultipleParents<maxNParents> *, maxNParents> validParents{};
            std::array<Arc *, maxNParents> validArcs{};
            unsigned validParentIndex = 0;
            for (unsigned parentIndex = 0; parentIndex < nParents; parentIndex++)
            {
                auto *currentParent = parents[parentIndex];
                if (isValidParent(currentParent))
                {
                    validParents[validParentIndex] = currentParent;
                    validArcs[validParentIndex] = treeArcs[parentIndex];
                    validParentIndex++;
                }
            }

            parents = validParents;
            treeArcs = validArcs;
            nParents = validParentIndex;

            assert(nParents <= maxNParents);

            return nParents;
        }

        /* bool hasValidParent() const
        {
            assert(parent[0] != nullptr || treeArc[0] == nullptr);
            return parent != nullptr && parent->level + 1 == level;
        } */

        std::array<Vertex *, maxNParents> getParents() const
        {
            std::array<Vertex *, maxNParents> parentVertices;
            std::transform(parents.begin(), parents.end(), parentVertices.begin(), [](const auto *parentData)
                           { return parentData ? parentData->vertex : nullptr; });
            return parentVertices;
        }

        SESVertexDataMultipleParents<maxNParents> *getParentData(size_t index)
        {
            assert(parents[index]);
            return parents[index];
        }

        bool hasAnyParent() const
        {
            return nParents;
        }

        bool tryRemoveParent(SESVertexDataMultipleParents<maxNParents> *p)
        {
            bool deleted = false;
            for (unsigned parentIndex = 0; parentIndex < nParents; parentIndex++)
            {
                const auto *currentParent = parents[parentIndex];
                if (currentParent == p)
                {
                    deleted = true;
                }
                if (deleted)
                {
                    if (parentIndex < nParents - 1)
                    {
                        parents[parentIndex] = parents[parentIndex + 1];
                        treeArcs[parentIndex] = treeArcs[parentIndex + 1];
                    }
                    else
                    {
                        parents[parentIndex] = nullptr;
                        treeArcs[parentIndex] = nullptr;
                    }
                }
            }

            if (deleted)
            {
                nParents--;
            }

            assert(nParents <= maxNParents);

            return deleted;
        }

        // private:
        bool isValidParent(SESVertexDataMultipleParents<maxNParents> *p) const
        {
            return p != nullptr && p->level + 1 == level;
        }

        bool isTreeArc(const Arc *a)
        {
            for (size_t treeArcIndex = 0; treeArcIndex < nParents; treeArcIndex++)
            {
                if (treeArcs[treeArcIndex] == a)
                {
                    return true;
                }
            }
            return false;
        }

        level_type getLowestLevel() const
        {
            assert(nParents);
            level_type lowestLevel;
            for (size_t parentIndex = 0; parentIndex < nParents; parentIndex++)
            {
                const auto *currentParent = parents[parentIndex];
                if (currentParent->level < lowestLevel)
                {
                    lowestLevel = currentParent->level;
                }
            }

            return lowestLevel;
        }

        Vertex *vertex;
        size_t nParents;
        std::array<SESVertexDataMultipleParents<maxNParents> *, maxNParents> parents;
        std::array<Arc *, maxNParents> treeArcs;
        level_type level;
    };
}

#endif // SESVERTEXDATA_H
