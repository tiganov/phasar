/******************************************************************************
 * Copyright (c) 2017 Philipp Schubert.
 * All rights reserved. This program and the accompanying materials are made
 * available under the terms of LICENSE.txt.
 *
 * Contributors:
 *     Philipp Schubert and others
 *****************************************************************************/

/*
 * InterMonoTaintAnalysis.h
 *
 *  Created on: 22.08.2018
 *      Author: richard leer
 */

#ifndef PHASAR_PHASARLLVM_MONO_PROBLEMS_INTERMONOTAINTANALYSIS_H_
#define PHASAR_PHASARLLVM_MONO_PROBLEMS_INTERMONOTAINTANALYSIS_H_

#include <iosfwd>
#include <string>
#include <vector>

#include <phasar/PhasarLLVM/Mono/InterMonoProblem.h>
#include <phasar/PhasarLLVM/Utils/TaintSensitiveFunctions.h>

namespace llvm {
class Instruction;
class Value;
class Function;
} // namespace llvm

namespace psr {

class LLVMBasedICFG;

/**
 * This analysis tracks data-flows through a program. Data flows from
 * dedicated source functions, which generate tainted values, into
 * dedicated sink functions. A leak is reported once a tainted value
 * reached a sink function.
 *
 * @see TaintSensitiveFunctions on how to specify your own
 * taint-sensitive source and sink functions.
 */
class InterMonoTaintAnalysis
    : public InterMonoProblem<const llvm::Instruction *,
                              MonoSet<const llvm::Value *>,
                              const llvm::Function *, LLVMBasedICFG &> {
public:
  using Node_t = const llvm::Instruction *;
  using DomainElement_t = const llvm::Value *;
  using Domain_t = MonoSet<DomainElement_t>;
  using Method_t = const llvm::Function *;
  using ICFG_t = LLVMBasedICFG &;

  /// Holds all leaks found during the analysis
  std::map<Node_t, std::set<DomainElement_t>> Leaks;

protected:
  TaintSensitiveFunctions SourceSinkFunctions;
  std::vector<std::string> EntryPoints;

public:
  InterMonoTaintAnalysis(ICFG_t &Icfg, TaintSensitiveFunctions TSF,
                         std::vector<std::string> EntryPoints = {"main"});

  virtual ~InterMonoTaintAnalysis() = default;

  Domain_t join(const Domain_t &Lhs, const Domain_t &Rhs) override;

  bool sqSubSetEqual(const Domain_t &Lhs, const Domain_t &Rhs) override;

  Domain_t normalFlow(Node_t Stmt, const Domain_t &In) override;

  Domain_t callFlow(Node_t CallSite, Method_t Callee,
                    const Domain_t &In) override;

  Domain_t returnFlow(Node_t CallSite, Method_t Callee, Node_t RetSite,
                      const Domain_t &In) override;

  Domain_t callToRetFlow(Node_t CallSite, Node_t RetSite,
                         const Domain_t &In) override;

  MonoMap<Node_t, Domain_t> initialSeeds() override;

  std::string DtoString(const Domain_t d) override;

  std::string MtoString(Method_t m) override;

  std::string NtoString(Node_t n) override;

  bool recompute(Method_t Callee) override;

  void printLeaks() const;
};

} // namespace psr

#endif
