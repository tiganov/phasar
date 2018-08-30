set(MonoSources
	InterMonoGeneralizedSolverTest.cpp
)

foreach(TEST_SRC ${MonoSources})
	add_phasar_unittest(${TEST_SRC})
endforeach(TEST_SRC)
