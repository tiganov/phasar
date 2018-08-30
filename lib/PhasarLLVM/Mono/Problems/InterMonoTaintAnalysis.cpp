/******************************************************************************
 * Copyright (c) 2017 Philipp Schubert.
 * All rights reserved. This program and the accompanying materials are made
 * available under the terms of LICENSE.txt.
 *
 * Contributors:
 *     Philipp Schubert and others
 *****************************************************************************/

#include <ostream>

#include <llvm/IR/CallSite.h>
#include <llvm/IR/Instruction.h>
#include <llvm/IR/Value.h>

#include <phasar/PhasarLLVM/ControlFlow/LLVMBasedICFG.h>
#include <phasar/PhasarLLVM/Mono/Problems/InterMonoTaintAnalysis.h>
#include <phasar/Utils/LLVMShorthands.h>
#include <phasar/Utils/Logger.h>
#include <phasar/Utils/Macros.h>

using namespace std;
using namespace psr;

namespace psr {

using Node_t = InterMonoTaintAnalysis::Node_t;
using DomainElement_t = InterMonoTaintAnalysis::DomainElement_t;
using Domain_t = InterMonoTaintAnalysis::Domain_t;
using Method_t = InterMonoTaintAnalysis::Method_t;
using ICFG_t = InterMonoTaintAnalysis::ICFG_t;

InterMonoTaintAnalysis::InterMonoTaintAnalysis(ICFG_t &Icfg,
                                               TaintSensitiveFunctions TSF,
                                               vector<string> EntryPoints)
    : InterMonoProblem<Node_t, Domain_t, Method_t, ICFG_t>(Icfg),
      SourceSinkFunctions(TSF), EntryPoints(EntryPoints) {}

Domain_t InterMonoTaintAnalysis::join(const Domain_t &Lhs,
                                      const Domain_t &Rhs) {
  auto &lg = lg::get();
  LOG_IF_ENABLE(BOOST_LOG_SEV(lg, DEBUG) << "InterMonoTaintAnalysis::join()");
  Domain_t Result;
  set_union(Lhs.begin(), Lhs.end(), Rhs.begin(), Rhs.end(),
            inserter(Result, Result.begin()));
  LOG_IF_ENABLE(BOOST_LOG_SEV(lg, DEBUG) << "Result size: " << Result.size());
  return Result;
}

bool InterMonoTaintAnalysis::sqSubSetEqual(const Domain_t &Lhs,
                                           const Domain_t &Rhs) {
  auto &lg = lg::get();
  LOG_IF_ENABLE(BOOST_LOG_SEV(lg, DEBUG)
                << "InterMonoTaintAnalysis::sqSubSetEqual()");
  return includes(Rhs.begin(), Rhs.end(), Lhs.begin(), Lhs.end());
}

Domain_t InterMonoTaintAnalysis::normalFlow(const Node_t Stmt,
                                            const Domain_t &In) {
  auto &lg = lg::get();
  LOG_IF_ENABLE(BOOST_LOG_SEV(lg, DEBUG)
                << "InterMonoTaintAnalysis::normalFlow()");
  LOG_IF_ENABLE(BOOST_LOG_SEV(lg, DEBUG) << "Node: " << NtoString(Stmt));
  LOG_IF_ENABLE(BOOST_LOG_SEV(lg, DEBUG) << "IN Set:\n" << DtoString(In));
  // Check Store Instruction
  if (auto Store = llvm::dyn_cast<llvm::StoreInst>(Stmt)) {
    Domain_t Result(In.begin(), In.end());
    DomainElement_t PointerOp = Store->getPointerOperand();
    DomainElement_t ValueOp = Store->getValueOperand();
    // If a tainted value is stored, the store location must be tainted too
    if (In.count(ValueOp)) {
      Result.insert(PointerOp);
    }
    // If a not tainted value is stored, the store location is no longer tainted
    else if (!In.count(ValueOp) && In.count(PointerOp)) {
      Result.erase(PointerOp);
    }
    LOG_IF_ENABLE(BOOST_LOG_SEV(lg, DEBUG) << "OUT Set:\n"
                                           << DtoString(Result));
    return Result;
  }

  // Check Load Instruction
  if (auto Load = llvm::dyn_cast<llvm::LoadInst>(Stmt)) {
    Domain_t Result(In.begin(), In.end());
    // If a tainted value is loaded, the loaded value is of course tainted
    if (In.count(Load->getPointerOperand())) {
      Result.insert(Load);
    }
    LOG_IF_ENABLE(BOOST_LOG_SEV(lg, DEBUG) << "OUT Set:\n"
                                           << DtoString(Result));
    return Result;
  }

  // Check GetElementPtr Instruction
  if (auto GEP = llvm::dyn_cast<llvm::GetElementPtrInst>(Stmt)) {
    Domain_t Result(In.begin(), In.end());
    // Check if an address is computed from a tainted base pointer of an
    // aggregated object
    if (In.count(GEP->getPointerOperand())) {
      Result.insert(GEP);
    }
    LOG_IF_ENABLE(BOOST_LOG_SEV(lg, DEBUG) << "OUT Set:\n"
                                           << DtoString(Result));
    return Result;
  }

  // Otherwise we leave everything as it is
  return In;
}

Domain_t InterMonoTaintAnalysis::callFlow(const Node_t CallSite,
                                          const Method_t Callee,
                                          const Domain_t &In) {
  auto &lg = lg::get();
  LOG_IF_ENABLE(BOOST_LOG_SEV(lg, DEBUG)
                << "InterMonoTaintAnalysis::callFlow()");
  LOG_IF_ENABLE(BOOST_LOG_SEV(lg, DEBUG)
                << "Call-site : " << NtoString(CallSite));
  LOG_IF_ENABLE(BOOST_LOG_SEV(lg, DEBUG)
                << "Callee    : " << MtoString(Callee));
  LOG_IF_ENABLE(BOOST_LOG_SEV(lg, DEBUG) << "IN Set:\n" << DtoString(In));
  // Check if a source or sink function is called
  if (SourceSinkFunctions.isSource(Callee->getName().str()) ||
      (SourceSinkFunctions.isSink(Callee->getName().str()))) {
    // We then can kill all data-flow facts not following the called function.
    // The respective taints or leaks are then generated in the corresponding
    // call to return flow function.
    LOG_IF_ENABLE(BOOST_LOG_SEV(lg, DEBUG) << "OUT Set: empty");
    return Domain_t();
  }
  // Map the actual parameter into the formal parameter
  if (llvm::isa<llvm::CallInst>(CallSite) ||
      llvm::isa<llvm::InvokeInst>(CallSite)) {
    Domain_t Formals;
    llvm::ImmutableCallSite CS(CallSite);
    for (unsigned Idx = 0; Idx < CS.getNumArgOperands(); ++Idx) {
      // If an actual parameter is tainted, the corresponding formal parameter
      // must be tainted too
      if (In.count(CS.getArgOperand(Idx))) {
        Formals.insert(getNthFunctionArgument(Callee, Idx));
      }
    }
    LOG_IF_ENABLE(BOOST_LOG_SEV(lg, DEBUG) << "Tainted formal parameter:\n"
                                           << DtoString(Formals));
    return Formals;
  }
  // Pass everything else as identiy
  return In;
}

Domain_t InterMonoTaintAnalysis::returnFlow(const Node_t CallSite,
                                            const Method_t Callee,
                                            const Node_t RetSite,
                                            const Domain_t &In) {
  auto &lg = lg::get();
  LOG_IF_ENABLE(BOOST_LOG_SEV(lg, DEBUG)
                << "InterMonoTaintAnalysis::returnFlow()\n");
  LOG_IF_ENABLE(BOOST_LOG_SEV(lg, DEBUG)
                << "Call-site : " << NtoString(CallSite));
  LOG_IF_ENABLE(BOOST_LOG_SEV(lg, DEBUG)
                << "Callee    : " << MtoString(Callee));
  LOG_IF_ENABLE(BOOST_LOG_SEV(lg, DEBUG)
                << "Ret-site  : " << NtoString(RetSite));
  LOG_IF_ENABLE(BOOST_LOG_SEV(lg, DEBUG) << "IN Set:\n" << DtoString(In));
  // We must check if the return value and formal parameter are tainted, if so
  // we must taint all user's of the function call.
  Domain_t CalleeFacts;
  llvm::ImmutableCallSite CS(CallSite);
  auto RetInst = llvm::dyn_cast<llvm::ReturnInst>(RetSite);
  // Check the return value
  if (In.count(RetInst->getReturnValue())) {
    CalleeFacts.insert(CallSite);
  }
  // If a formal parameter is tainted and of pointer/reference type, the
  // corresponding actual parameter must be tainted too
  for (unsigned Idx = 0; Idx < CS.getNumArgOperands(); ++Idx) {
    auto Formal = getNthFunctionArgument(Callee, Idx);
    if (Formal->getType()->isPointerTy() && In.count(Formal)) {
      CalleeFacts.insert(CS.getArgOperand(Idx));
    }
  }
  LOG_IF_ENABLE(BOOST_LOG_SEV(lg, DEBUG) << "Propagated callee facts:\n"
                                         << DtoString(CalleeFacts));
  return CalleeFacts;
  // All other Callee facts are killed at this point
}

Domain_t InterMonoTaintAnalysis::callToRetFlow(const Node_t CallSite,
                                               const Node_t RetSite,
                                               const Domain_t &In) {
  auto &lg = lg::get();
  LOG_IF_ENABLE(BOOST_LOG_SEV(lg, DEBUG)
                << "InterMonoTaintAnalysis::callToRetFlow()");
  LOG_IF_ENABLE(BOOST_LOG_SEV(lg, DEBUG)
                << "Call-site : " << NtoString(CallSite));
  LOG_IF_ENABLE(BOOST_LOG_SEV(lg, DEBUG)
                << "Ret-site  : " << NtoString(RetSite));
  LOG_IF_ENABLE(BOOST_LOG_SEV(lg, DEBUG) << "IN Set:\n" << DtoString(In));
  // Process the effects of source or sink functions that are called
  for (auto *Callee : ICFG.getCalleesOfCallAt(CallSite)) {
    string FunctionName = cxx_demangle(Callee->getName().str());
    llvm::ImmutableCallSite CS(CallSite);
    // Generate taint values according to source function
    if (SourceSinkFunctions.isSource(FunctionName)) {
      LOG_IF_ENABLE(BOOST_LOG_SEV(lg, DEBUG) << "Plugin SOURCE effects");
      auto Source = SourceSinkFunctions.getSource(FunctionName);
      Domain_t Taints;

      // Add actual parameter to the set of tainted values
      for (auto FormalIndex : Source.TaintedArgs) {
        auto Actual = CS.getArgOperand(FormalIndex);
        Taints.insert(Actual);

        // Also taint all alias of that parameter
        auto PointsToSet = ICFG.getWholeModulePTG().getPointsToSet(Actual);
        for (auto Alias : PointsToSet) {
          Taints.insert(Alias);
        }
      }
      // Add the return value to the set of tainted values
      if (Source.TaintsReturn) {
        Taints.insert(CallSite);
      }
      return Taints;
    }

    // Process leaks
    if (SourceSinkFunctions.isSink(FunctionName)) {
      LOG_IF_ENABLE(BOOST_LOG_SEV(lg, DEBUG) << "Plugin SINK effects");
      auto Sink = SourceSinkFunctions.getSink(FunctionName);
      // Report if a tainted value flows into a sink
      for (unsigned Idx = 0; Idx < CS.getNumArgOperands(); ++Idx) {
        auto Actual = CS.getArgOperand(Idx);
        if (In.count(Actual) && Sink.isLeakedArg(Idx)) {
          LOG_IF_ENABLE(BOOST_LOG_SEV(lg, DEBUG) << "Found a leak!");
          Leaks[CallSite].insert(Actual);
        }
      }
    }
  }
  // Pass everything else as identity
  return In;
}

MonoMap<Node_t, Domain_t> InterMonoTaintAnalysis::initialSeeds() {
  auto &lg = lg::get();
  LOG_IF_ENABLE(BOOST_LOG_SEV(lg, DEBUG)
                << "InterMonoTaintAnalysis::initialSeeds()\n");
  // Taint the commandline arguments
  MonoMap<Node_t, Domain_t> Seeds;
  for (auto &EntryPointName : EntryPoints) {
    const Method_t EntryPointMethod = ICFG.getMethod(EntryPointName);
    if (EntryPointName == "main") {
      Domain_t CmdArgs;
      for (auto &Arg : EntryPointMethod->args()) {
        CmdArgs.insert(&Arg);
      }
      Seeds.insert(make_pair(&EntryPointMethod->front().front(), CmdArgs));
    } else {
      Seeds.insert(make_pair(&EntryPointMethod->front().front(), Domain_t()));
    }
  }
  return Seeds;
}

string InterMonoTaintAnalysis::DtoString(const Domain_t d) {
  string str;
  for (auto fact : d) {
    str += llvmIRToString(fact) + '\n';
  }
  return str;
}

string InterMonoTaintAnalysis::MtoString(const Method_t m) {
  return m->getName().str();
}

string InterMonoTaintAnalysis::NtoString(const Node_t n) {
  return llvmIRToString(n);
}

bool InterMonoTaintAnalysis::recompute(const Method_t Callee) { return false; }

void InterMonoTaintAnalysis::printLeaks() const {
  cout << "\n----- Found the following leaks -----\n";
  if (Leaks.empty()) {
    cout << "No leaks found!\n";
  } else {
    for (auto Leak : Leaks) {
      string ModuleName = getModuleNameFromVal(Leak.first);
      cout << "At instruction:  '" << llvmIRToString(Leak.first) << "'\n";
      cout << "In file:         '" << ModuleName << "'\n";
      for (auto LeakValue : Leak.second) {
        cout << "Leak:            '" << llvmIRToString(LeakValue) << "'\n";
      }
      cout << "-------------------\n";
    }
  }
}

} // namespace psr
