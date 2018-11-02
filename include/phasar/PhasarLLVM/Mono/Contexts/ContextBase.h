/******************************************************************************
 * Copyright (c) 2017 Philipp Schubert.
 * All rights reserved. This program and the accompanying materials are made
 * available under the terms of LICENSE.txt.
 *
 * Contributors:
 *     Nicolas Bellec and others
 *****************************************************************************/

/*
 * ContextBase.h
 *
 *  Created on: 18.06.2018
 *      Author: nicolas
 */

#ifndef PHASAR_PHASARLLVM_MONO_CONTEXTS_CONTEXTBASE_H_
#define PHASAR_PHASARLLVM_MONO_CONTEXTS_CONTEXTBASE_H_

#include <ostream>
#include <phasar/PhasarLLVM/Utils/Printer.h>

namespace psr {

/**
 * Base class for function contexts used in the monotone framework. A function
 * context describes the state of the analyzed function.
 * @tparam N node in the ICFG
 * @tparam D domain of the analysis
 */
template <typename N, typename D> class ContextBase {
public:
  using Node_t = N;
  using Domain_t = D;
  // const NodePrinter<N> *NP;
  // const DataFlowFactPrinter<D> *DP;

  /**
   * @brief Update the context when exiting a function
   */
  virtual void exitFunction(const Node_t src, const Node_t dest,
                            const Domain_t &In) = 0;

  /**
   * @brief Update the context when entering a function
   */
  virtual void enterFunction(const Node_t src, const Node_t dest,
                             const Domain_t &In) = 0;

  virtual bool isUnsure() = 0;
  virtual bool isEqual(const ContextBase<N, D> &rhs) const = 0;
  virtual bool isDifferent(const ContextBase<N, D> &rhs) const = 0;
  virtual bool isLessThan(const ContextBase<N, D> &rhs) const = 0;
  virtual void print(std::ostream &os) const = 0;

  friend bool operator==(const ContextBase<N, D> &lhs,
                         const ContextBase<N, D> &rhs) {
    return lhs.isEqual(rhs);
  }
  friend bool operator!=(const ContextBase<N, D> &lhs,
                         const ContextBase<N, D> &rhs) {
    return lhs.isDifferent(rhs);
  }
  friend bool operator<(const ContextBase<N, D> &lhs,
                        const ContextBase<N, D> &rhs) {
    return lhs.isLessThan(rhs);
  }
  friend std::ostream &operator<<(std::ostream &os,
                                  const ContextBase<N, D> &c) {
    c.print(os);
    return os;
  }
};

} // namespace psr

#endif
