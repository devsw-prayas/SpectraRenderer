#include "SpectraCudaBackend.h"
#include "CudaGraph.h"

#define ALLOW_SYSCALL
#include "SpecCudaSyscall.h"

#define ALLOW_HELPERS
#include "CudaInternalHelpers.h"

namespace Spectra::Cuda::Graphs {
	GpuGraph DeviceGraphs::createGraph() {
		CUgraph graph;
		GpuGraph ro_Graph{};

		Instrumentation::staticSwitch(cuGraphCreate(&graph, 0),
			Instrumentation::caseOf<CUDA_SUCCESS>([&]{ ro_Graph.m_GraphHandle = graph; }),
			Instrumentation::otherwise(Internal::trapCudaError));
		return ro_Graph;
	}

	void DeviceGraphs::destroyGraph(GpuGraph& ro_Graph) {
		if (!ro_Graph.isValid()) return;

		Instrumentation::staticSwitch(cuGraphDestroy(static_cast<CUgraph>(ro_Graph.m_GraphHandle)),
			Instrumentation::caseOf<CUDA_SUCCESS>([&]{ ro_Graph.m_GraphHandle = nullptr; }),
			Instrumentation::otherwise(Internal::trapCudaError));
	}

	GpuGraphNode DeviceGraphs::addKernelNode(GpuGraph& ro_Graph, const GpuGraphNode* p_Deps, uint32_t v_DepCount, const KernelNodeParams& ro_Params) {
		SPEC_CUDA_BK_ASSERT(ro_Graph.isValid());
		SPEC_CUDA_BK_ASSERT(ro_Params.m_Function != nullptr);
		SPEC_CUDA_BK_ASSERT(v_DepCount <= MAX_GRAPH_DEPS);

		CUgraphNode depBuf[MAX_GRAPH_DEPS];
		for (uint32_t i = 0; i < v_DepCount; ++i)
			depBuf[i] = static_cast<CUgraphNode>(p_Deps[i].m_NodeHandle);

		const CUDA_KERNEL_NODE_PARAMS params = Internal::CUDA_PackingFunctions::packKernelNodeParams(ro_Params);
		CUgraphNode node;
		GpuGraphNode ro_Node{};

		Instrumentation::staticSwitch(cuGraphAddKernelNode(
				&node,
				static_cast<CUgraph>(ro_Graph.m_GraphHandle),
				v_DepCount > 0 ? depBuf : nullptr,
				static_cast<size_t>(v_DepCount),
				&params
			),
			Instrumentation::caseOf<CUDA_SUCCESS>([&]{ ro_Node.m_NodeHandle = node; }),
			Instrumentation::otherwise(Internal::trapCudaError));
		return ro_Node;
	}

	GpuGraphNode DeviceGraphs::addMemcpyNode(GpuGraph& ro_Graph, const GpuGraphNode* p_Deps, uint32_t v_DepCount, const MemCpy3DDesc& ro_Desc) {
		SPEC_CUDA_BK_ASSERT(ro_Graph.isValid());
		SPEC_CUDA_BK_ASSERT(v_DepCount <= MAX_GRAPH_DEPS);

		CUgraphNode depBuf[MAX_GRAPH_DEPS];
		for (uint32_t i = 0; i < v_DepCount; ++i)
			depBuf[i] = static_cast<CUgraphNode>(p_Deps[i].m_NodeHandle);

		const CUDA_MEMCPY3D copyParams = Internal::CUDA_PackingFunctions::pack3dMemcpyDesc(ro_Desc);

		CUcontext ctx{};
		cuCtxGetCurrent(&ctx);

		CUgraphNode node;
		GpuGraphNode ro_Node{};

		Instrumentation::staticSwitch(cuGraphAddMemcpyNode(
				&node,
				static_cast<CUgraph>(ro_Graph.m_GraphHandle),
				v_DepCount > 0 ? depBuf : nullptr,
				static_cast<size_t>(v_DepCount),
				&copyParams,
				ctx
			),
			Instrumentation::caseOf<CUDA_SUCCESS>([&]{ ro_Node.m_NodeHandle = node; }),
			Instrumentation::otherwise(Internal::trapCudaError));
		return ro_Node;
	}

	GpuGraphNode DeviceGraphs::addMemsetNode(GpuGraph& ro_Graph, const GpuGraphNode* p_Deps, uint32_t v_DepCount, const MemsetNodeParams& ro_Params) {
		SPEC_CUDA_BK_ASSERT(ro_Graph.isValid());
		SPEC_CUDA_BK_ASSERT(v_DepCount <= MAX_GRAPH_DEPS);

		CUgraphNode depBuf[MAX_GRAPH_DEPS];
		for (uint32_t i = 0; i < v_DepCount; ++i)
			depBuf[i] = static_cast<CUgraphNode>(p_Deps[i].m_NodeHandle);

		const CUDA_MEMSET_NODE_PARAMS params = Internal::CUDA_PackingFunctions::packMemsetNodeParams(ro_Params);

		CUcontext ctx{};
		cuCtxGetCurrent(&ctx);

		CUgraphNode node;
		GpuGraphNode ro_Node{};

		Instrumentation::staticSwitch(cuGraphAddMemsetNode(
				&node,
				static_cast<CUgraph>(ro_Graph.m_GraphHandle),
				v_DepCount > 0 ? depBuf : nullptr,
				static_cast<size_t>(v_DepCount),
				&params,
				ctx
			),
			Instrumentation::caseOf<CUDA_SUCCESS>([&]{ ro_Node.m_NodeHandle = node; }),
			Instrumentation::otherwise(Internal::trapCudaError));
		return ro_Node;
	}

	void DeviceGraphs::addDependencies(GpuGraph& ro_Graph, const GpuGraphNode* p_From, const GpuGraphNode* p_To, uint32_t v_Count, const GpuGraphEdgeData* p_Data) {
		SPEC_CUDA_BK_ASSERT(ro_Graph.isValid());
		SPEC_CUDA_BK_ASSERT(p_From != nullptr);
		SPEC_CUDA_BK_ASSERT(p_To != nullptr);
		SPEC_CUDA_BK_ASSERT(v_Count > 0 && v_Count <= MAX_GRAPH_DEPS);

		CUgraphNode fromBuf[MAX_GRAPH_DEPS];
		CUgraphNode toBuf[MAX_GRAPH_DEPS];
		for (uint32_t i = 0; i < v_Count; ++i) {
			fromBuf[i] = static_cast<CUgraphNode>(p_From[i].m_NodeHandle);
			toBuf[i] = static_cast<CUgraphNode>(p_To[i].m_NodeHandle);
		}

		CUgraphEdgeData data{};
		if (p_Data) {
			data.from_port = p_Data->m_FromPort;
			data.to_port = p_Data->m_ToPort;
			data.type = p_Data->m_Type;
		}
		Instrumentation::staticSwitch(cuGraphAddDependencies(
				static_cast<CUgraph>(ro_Graph.m_GraphHandle),
				fromBuf,
				toBuf,
				static_cast<const CUgraphEdgeData*>(&data),
				static_cast<size_t>(v_Count)
			),
			Instrumentation::caseOf<CUDA_SUCCESS>([]{}),
			Instrumentation::otherwise(Internal::trapCudaError));
	}

	GpuGraphExec DeviceGraphs::instantiate(const GpuGraph& ro_Graph) {
		SPEC_CUDA_BK_ASSERT(ro_Graph.isValid());

		CUgraphExec exec;
		GpuGraphExec ro_Exec{};

		Instrumentation::staticSwitch(cuGraphInstantiate(&exec, static_cast<CUgraph>(ro_Graph.m_GraphHandle), 0),
			Instrumentation::caseOf<CUDA_SUCCESS>([&]{ ro_Exec.m_ExecHandle = exec; }),
			Instrumentation::otherwise(Internal::trapCudaError));
		return ro_Exec;
	}

	void DeviceGraphs::launch(const GpuGraphExec& ro_Exec, const GpuStream& ro_Stream) {
		SPEC_CUDA_BK_ASSERT(ro_Exec.isValid());
		SPEC_CUDA_BK_ASSERT(ro_Stream.isValid());

		Instrumentation::staticSwitch(cuGraphLaunch(
				static_cast<CUgraphExec>(ro_Exec.m_ExecHandle),
				static_cast<CUstream>(ro_Stream.m_StreamHandle)
			),
			Instrumentation::caseOf<CUDA_SUCCESS>([]{}),
			Instrumentation::otherwise(Internal::trapCudaError));
	}

	bool DeviceGraphs::execUpdate(GpuGraphExec& ro_Exec, const GpuGraph& ro_NewGraph) {
		SPEC_CUDA_BK_ASSERT(ro_Exec.isValid());
		SPEC_CUDA_BK_ASSERT(ro_NewGraph.isValid());

		CUgraphExecUpdateResultInfo resultInfo{};
		const CUresult result = cuGraphExecUpdate(
			static_cast<CUgraphExec>(ro_Exec.m_ExecHandle),
			static_cast<CUgraph>(ro_NewGraph.m_GraphHandle),
			&resultInfo
		);

		if (result == CUDA_SUCCESS && resultInfo.result == CU_GRAPH_EXEC_UPDATE_SUCCESS) return true;
		return false;
	}

	void DeviceGraphs::destroyExec(GpuGraphExec& ro_Exec) {
		if (!ro_Exec.isValid()) return;

		Instrumentation::staticSwitch(cuGraphExecDestroy(static_cast<CUgraphExec>(ro_Exec.m_ExecHandle)),
			Instrumentation::caseOf<CUDA_SUCCESS>([&]{ ro_Exec.m_ExecHandle = nullptr; }),
			Instrumentation::otherwise(Internal::trapCudaError));
	}
}
