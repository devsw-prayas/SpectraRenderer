#include "KerbecsTests.h"
#include "ValidationFixture.h"
#include "Kerbecs.h"
#include "ShadowPtr.h"
#include "MemoryLayouts.h"
#include "MemoryZone.h"
#include "Logger.h"
#include "ShadowMap.h"

#include <tuple>
#include <type_traits>
#include <thread>
#include <typeinfo>
#include <cstdio>

namespace KerbecsValidation {

    using namespace Kerbecs;
    struct alignas(1)  Payload1B   { uint8_t v; };
    struct alignas(4)  Payload4B   { float v; };
    struct alignas(16) Vector4     { float x, y, z, w; };
    struct alignas(32) Matrix4x4   { float m[16]; };
    struct alignas(64) L2CacheBlock{ uint8_t b[128]; };

    // Alignments
    template <size_t A> struct AlignmentConstraint { static constexpr size_t value = A; };
    using AlignDefault = AlignmentConstraint<0>; // Use alignof(T)
    using AlignAVX2    = AlignmentConstraint<32>;
    using AlignAVX512  = AlignmentConstraint<64>;
    using AlignL2      = AlignmentConstraint<128>;

    // Counts
    template <size_t C> struct AllocTupleCount { static constexpr size_t value = C; };
    using CountSingle = AllocTupleCount<1>;
    using CountSmall  = AllocTupleCount<10>;
    using CountLarge  = AllocTupleCount<1024>;
    using CountPage   = AllocTupleCount<8192>;

    // Concurrency Contexts
    struct ConcurrencySingle {};
    struct ConcurrencyMulti {};

    struct DummyHasher {
        void     add(uint64_t) noexcept {}
        uint64_t finalize() const noexcept { return 0; }
        void     reset() noexcept {}
    };

    struct ViolationLogger {
        void report(const Kerbecs::Violation&) noexcept {}
    };

    static Spectra::Validation::Validator* s_pValidator = nullptr;

    template <
        template<typename, typename, typename, typename, typename> class FixtureTempl,
        typename LayoutPol,
        typename PayloadT,
        typename CountT,
        typename AlignT,
        typename ConcT
    >
    struct CombinatorialBase : Spectra::Validation::ValidationFixture<FixtureTempl<LayoutPol, PayloadT, CountT, AlignT, ConcT>, CpuAdapter> {

        using Base = Spectra::Validation::ValidationFixture<FixtureTempl<LayoutPol, PayloadT, CountT, AlignT, ConcT>, CpuAdapter>;

        using allocator_type = Allocators::GlobalAllocator;
        using shadow_ptr = Kerbecs::Shadow::ShadowPtr<LayoutPol, KerbecsShadowMap, ViolationLogger, DummyHasher, allocator_type>;

        shadow_ptr m_Shadow;
        DummyHasher m_Hasher;
        ViolationLogger m_KerbecsLogger;
        KerbecsShadowMap m_ShadowMap;

        explicit CombinatorialBase(CpuAdapter& ro_Adapter)
            : Base(ro_Adapter, *s_pValidator) {
            
            // Link up the injection dependencies
            m_Shadow.m_Map = &m_ShadowMap;
            m_Shadow.m_Logger = &m_KerbecsLogger;
            m_Shadow.m_Hasher = &m_Hasher;
            m_Shadow.m_Name = "CombinatorialTestCore";
            
            // Grab the global allocator from instance for tests (needs to be initialized)
            auto& zone = MemoryZone::instance();
            if(!zone.m_Initialized) zone.init();
            m_Shadow.m_UserAllocator = &zone.m_GlobalAllocator;
        }

        size_t alignment() const {
            return AlignT::value == 0 ? alignof(PayloadT) : AlignT::value;
        }

        uint64_t getDeterminismHashImpl() {
            return 0; // functional tests
        }

        void dumpAndVerify(bool v_Cond, const char* p_Message) {
            if (!v_Cond) {
                auto& zone = Kerbecs::MemoryZone::instance();
                std::fprintf(stderr,
                    "[DIAG] FAIL: \"%s\"\n"
                    "  Layout=%-30s  Payload=%-20s  Count=%-6zu  Align=%-4zu  Conc=%s\n"
                    "  Shadow { BlockBase=%p  RawPtr=%p  Allocator=%p  ZoneInit=%d }\n",
                    p_Message,
                    typeid(LayoutPol).name(), typeid(PayloadT).name(),
                    CountT::value,
                    AlignT::value == 0 ? alignof(PayloadT) : AlignT::value,
                    std::is_same_v<ConcT, ConcurrencySingle> ? "Single" : "Multi",
                    this->m_Shadow.m_BlockBase,
                    this->m_Shadow.m_RawPtr,
                    static_cast<void*>(this->m_Shadow.m_UserAllocator),
                    static_cast<int>(zone.m_Initialized));
            }
            this->verify(v_Cond, p_Message);
        }
    };

    template <typename L, typename P, typename C, typename A, typename CX>
    struct LifecycleFixture : CombinatorialBase<LifecycleFixture, L, P, C, A, CX> {
        using Base = CombinatorialBase<LifecycleFixture, L, P, C, A, CX>;

        explicit LifecycleFixture(CpuAdapter& ro_Adapter)
            : Base(ro_Adapter) {}

        void startupImpl() {
            bool allocOK = Kerbecs::Shadow::shadowAllocate<P>(&this->m_Shadow, C::value);
            this->dumpAndVerify(allocOK, "shadowAllocate failed");
        }

        void executeImpl() {
            if constexpr (std::is_same_v<CX, ConcurrencySingle>) {
                _exec();
            } else {
                std::thread t([this] { _exec(); });
                t.join();
            }
        }

        void _exec() {
            // Construct first element
            bool ctorOK = Kerbecs::Shadow::shadowConstruct<P>(&this->m_Shadow);
            this->verify(ctorOK, "shadowConstruct failed");

            Kerbecs::Shadow::Utils::MemoryState s = Kerbecs::Shadow::shadowGetMemoryState<P>(&this->m_Shadow, 0);
            this->m_Validator.expect(s == Kerbecs::Shadow::Utils::MemoryState::CONSTRUCTED, "Object should be CONSTRUCTED");
        }

        void resetImpl(CpuAdapter&) {}

        void teardownImpl() {
            // Destroy index 0 (the one that was constructed in executeImpl).
            bool dtorOK = Kerbecs::Shadow::shadowDestroy<P>(&this->m_Shadow, 0);
            this->verify(dtorOK, "shadowDestroy failed");

            // For C > 1, indices 1..C-1 were allocated but never constructed.
            // Construct then immediately destroy each so liveCount reaches 0,
            // allowing the registry node to retire cleanly.
            for (size_t i = 1; i < C::value; ++i) {
                Kerbecs::Shadow::shadowConstructAt<P>(&this->m_Shadow, i);
                Kerbecs::Shadow::shadowDestroy<P>(&this->m_Shadow, i);
            }
        }
    };

    template <typename L, typename P, typename C, typename A, typename CX>
    struct PartialInitFixture : CombinatorialBase<PartialInitFixture, L, P, C, A, CX> {
        using Base = CombinatorialBase<PartialInitFixture, L, P, C, A, CX>;

        explicit PartialInitFixture(CpuAdapter& ro_Adapter) : Base(ro_Adapter) {}

        void startupImpl() {
            this->dumpAndVerify(Kerbecs::Shadow::shadowAllocate<P>(&this->m_Shadow, C::value), "Allocate failed");
        }

        void executeImpl() {
            if constexpr (C::value > 1) {
                for (size_t i = 0; i < C::value; i += 2) {
                    this->verify(Kerbecs::Shadow::shadowConstructAt<P>(&this->m_Shadow, i), "Construct failed");
                }
                
                for (size_t i = 0; i < C::value; i++) {
                    auto s = Kerbecs::Shadow::shadowGetMemoryState<P>(&this->m_Shadow, i);
                    if (i % 2 == 0) {
                        this->m_Validator.expect(s == Kerbecs::Shadow::Utils::MemoryState::CONSTRUCTED, "Even idx CONSTRUCTED target failed");
                    } else {
                        this->m_Validator.expect(s == Kerbecs::Shadow::Utils::MemoryState::UNINITIALIZED, "Odd idx UNINITIALIZED target failed");
                    }
                }
            }
        }

        void resetImpl(CpuAdapter&) {}

        void teardownImpl() {
            for (size_t i = 0; i < C::value; i += 2) {
                Kerbecs::Shadow::shadowDestroy<P>(&this->m_Shadow, i);
            }
            // Odd indices were never constructed - construct+destroy to drain liveCount to 0.
            for (size_t i = 1; i < C::value; i += 2) {
                Kerbecs::Shadow::shadowConstructAt<P>(&this->m_Shadow, i);
                Kerbecs::Shadow::shadowDestroy<P>(&this->m_Shadow, i);
            }
        }
    };

    template <typename L, typename P, typename C, typename A, typename CX>
    struct QuarantineFlowFixture : CombinatorialBase<QuarantineFlowFixture, L, P, C, A, CX> {
        using Base = CombinatorialBase<QuarantineFlowFixture, L, P, C, A, CX>;
        explicit QuarantineFlowFixture(CpuAdapter& ro_Adapter) : Base(ro_Adapter) {}

        void startupImpl() {
            this->dumpAndVerify(Kerbecs::Shadow::shadowAllocate<P>(&this->m_Shadow, 1), "Allocate failed");
            this->dumpAndVerify(Kerbecs::Shadow::shadowConstruct<P>(&this->m_Shadow), "Construct failed");
        }

        void executeImpl() {
            // Destroy triggers insertion to quarantine
            bool destroyOk = Kerbecs::Shadow::shadowDestroy<P>(&this->m_Shadow, 0);
            KERBECS_UNUSED(destroyOk);

            auto& zone = Kerbecs::MemoryZone::instance();

            // Advance Epoch by 1 -> node is in Quarantine Queue but not flushable
            zone.m_Epoch.fetch_add(1, std::memory_order_release);

            // Should still exist in registry as Quarantine due to grace period
            auto s1 = Kerbecs::Shadow::shadowGetMemoryState<P>(&this->m_Shadow, 0);
            this->m_Validator.expect(s1 == Kerbecs::Shadow::Utils::MemoryState::DESTROYED, "Should report DESTROYED whilst in quarantine");

            // Advance Epoch by 2 -> flushable
            zone.m_Epoch.fetch_add(2, std::memory_order_release);
            zone.m_Quarantine.flushEligible(zone.m_Registry.poolSegment());

            // Node stripped from registry -> gets CORRUPTED/WildPointer evaluation
            auto s2 = Kerbecs::Shadow::shadowGetMemoryState<P>(&this->m_Shadow, 0);
            this->m_Validator.expect(s2 == Kerbecs::Shadow::Utils::MemoryState::CORRUPTED, "Should report CORRUPTED once freed completely");
        }

        void resetImpl(CpuAdapter&) {
            // Hades calls execute twice (calibration + measurement). Between calls,
            // flush quarantine and re-allocate so executeImpl starts from a clean state.
            auto& zone = Kerbecs::MemoryZone::instance();
            zone.m_Epoch.fetch_add(4, std::memory_order_acq_rel);
            zone.m_Quarantine.flushEligible(zone.m_Registry.poolSegment());
            Kerbecs::Shadow::shadowAllocate<P>(&this->m_Shadow, 1);
            Kerbecs::Shadow::shadowConstruct<P>(&this->m_Shadow);
        }

        void teardownImpl() {}
    };

    // Meta-lists
    using Layouts     = std::tuple<Kerbecs::Layout::NormalLayout, Kerbecs::Layout::EnhancedLayout, Kerbecs::Layout::StaticLayout>;
    using Payloads    = std::tuple<Payload1B, Payload4B, Vector4, Matrix4x4, L2CacheBlock>;
    using Counts      = std::tuple<CountSingle, CountSmall, CountLarge, CountPage>;
    using Alignments  = std::tuple<AlignDefault, AlignAVX2, AlignAVX512, AlignL2>;
    using Concurrency = std::tuple<ConcurrencySingle, ConcurrencyMulti>;

    template <template<typename, typename, typename, typename, typename> class FixtureTempl>
    struct Unroller {
        template <typename... Args>
        static void submit(Hades::Runtime::HadesEngine<CpuAdapter, Hades::Runtime::XorHashAccumulator, KerbecsStorage>& engine) {
            Hades::Runtime::Config cfg;
            cfg.m_ThreadCount = 1; cfg.m_Iterations = 1; cfg.m_MinSlices = 1; cfg.m_MaxSlices = 1;
            auto factory = Hades::Runtime::makeFactory<FixtureTempl<Args...>, CpuAdapter>();
            while (!engine.submit(factory, cfg)) {
                std::this_thread::yield();
            }
        }

        template <typename TTuple1, typename TTuple2, typename TTuple3, typename TTuple4, typename TTuple5>
        static void unrollAll(Hades::Runtime::HadesEngine<CpuAdapter, Hades::Runtime::XorHashAccumulator, KerbecsStorage>& engine, 
            std::index_sequence<>, TTuple2, TTuple3, TTuple4, TTuple5) {}

        template <typename TTuple1, size_t I, size_t... Is, typename TTuple2, typename TTuple3, typename TTuple4, typename TTuple5>
        static void unrollAll(Hades::Runtime::HadesEngine<CpuAdapter, Hades::Runtime::XorHashAccumulator, KerbecsStorage>& engine,
            std::index_sequence<I, Is...>, TTuple2 t2, TTuple3 t3, TTuple4 t4, TTuple5 t5) {
            
            using L = std::tuple_element_t<I, TTuple1>;
            unrollPayload<L>(engine, std::make_index_sequence<std::tuple_size_v<TTuple2>>{}, t3, t4, t5);
            unrollAll<TTuple1>(engine, std::index_sequence<Is...>{}, t2, t3, t4, t5);
        }

        template <typename L, size_t... Is, typename TTuple3, typename TTuple4, typename TTuple5>
        static void unrollPayload(Hades::Runtime::HadesEngine<CpuAdapter, Hades::Runtime::XorHashAccumulator, KerbecsStorage>& engine, 
            std::index_sequence<Is...>, TTuple3 t3, TTuple4 t4, TTuple5 t5) {
            (unrollCount<L, std::tuple_element_t<Is, Payloads>>(engine, std::make_index_sequence<std::tuple_size_v<TTuple3>>{}, t4, t5), ...);
        }

        template <typename L, typename P, size_t... Is, typename TTuple4, typename TTuple5>
        static void unrollCount(Hades::Runtime::HadesEngine<CpuAdapter, Hades::Runtime::XorHashAccumulator, KerbecsStorage>& engine, 
            std::index_sequence<Is...>, TTuple4 t4, TTuple5 t5) {
            (unrollAlign<L, P, std::tuple_element_t<Is, Counts>>(engine, std::make_index_sequence<std::tuple_size_v<TTuple4>>{}, t5), ...);
        }

        template <typename L, typename P, typename C, size_t... Is, typename TTuple5>
        static void unrollAlign(Hades::Runtime::HadesEngine<CpuAdapter, Hades::Runtime::XorHashAccumulator, KerbecsStorage>& engine, 
            std::index_sequence<Is...>, TTuple5 t5) {
            (unrollConc<L, P, C, std::tuple_element_t<Is, Alignments>>(engine, std::make_index_sequence<std::tuple_size_v<TTuple5>>{}), ...);
        }

        template <typename L, typename P, typename C, typename A, size_t... Is>
        static void unrollConc(Hades::Runtime::HadesEngine<CpuAdapter, Hades::Runtime::XorHashAccumulator, KerbecsStorage>& engine, 
            std::index_sequence<Is...>) {
            (submit<L, P, C, A, std::tuple_element_t<Is, Concurrency>>(engine), ...);
        }

        static void go(Hades::Runtime::HadesEngine<CpuAdapter, Hades::Runtime::XorHashAccumulator, KerbecsStorage>& engine) {
            unrollAll<Layouts>(engine, std::make_index_sequence<std::tuple_size_v<Layouts>>{}, Payloads{}, Counts{}, Alignments{}, Concurrency{});
        }
    };

    void submitCombinatorialTests(
        Hades::Runtime::HadesEngine<CpuAdapter, Hades::Runtime::XorHashAccumulator, KerbecsStorage>& ro_Engine,
        Spectra::Validation::Validator& ro_Validator,
        CpuAdapter&) {

        s_pValidator = &ro_Validator;

        // Start engine submissions mapping all permutations to Hades
        Unroller<LifecycleFixture>::go(ro_Engine);
        Unroller<PartialInitFixture>::go(ro_Engine);
        Unroller<QuarantineFlowFixture>::go(ro_Engine);
        
        // 4. Registry Range Check, 5. Guard Verification are intentionally short-circuited 
        // here to prevent exceeding translation unit complexity for now, but 
        // the core 3 test domains encompass ~1,440 configurations automatically.
    }

} // KerbecsValidation
