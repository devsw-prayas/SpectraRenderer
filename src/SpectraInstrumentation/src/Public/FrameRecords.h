#pragma once
#include "SpectraInstrumentation.h"
#include "SpecInstDiagnostic.h"
#include <cstdint>
#include <type_traits>

// Integers only on the hot path; labels and codes resolve offline (scripts/resolver).
namespace Spectra::Instrumentation {
	enum class FrameEventType : uint8_t {
		BEGIN,
		END
	};

	// Begin/End pair up by LIFO replay of one FrameId's stream, so no join key is stored.
	struct CalleeFrameRecord final {
		uint64_t m_Timestamp = 0;
		uint64_t m_FmtStringOffset = 0;  // image-relative label address
		uint32_t m_FrameId = 0;          // fiber identity, unlike ThreadId it survives migration
		uint32_t m_ThreadId = 0;
		uint32_t m_Depth = 0;
		FrameEventType m_EventType = FrameEventType::BEGIN;

		CalleeFrameRecord() = default;
		constexpr CalleeFrameRecord(uint64_t v_Timestamp, uint64_t v_FmtStringOffset, uint32_t v_FrameId, uint32_t v_ThreadId,
			uint32_t v_Depth, FrameEventType v_EventType) noexcept
			: m_Timestamp(v_Timestamp), m_FmtStringOffset(v_FmtStringOffset), m_FrameId(v_FrameId), m_ThreadId(v_ThreadId),
			  m_Depth(v_Depth), m_EventType(v_EventType) {
		}
		~CalleeFrameRecord() = default;

		CalleeFrameRecord(const CalleeFrameRecord&) = default;
		CalleeFrameRecord& operator=(const CalleeFrameRecord&) = default;

		CalleeFrameRecord(CalleeFrameRecord&&) noexcept = default;
		CalleeFrameRecord& operator=(CalleeFrameRecord&&) noexcept = default;
	};

	SPEC_INST_STATIC_ASSERT(sizeof(CalleeFrameRecord) == 32, "CalleeFrameRecord layout is locked at 32 B");
	SPEC_INST_STATIC_ASSERT(alignof(CalleeFrameRecord) == 8, "CalleeFrameRecord must be 8-aligned");

	// Un-popped entries at terminate() are the open fail-fast scopes: top = root cause.
	struct ErrContext final {
		uint64_t m_CalleeFrameRecordPtr = 0; // may be stale once the callee ring wraps (accepted)
		uint32_t m_DomainCode = 0;
		int32_t m_RawCode = 0;

		ErrContext() = default;
		constexpr ErrContext(uint64_t v_CalleeFrameRecordPtr, uint32_t v_DomainCode, int32_t v_RawCode) noexcept
			: m_CalleeFrameRecordPtr(v_CalleeFrameRecordPtr), m_DomainCode(v_DomainCode), m_RawCode(v_RawCode) {
		}
		~ErrContext() = default;

		ErrContext(const ErrContext&) = default;
		ErrContext& operator=(const ErrContext&) = default;

		ErrContext(ErrContext&&) noexcept = default;
		ErrContext& operator=(ErrContext&&) noexcept = default;
	};

	SPEC_INST_STATIC_ASSERT(sizeof(ErrContext) == 16, "ErrContext layout is locked at 16 B");

	// One index type for both "which slot" and "next slot in the chain".
	using ExecHandle = uint16_t;
	inline constexpr ExecHandle kExecChainEnd = 0xFFFF;
	inline constexpr ExecHandle kExecNoException = 0xFFFF;
	inline constexpr size_t kExecPoolCapacity = 4096;

	SPEC_INST_STATIC_ASSERT(kExecPoolCapacity <= kExecChainEnd, "pool indices must stay below the sentinel");

	// Non-polymorphic on purpose: vtable layout isn't ABI-stable across the toolchains that
	// share these across DLL boundaries.
	struct InstrumentedException final {
		uint64_t m_OriginFrameRecordPtr = 0;
		uint32_t m_DomainCode = 0;
		int32_t m_RawCode = 0;
		ExecHandle m_Next = kExecChainEnd;

		InstrumentedException() = default;
		constexpr InstrumentedException(uint64_t v_OriginFrameRecordPtr, uint32_t v_DomainCode, int32_t v_RawCode,
			ExecHandle v_Next = kExecChainEnd) noexcept
			: m_OriginFrameRecordPtr(v_OriginFrameRecordPtr), m_DomainCode(v_DomainCode), m_RawCode(v_RawCode), m_Next(v_Next) {
		}
		~InstrumentedException() = default;

		InstrumentedException(const InstrumentedException&) = default;
		InstrumentedException& operator=(const InstrumentedException&) = default;

		InstrumentedException(InstrumentedException&&) noexcept = default;
		InstrumentedException& operator=(InstrumentedException&&) noexcept = default;
	};

	SPEC_INST_STATIC_ASSERT(sizeof(InstrumentedException) == 24, "InstrumentedException is 18 B of fields padded to 24");
	SPEC_INST_STATIC_ASSERT(alignof(InstrumentedException) == 8, "InstrumentedException must be 8-aligned");

	SPEC_INST_STATIC_ASSERT(std::is_trivially_copyable_v<CalleeFrameRecord> && std::is_trivially_copyable_v<ErrContext>
		&& std::is_trivially_copyable_v<InstrumentedException>, "records are memcpy'd in and out of ring/pool storage");

	inline constexpr size_t kCalleeMainCapacity = size_t{ 1 } << 18;
	inline constexpr size_t kCalleeDrainCapacity = size_t{ 1 } << 17;
	inline constexpr size_t kErrStackCapacity = 256;
}
