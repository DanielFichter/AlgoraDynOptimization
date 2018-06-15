#ifndef SIMPLEINCSSREACHALGORITHM_H
#define SIMPLEINCSSREACHALGORITHM_H

#include "dynamicssreachalgorithm.h"

#include "property/propertymap.h"

namespace Algora {

class SimpleIncSSReachAlgorithm : public DynamicSSReachAlgorithm
{
public:
    explicit SimpleIncSSReachAlgorithm(bool reverse = false, bool searchForward = false);
    virtual ~SimpleIncSSReachAlgorithm();

    // DiGraphAlgorithm interface
public:
    virtual void run() override;
    virtual std::string getName() const noexcept override {
        return reverse ? ( searchForward ? "Simple Incremental Single-Source Reachability Algorithm (reverse, search forward)"
                                            : "Simple Incremental Single-Source Reachability Algorithm (reverse)")
                        :  ( searchForward ? "Simple Incremental Single-Source Reachability Algorithm (search forward)"
                                            : "Simple Incremental Single-Source Reachability Algorithm");
    }
    virtual std::string getShortName() const noexcept override {
       return  reverse ? ( searchForward ? "Simple-ISSReach-R-SF"
                                            : "Simple-ISSReach-R")
                        :  ( searchForward ? "Simple-ISSReach-SF"
                                            : "Simple-ISSReach");
    }

protected:
    virtual void onDiGraphUnset() override;

    // DynamicDiGraphAlgorithm interface
protected:
    virtual void onVertexAdd(Vertex *) override;
    virtual void onVertexRemove(Vertex *v) override;
    virtual void onArcAdd(Arc *a) override;
    virtual void onArcRemove(Arc *a) override;

    // DynamicSSReachAlgorithm interface
public:
    virtual bool query(const Vertex *t) override;
    virtual void dumpData(std::ostream &os) override;

    // DynamicSSReachAlgorithm interface
protected:
    virtual void onSourceSet() override;

private:
    struct Reachability;
    Reachability *data;
    bool initialized;

    bool reverse;
    bool searchForward;
};

}

#endif // SIMPLEINCSSREACHALGORITHM_H
