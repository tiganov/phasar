/******************************************************************************
 * Copyright (c) 2017 Philipp Schubert.
 * All rights reserved. This program and the accompanying materials are made
 * available under the terms of LICENSE.txt.
 *
 * Contributors:
 *     Philipp Schubert and others
 *****************************************************************************/

#include <gtest/gtest.h>
#include <memory>
#include <phasar/DB/ProjectIRDB.h>
#include <phasar/PhasarLLVM/ControlFlow/LLVMBasedICFG.h>
#include <phasar/PhasarLLVM/IfdsIde/LLVMZeroValue.h>
#include <phasar/PhasarLLVM/Mono/Contexts/CallString.h>
#include <phasar/PhasarLLVM/Mono/Contexts/ValueBasedContext.h>
#include <phasar/PhasarLLVM/Mono/Problems/InterMonoTaintAnalysis.h>
#include <phasar/PhasarLLVM/Mono/Solver/LLVMInterMonoSolver.h>
#include <phasar/PhasarLLVM/Passes/ValueAnnotationPass.h>
#include <phasar/PhasarLLVM/Pointer/LLVMTypeHierarchy.h>
#include <phasar/Utils/LLVMShorthands.h>
#include <phasar/Utils/Logger.h>

using namespace std;
using namespace psr;

/* ============== TEST FIXTURE ============== */

class InterMonoTaintAnalysisTest : public ::testing::Test {
protected:
  const std::string pathToLLFiles =
      PhasarDirectory + "build/test/llvm_test_code/taint_analysis/";
  // Currently the solver does only support a single function as entry point.
  // However, the ICFG and the Taint-Analysis problem can be initialized with
  // an vector of entry points.
  const std::vector<std::string> EntryPoints = {"main"};

  ProjectIRDB *IRDB;
  LLVMTypeHierarchy *TH;
  LLVMBasedICFG *ICFG;
  InterMonoTaintAnalysis *TaintProblem;
  TaintSensitiveFunctions *TSF;

public:
  InterMonoTaintAnalysisTest() = default;
  virtual ~InterMonoTaintAnalysisTest() = default;

  void Initialize(const std::vector<std::string> &IRFiles) {
    IRDB = new ProjectIRDB(IRFiles);
    IRDB->preprocessIR();
    TH = new LLVMTypeHierarchy(*IRDB);
    ICFG =
        new LLVMBasedICFG(*TH, *IRDB, CallGraphAnalysisType::OTF, EntryPoints);
    TSF = new TaintSensitiveFunctions(true);
    TaintProblem = new InterMonoTaintAnalysis(*ICFG, *TSF, EntryPoints);
  }

  void SetUp() override {
    bl::core::get()->set_logging_enabled(false);
    ValueAnnotationPass::resetValueID();
  }

  void TearDown() override {
    delete IRDB;
    delete TH;
    delete ICFG;
    delete TaintProblem;
    delete TSF;
  }

  // void compareResults(map<int, set<string>> &GroundTruth) {
  //   // std::map<n_t, std::set<d_t>> Leaks;
  //   map<int, set<string>> FoundLeaks;
  //   for (auto Leak : TaintProblem->Leaks) {
  //     int SinkId = stoi(getMetaDataID(Leak.first));
  //     set<string> LeakedValueIds;
  //     for (auto LV : Leak.second) {
  //       LeakedValueIds.insert(getMetaDataID(LV));
  //     }
  //     FoundLeaks.insert(make_pair(SinkId, LeakedValueIds));
  //   }
  //   EXPECT_EQ(FoundLeaks, GroundTruth);
  // }
}; // Test Fixture

TEST_F(InterMonoTaintAnalysisTest, IMTaintTest_01) {
  Initialize({pathToLLFiles + "dummy_source_sink/taint_01.ll"});
  CallString<typename InterMonoTaintAnalysis::Node_t,
             typename InterMonoTaintAnalysis::Domain_t, 2>
      CS;
  const llvm::Function *F =
      ICFG->getMethod("main"); /*IRDB.getFunction(EntryPoints.front());*/
  // LLVMInterMonoSolver solver(*TaintProblem,CS,F);
  // auto S1 = make_LLVMBasedIMS(*TaintProblem, CS, F);
  // S1->solve();
  // S1->dumpResults();

  // ValueBasedContext<typename InterMonoTaintAnalysis::Node_t,
  //                   typename InterMonoTaintAnalysis::Domain_t>
  //     VBC;
  // auto S2 = make_LLVMBasedIMS(*TaintProblem, VBC, ICFG->getMethod("main"));
  // S2->solve();
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
