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

#ifndef SESVERTEXDATATIMESTAMPS_H
#define SESVERTEXDATATIMESTAMPS_H

#include <vector>
#include <climits>
#include <iostream>
#include <cassert>
#include <limits>
#include "graph/digraph.h"

namespace Algora {

class Vertex;
class Arc;

class SESVertexDataTimestamps
{
    friend std::ostream& operator<<(std::ostream &os, const SESVertexDataTimestamps *vd);
    

public:
    typedef DiGraph::size_type level_type;
    typedef unsigned long long DynamicTime;
    static constexpr level_type UNREACHABLE = std::numeric_limits<level_type>::max();

    SESVertexDataTimestamps(Vertex *v, SESVertexDataTimestamps *p = nullptr, Arc *a = nullptr,
                  level_type l = UNREACHABLE)
        : vertex(v), parent(p), treeArc(a), level(l) {
        if (p != nullptr) {
            level = p->level + 1;
        }
    }

    void reset(SESVertexDataTimestamps *p = nullptr, Arc *a = nullptr, level_type l = UNREACHABLE) {
        parent = p;
        level = l;
        treeArc = a;
        if (p != nullptr) {
            level = p->level + 1;
        }
    }

    level_type getLevel() const {
        return level;
    }

    Vertex *getVertex() const { return vertex; }
    SESVertexDataTimestamps *getParentData() const { return parent; }
    Arc *getTreeArc() const { return treeArc; }

    void setUnreachable() {
        parent = nullptr;
        treeArc = nullptr;
        level = UNREACHABLE;
    }

    void setParent(SESVertexDataTimestamps *pd, Arc *a) {
        parent = pd;
        treeArc = a;
        level = pd->getLevel() + 1U;
    }

    bool isReachable() const {
        return level != UNREACHABLE;
    }

    bool isTreeArc(const Arc *a) {
        return a == treeArc;
    }

    bool isParent(const SESVertexDataTimestamps *p) {
        return p == parent;
    }

    bool hasValidParent() const {
        assert(parent != nullptr || treeArc == nullptr);
        return parent != nullptr && parent->level + 1 == level;
    }

    Vertex *getParent() const {
        if (parent == nullptr) {
            return nullptr;
        }
        return parent->vertex;
    }

    void addArc(Arc* pArc, DynamicTime birth)
    {
        // TODO: consider multiarcs
        inArcInfos.push_back({pArc, birth});
    }

//private:

    struct ArcInfo
    {
        Arc* pArc;
        DynamicTime birth;
    };

    std::vector<ArcInfo> inArcInfos;

    Vertex *vertex;
    SESVertexDataTimestamps *parent;
    Arc *treeArc;
    level_type level;
};

std::ostream& operator<<(std::ostream& os, const SESVertexDataTimestamps *vd);
}

#endif // SESVERTEXDATATIMESTAMPS_H
