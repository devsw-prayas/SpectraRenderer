project(SpectraDX12Backend)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

add_library(SpectraCore SHARED 
	src/Private/SpectraCore.cpp src/Public/SpectraCore.h
	src/Private/S_int4.cpp src/Public/S_int4.h
	src/Private/S_uint4.cpp src/Public/S_uint4.h
	src/Private/ThreadFactory.cpp src/Public/ThreadFactory.h
	src/Private/ThreadPool.cpp src/Public/ThreadPool.h
	src/Public/ThreadSafeQueue.h src/Public/ThreadSafeQueue.inl
	src/Public/ThreadSafePriorityQueue.h src/Public/ThreadSafePriorityQueue.inl
	src/Public/PlatformHandles.h
	src/Public/Vector.h src/Private/Vector.cpp
)

target_include_directories(SpectraCore PUBLIC src/Public)

target_link_libraries(SpectraCore SpectraInstrumentation, SpectraSTL)

