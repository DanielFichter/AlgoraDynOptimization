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
            for (const auto *parent : vd->parents)
            {
                if (parent)
                {
                    os << "{" << parent->vertex << ", tree arcs: [";
                    for (const auto* treeArc: parent->treeArcs)
                    {
                        os << treeArc << " ";
                    }
                    os << "], level: " << parent->level << "}";
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
                ;
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
        Arc * getTreeArc(size_t index) const { return treeArcs[index]; }
        std::vector<Arc*> getExistingTreeArcs() const
        {
            std::vector<Arc*> existingTreeArcs(nParents);
            std::copy(treeArcs.begin(), treeArcs.begin() + nParents, existingTreeArcs.begin());
            return existingTreeArcs;
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
            std::array<SESVertexDataMultipleParents<maxNParents>*, maxNParents> validParents;
            unsigned nValidParents = 0;
            for (unsigned parentIndex = 0; parentIndex < nParents; parentIndex++)
            {
                auto *currentParent = parents[parentIndex];
                if (isValidParent(currentParent))
                {
                    validParents[nValidParents++] = currentParent;
                }
            }

            parents = validParents;
            nParents = nValidParents;

            return nParents;
        }

        bool isOnlyTreeArc(Arc* arc)
        {
            return nParents == 1 && treeArcs[0] == arc;
        }

        /* bool hasValidParent() const
        {
            assert(parent[0] != nullptr || treeArc[0] == nullptr);
            return parent != nullptr && parent->level + 1 == level;
        } */

        std::array<Vertex *, maxNParents> getParents() const
        {
            std::array<Vertex *, maxNParents> parentVertices;
            std::transform(parents.begin(), parents.end(), parentVertices.begin(), [] (const auto* parentData) {
                return parentData ? parentData->vertex : nullptr;
            });
            return parentVertices;
        }

    //private:
        bool isValidParent(SESVertexDataMultipleParents<maxNParents> *p) const
        {
            return p != nullptr && p->level + 1 == level;
        }

        bool isTreeArc(const Arc *a)
        {
            for (unsigned treeArcIndex = 0; treeArcIndex < nParents; treeArcIndex++)
            {
                if (treeArcs[treeArcIndex] == a)
                {
                    return true;
                }
            }
            return false;
        }

        Vertex *vertex;
        unsigned nParents;
        std::array<SESVertexDataMultipleParents<maxNParents>*, maxNParents> parents;
        std::array<Arc *, maxNParents> treeArcs;
        level_type level;
    };

    /* struct SES_Priority
    {
        template<unsigned maxNParents = 2>
        typename SESVertexDataMultipleParents<maxNParents>::level_type operator()(const SESVertexDataMultipleParents<maxNParents> *vd)
        {
            return vd->getLevel();
        }
    }; */

}

#endif // SESVERTEXDATA_H
