/******************************************************************************
 * Copyright (c) 2018 Philipp Schubert.
 * All rights reserved. This program and the accompanying materials are made
 * available under the terms of LICENSE.txt.
 *
 * Contributors:
 *     Philipp Schubert and others
 *****************************************************************************/

#include <iostream>

#include <llvm/IR/Function.h>
#include <llvm/IR/Instruction.h>
#include <llvm/IR/Value.h>

#include <phasar/PhasarLLVM/ControlFlow/LLVMBasedICFG.h>
#include <phasar/PhasarLLVM/IfdsIde/FlowFunctions/Gen.h>
#include <phasar/PhasarLLVM/IfdsIde/FlowFunctions/Identity.h>
#include <phasar/PhasarLLVM/IfdsIde/LLVMZeroValue.h>
#include <phasar/PhasarLLVM/IfdsIde/Problems/IFDSLiveVariableAnalysis.h>
#include <phasar/Utils/LLVMShorthands.h>

using namespace psr;
using namespace std;

namespace psr {

IFDSLiveVariableAnalysis::IFDSLiveVariableAnalysis(
    IFDSLiveVariableAnalysis::i_t icfg, vector<string> EntryPoints)
    : DefaultIFDSTabulationProblem(icfg), EntryPoints(EntryPoints) {
  IFDSLiveVariableAnalysis::zerovalue = createZeroValue();
}

shared_ptr<FlowFunction<IFDSLiveVariableAnalysis::d_t>>
IFDSLiveVariableAnalysis::getNormalFlowFunction(
    IFDSLiveVariableAnalysis::n_t curr, IFDSLiveVariableAnalysis::n_t succ) {
  cout << "IFDSLiveVariableAnalysis::getNormalFlowFunction()\n";
  return Identity<IFDSLiveVariableAnalysis::d_t>::getInstance();
}

shared_ptr<FlowFunction<IFDSLiveVariableAnalysis::d_t>>
IFDSLiveVariableAnalysis::getCallFlowFunction(
    IFDSLiveVariableAnalysis::n_t callStmt,
    IFDSLiveVariableAnalysis::m_t destMthd) {
  cout << "IFDSLiveVariableAnalysis::getCallFlowFunction()\n";
  return Identity<IFDSLiveVariableAnalysis::d_t>::getInstance();
}

shared_ptr<FlowFunction<IFDSLiveVariableAnalysis::d_t>>
IFDSLiveVariableAnalysis::getRetFlowFunction(
    IFDSLiveVariableAnalysis::n_t callSite,
    IFDSLiveVariableAnalysis::m_t calleeMthd,
    IFDSLiveVariableAnalysis::n_t exitStmt,
    IFDSLiveVariableAnalysis::n_t retSite) {
  cout << "IFDSLiveVariableAnalysis::getRetFlowFunction()\n";
  return Identity<IFDSLiveVariableAnalysis::d_t>::getInstance();
}

shared_ptr<FlowFunction<IFDSLiveVariableAnalysis::d_t>>
IFDSLiveVariableAnalysis::getCallToRetFlowFunction(
    IFDSLiveVariableAnalysis::n_t callSite,
    IFDSLiveVariableAnalysis::n_t retSite,
    set<IFDSLiveVariableAnalysis::m_t> callees) {
  cout << "IFDSLiveVariableAnalysis::getCallToRetFlowFunction()\n";
  return Identity<IFDSLiveVariableAnalysis::d_t>::getInstance();
}

shared_ptr<FlowFunction<IFDSLiveVariableAnalysis::d_t>>
IFDSLiveVariableAnalysis::getSummaryFlowFunction(
    IFDSLiveVariableAnalysis::n_t callStmt,
    IFDSLiveVariableAnalysis::m_t destMthd) {
  cout << "IFDSLiveVariableAnalysis::getSummaryFlowFunction()\n";
  return nullptr;
}

map<IFDSLiveVariableAnalysis::n_t, set<IFDSLiveVariableAnalysis::d_t>>
IFDSLiveVariableAnalysis::initialSeeds() {
  cout << "IFDSLiveVariableAnalysis::initialSeeds()\n";
  map<IFDSLiveVariableAnalysis::n_t, set<IFDSLiveVariableAnalysis::d_t>>
      SeedMap;
  for (auto &EntryPoint : EntryPoints) {
    SeedMap.insert(
        make_pair(&icfg.getMethod(EntryPoint)->front().front(),
                  set<IFDSLiveVariableAnalysis::d_t>({zeroValue()})));
  }
  return SeedMap;
}

IFDSLiveVariableAnalysis::d_t IFDSLiveVariableAnalysis::createZeroValue() {
  // create a special value to represent the zero value!
  return LLVMZeroValue::getInstance();
}

bool IFDSLiveVariableAnalysis::isZeroValue(
    IFDSLiveVariableAnalysis::d_t d) const {
  return isLLVMZeroValue(d);
}

void IFDSLiveVariableAnalysis::printNode(
    ostream &os, IFDSLiveVariableAnalysis::n_t n) const {
  os << llvmIRToString(n);
}

void IFDSLiveVariableAnalysis::printDataFlowFact(
    ostream &os, IFDSLiveVariableAnalysis::d_t d) const {
  os << llvmIRToString(d);
}

void IFDSLiveVariableAnalysis::printMethod(
    ostream &os, IFDSLiveVariableAnalysis::m_t m) const {
  os << m->getName().str();
}

} // namespace psr
