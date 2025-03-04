target_sources(DommusCore
	PRIVATE
	Array.h
	AtomicVec2.cpp
	AtomicVec2.h
	FixedString.h
	ReciclingVector.h
	SparseSet.h
	Vector.h
	CVector.h
)

target_include_directories(DommusCore
	PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}
)
