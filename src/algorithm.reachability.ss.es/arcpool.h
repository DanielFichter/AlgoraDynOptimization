#ifndef ARCPOOL_H
#define ARCPOOL_H

#include "sesvertexdata.h"

namespace Algora {
class ArcPool {
public:
  void insert(std::pair<Arc *, SESVertexData *> arc) {
    if (arraySize == nArcs) {
      arraySize *= 2;
      std::pair<Arc *, SESVertexData *> *newData =
          new std::pair<Arc *, SESVertexData *>[arraySize];
      for (size_t arcIndex = 0; arcIndex < nArcs; arcIndex++) {
        newData[arcIndex] = data[arcIndex];
      }
      delete[] data;
      data = newData;
    }
    data[nArcs] = arc;
    nArcs++;
  }

  void clear() { nArcs = 0; }

  ~ArcPool() { delete[] data; }

  bool empty() const { return nArcs == 0; }

  size_t size() const { return nArcs; }

  const std::pair<Arc *, SESVertexData *> &at(size_t index) const {
    return data[index];
  }

private:
  std::pair<Arc *, SESVertexData *> *data =
      new std::pair<Arc *, SESVertexData *>[5]{};
  size_t nArcs = 0;
  size_t arraySize = 5;
};
} // namespace Algora

#endif