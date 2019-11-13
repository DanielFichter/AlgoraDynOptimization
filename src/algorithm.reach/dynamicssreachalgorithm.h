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

#ifndef DYNAMICSSREACHALGORITHM_H
#define DYNAMICSSREACHALGORITHM_H

#include "algorithm/dynamicdigraphalgorithm.h"
#include "graph/digraph.h"
#include <ostream>
#include <sstream>

namespace Algora {

class DynamicSSReachAlgorithm : public DynamicDiGraphAlgorithm
{
public:
    explicit DynamicSSReachAlgorithm() : DynamicDiGraphAlgorithm(), source(nullptr) { }
    virtual ~DynamicSSReachAlgorithm() override { }

    void setSource(Vertex *s) { source = s; onSourceSet(); }
    Vertex *getSource() const { return source; }

    virtual bool query(const Vertex *t) = 0;
    virtual std::vector<Arc*> queryPath(const Vertex *) { return std::vector<Arc*>(); }

    // DiGraphAlgorithm interface
public:
    virtual bool prepare() override { return source != nullptr && DynamicDiGraphAlgorithm::prepare() && diGraph->containsVertex(source); }
    virtual std::string getProfilingInfo() const override {
        std::stringstream ss;
        for (const auto &[key, value] : getProfile()) {
            ss << key << ": " << value << '\n';
        }
        return ss.str();
    }

protected:
    virtual void onSourceSet() { }
    virtual void onDiGraphSet() override { DynamicDiGraphAlgorithm::onDiGraphSet(); resetProfileData(); }

    Vertex *source;
};

}

#endif // DYNAMICSSREACHALGORITHM_H
