# Streams and Events

**Headers:** `Public/CudaStream.h`, `Public/CudaEvent.h`

## DeviceStreams (Spectra::Cuda::Streams)

Streams are ordered execution queues. Work submitted on the same stream runs in-order; different streams run concurrently (subject to hardware limits).

```cpp
#include "CudaStream.h"
using namespace Spectra::Cuda::Streams;
using namespace Spectra::Cuda::Utils;

// Default stream (synchronizes with all other streams)
GpuStream defaultStream = DeviceStreams::createStream(StreamFlags::DEFAULT);

// Non-blocking stream (runs independently of the default stream)
GpuStream computeStream = DeviceStreams::createStream(StreamFlags::NON_BLOCKING);

// Prioritized stream (lower number = higher priority; 0 is typically the highest)
int highPriority = -1;
GpuStream highPriStream = DeviceStreams::createStreamWithPriority(StreamFlags::NON_BLOCKING, highPriority);

// Block CPU until all previously submitted work on the stream completes
DeviceStreams::syncStream(computeStream);

// Non-blocking poll (true = all work done)
bool done = DeviceStreams::queryStream(computeStream);

// Make stream wait for an event recorded on another stream (GPU-side dependency, no CPU block)
DeviceStreams::streamWaitEvent(computeStream, someEvent);

// Host callback (called on CPU when stream reaches this point)
auto callback = [](void* userData) { /* ... */ };
DeviceStreams::addStreamCallback(computeStream, reinterpret_cast<void*>(+callback), nullptr);

// Destroy
DeviceStreams::destroyStream(computeStream);
```

### Graph capture from a stream

```cpp
DeviceStreams::beginStreamCapture(computeStream, StreamCaptureMode::GLOBAL);
// ... submit kernel launches, memcopies etc. on computeStream ...
GpuGraph captured = DeviceStreams::endStreamCapture(computeStream);
// captured is now a replayable CUDA graph (see graphs.md)
```

`StreamCaptureMode` values:

| Mode | Behaviour |
|------|-----------|
| `GLOBAL` | Any work that cross-stream syncs with this stream is also captured |
| `THREAD_LOCAL` | Only work explicitly submitted on this stream is captured |
| `RELAXED` | Like GLOBAL but allows some synchronization actions during capture |

## DeviceEvents (Spectra::Cuda::Events)

Events are GPU-side timestamps / synchronization points recorded into a stream.

```cpp
#include "CudaEvent.h"
using namespace Spectra::Cuda::Events;
using namespace Spectra::Cuda::Utils;

// Create with timing enabled
uint32_t timedFlags = CudaHelpers::computeEventFlags({ EventFlags::DEFAULT });
GpuEvent startEvent = DeviceEvents::createEvent(timedFlags);
GpuEvent stopEvent  = DeviceEvents::createEvent(timedFlags);

// Create without timing (cheaper, use when you only need synchronization)
uint32_t syncOnlyFlags = CudaHelpers::computeEventFlags({ EventFlags::DISABLE_TIMING });
GpuEvent syncEvent = DeviceEvents::createEvent(syncOnlyFlags);

// Record into a stream (GPU records its timestamp when it reaches this point)
DeviceEvents::recordEvent(startEvent, computeStream);
// ... launch kernels ...
DeviceEvents::recordEvent(stopEvent, computeStream);

// CPU-side wait until the event is reached by the GPU
DeviceEvents::syncEvent(stopEvent);

// Non-blocking poll
bool reached = DeviceEvents::queryEvent(stopEvent);

// Elapsed time between two recorded events (milliseconds, float)
float ms = DeviceEvents::elapsedTime(startEvent, stopEvent);

// Cross-process event sharing (IPC)
GpuIpcEventHandle ipcHandle = DeviceEvents::getIpcHandle(startEvent);
// Send ipcHandle to another process (e.g. via shared memory or socket)
GpuEvent imported = DeviceEvents::openIpcHandle(ipcHandle);

// Destroy
DeviceEvents::destroyEvent(startEvent);
DeviceEvents::destroyEvent(stopEvent);
```

`EventFlags` (combine with `CudaHelpers::computeEventFlags`):

| Flag | Effect |
|------|--------|
| `DEFAULT` | Standard event with timing |
| `BLOCKING_SYNC` | CPU sleeps in `syncEvent` instead of spinning |
| `DISABLE_TIMING` | No timestamp; cheaper to create and record |
| `INTERPROCESS` | Enables `getIpcHandle` for cross-process sharing |

### Common pattern: cross-stream ordering

```cpp
// Record on upload stream when DMA finishes
DeviceEvents::recordEvent(uploadDone, uploadStream);

// Make compute stream wait for upload before processing
DeviceStreams::streamWaitEvent(computeStream, uploadDone);

// Launch kernels on compute stream immediately after (no CPU block)
DeviceCompute::launchKernel(..., computeStream, ...);
```
