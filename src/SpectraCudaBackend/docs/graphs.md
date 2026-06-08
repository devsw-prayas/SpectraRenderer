# CUDA Graphs

**Header:** `Public/CudaGraph.h`  
**Namespace:** `Spectra::Cuda::Graphs`

CUDA Graphs record a sequence of operations (kernels, memcopies, memsets) as a replayable DAG. Once instantiated, repeated launches avoid per-launch CPU overhead. Use them for stable workloads that run every frame.

## Building a graph manually

```cpp
#include "CudaGraph.h"
using namespace Spectra::Cuda::Graphs;
using namespace Spectra::Cuda::Utils;

GpuGraph graph = DeviceGraphs::createGraph();

// Root kernel node (no dependencies)
KernelNodeParams params{};
params.m_Function     = fn.m_FunctionHandle;   // raw CUfunction from GpuFunction
params.m_GridDimX     = (N + 255) / 256;
params.m_BlockDimX    = 256;
params.m_KernelParams = kernelParamPtrs;

GpuGraphNode kernelNode = DeviceGraphs::addKernelNode(
    graph, /*deps=*/nullptr, /*depCount=*/0, params);

// Memset node that depends on the kernel completing
MemsetNodeParams msParams{};
msParams.m_Dst         = resultBuf.m_GpuAddr;
msParams.m_Value       = 0;
msParams.m_ElementSize = 4;
msParams.m_Width       = N;
msParams.m_Height      = 1;

GpuGraphNode memsetNode = DeviceGraphs::addMemsetNode(
    graph, &kernelNode, 1, msParams);

// Memcpy node after memset
MemCpy3DDesc cpyDesc{};
initMemCpy3DDesc(cpyDesc);
setMemCpy3DSrcDevice(cpyDesc, srcBuf);
setMemCpy3DDstDevice(cpyDesc, dstBuf);
setMemCpy3DDimensions(cpyDesc, N * sizeof(float), 1, 1);

GpuGraphNode copyNode = DeviceGraphs::addMemcpyNode(
    graph, &memsetNode, 1, cpyDesc);

// Add an explicit extra dependency edge if needed
DeviceGraphs::addDependencies(graph, &kernelNode, &copyNode, 1);
// max v_Count is MAX_GRAPH_DEPS (64)
```

## Instantiation and launch

```cpp
// Compile the graph into an executable
GpuGraphExec exec = DeviceGraphs::instantiate(graph);

// Launch (no CPU overhead for the individual nodes)
DeviceGraphs::launch(exec, stream);
DeviceStreams::syncStream(stream);

// Relaunch as many times as needed
DeviceGraphs::launch(exec, stream);
```

## Hot-patching parameters without re-instantiating

If only kernel parameters change (not the graph topology), `execUpdate` avoids the cost of a full `instantiate`:

```cpp
// Modify the source graph's node params in-place (via addKernelNode on a new graph)
GpuGraph updatedGraph = DeviceGraphs::createGraph();
// ... add same topology with new params ...

bool ok = DeviceGraphs::execUpdate(exec, updatedGraph);
if (!ok) {
    // Topology changed, must re-instantiate
    DeviceGraphs::destroyExec(exec);
    exec = DeviceGraphs::instantiate(updatedGraph);
}
DeviceGraphs::destroyGraph(updatedGraph);
```

## Graph capture from a stream

The simpler way to build a graph when the submission code is already written:

```cpp
#include "CudaStream.h"
using namespace Spectra::Cuda::Streams;

GpuStream captureStream = DeviceStreams::createStream(StreamFlags::NON_BLOCKING);

DeviceStreams::beginStreamCapture(captureStream, StreamCaptureMode::THREAD_LOCAL);

// Submit work as normal
DeviceCompute::launchKernel(fn, grid, block, 0, captureStream, params);
PinnedMemory::copyHostToDevAsync(dst, src, bytes, captureStream);

GpuGraph captured = DeviceStreams::endStreamCapture(captureStream);
GpuGraphExec exec  = DeviceGraphs::instantiate(captured);

// captureStream is now free to use for other work
```

## Teardown

```cpp
DeviceGraphs::destroyExec(exec);
DeviceGraphs::destroyGraph(graph);
```
