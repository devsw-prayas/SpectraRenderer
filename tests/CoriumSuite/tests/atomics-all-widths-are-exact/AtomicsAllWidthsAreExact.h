#pragma once
#include <Fixture.h>

#include <HadesAdapters.h>
#include <HadesChrono.h>
#include <HadesHash.h>

#include <AtomicValue.h>

#include <cstdio>
#include <cstdlib>
#include <limits>
#include <thread>
#include <type_traits>
#include <vector>

// Runs in every configuration: CORIUM_ASSERT is compiled out of Release, so failures are counted and abort() at the end.
namespace AtomicsCheck {
	using namespace Corium::Atomics;
	inline int g_fail = 0;
#define ATOMICS_CHECK(c) do { if (!(c)) { std::printf("ATOMICS CHECK FAILED line %d: %s\n", __LINE__, #c); ++g_fail; } } while (0)

// invalid orders must not compile (dependent concepts so the check is a real SFINAE context)
template<MemoryOrder O, typename P> concept CanStore = requires(P p) { store<O>(p, 1); };
template<MemoryOrder O, typename P> concept CanLoad = requires(P p) { load<O>(p); };
template<MemoryOrder S, MemoryOrder F, typename P> concept CanCas = requires(P p, std::remove_pointer_t<P>& e) { compareExchange<S, F>(p, e, 1); };
template<typename P> concept CanFetchAdd = requires(P p) { fetchAdd(p, 1); };
static_assert(!CanStore<MemoryOrder::ACQUIRE, uint32_t*>);
static_assert(!CanStore<MemoryOrder::ACQ_REL, uint32_t*>);
static_assert(CanStore<MemoryOrder::RELEASE, uint32_t*>);          // literal does not fight the pointer
static_assert(CanStore<MemoryOrder::SEQ_CST, uint8_t*>);
static_assert(!CanLoad<MemoryOrder::RELEASE, uint32_t*>);
static_assert(!CanLoad<MemoryOrder::ACQ_REL, uint32_t*>);
static_assert(CanLoad<MemoryOrder::ACQUIRE, uint16_t*>);
static_assert(!CanCas<MemoryOrder::SEQ_CST, MemoryOrder::RELEASE, uint32_t*>);
static_assert(CanCas<MemoryOrder::RELEASE, MemoryOrder::RELAXED, uint32_t*>);
static_assert(!CanFetchAdd<float*>);                               // no float arithmetic
static_assert(!CanFetchAdd<bool*>);
static_assert(CanFetchAdd<uint8_t*>);

template<typename T> void testType() {
	alignas(8) T x{};
	store(&x, T(5));                                   ATOMICS_CHECK(load(&x) == T(5));
	store<MemoryOrder::RELAXED>(&x, T(6));             ATOMICS_CHECK(load<MemoryOrder::RELAXED>(&x) == T(6));
	store<MemoryOrder::RELEASE>(&x, T(7));             ATOMICS_CHECK(load<MemoryOrder::ACQUIRE>(&x) == T(7));
	ATOMICS_CHECK(exchange(&x, T(9)) == T(7));                 ATOMICS_CHECK(load(&x) == T(9));
	T e = T(1); ATOMICS_CHECK(!compareExchange(&x, e, T(3)));  ATOMICS_CHECK(e == T(9) && load(&x) == T(9));
	ATOMICS_CHECK(compareExchange(&x, e, T(3)));               ATOMICS_CHECK(load(&x) == T(3));
	ATOMICS_CHECK(fetchAdd(&x, T(4)) == T(3));                 ATOMICS_CHECK(load(&x) == T(7));
	ATOMICS_CHECK(fetchSub(&x, T(2)) == T(7));                 ATOMICS_CHECK(load(&x) == T(5));
	store(&x, T(0b1100));
	ATOMICS_CHECK(fetchAnd(&x, T(0b0110)) == T(0b1100));       ATOMICS_CHECK(load(&x) == T(0b0100));
	ATOMICS_CHECK(fetchOr(&x, T(0b0011)) == T(0b0100));        ATOMICS_CHECK(load(&x) == T(0b0111));
	ATOMICS_CHECK(fetchXor(&x, T(0b0101)) == T(0b0111));       ATOMICS_CHECK(load(&x) == T(0b0010));
	store(&x, T(0b1100));
	ATOMICS_CHECK(fetchNand(&x, T(0b0110)) == T(0b1100));      ATOMICS_CHECK(load(&x) == static_cast<T>(~T(0b0100)));
	store(&x, T(10));
	ATOMICS_CHECK(fetchMax(&x, T(20)) == T(10));               ATOMICS_CHECK(load(&x) == T(20));
	ATOMICS_CHECK(fetchMax(&x, T(5)) == T(20));                ATOMICS_CHECK(load(&x) == T(20));
	ATOMICS_CHECK(fetchMin(&x, T(2)) == T(20));                ATOMICS_CHECK(load(&x) == T(2));
	ATOMICS_CHECK(fetchMin(&x, T(9)) == T(2));                 ATOMICS_CHECK(load(&x) == T(2));
	if constexpr (std::is_signed_v<T>) {               // signedness must matter
		store(&x, T(1)); (void)fetchMax(&x, T(-1));    ATOMICS_CHECK(load(&x) == T(1));
		(void)fetchMin(&x, T(-1));                     ATOMICS_CHECK(load(&x) == T(-1));
	} else {
		store(&x, T(1)); (void)fetchMax(&x, std::numeric_limits<T>::max()); ATOMICS_CHECK(load(&x) == std::numeric_limits<T>::max());
	}
	store(&x, T(5));
	ATOMICS_CHECK(increment(&x) == T(6));                      ATOMICS_CHECK(decrement(&x) == T(5));
	store(&x, T(0));
	ATOMICS_CHECK(!bitTestAndSet(&x, 2));                      ATOMICS_CHECK(bitTestAndSet(&x, 2));  ATOMICS_CHECK(load(&x) == T(4));
	ATOMICS_CHECK(bitTestAndReset(&x, 2));                     ATOMICS_CHECK(!bitTestAndReset(&x, 2)); ATOMICS_CHECK(load(&x) == T(0));
	store(&x, std::numeric_limits<T>::max());          // wrap
	ATOMICS_CHECK(fetchAdd(&x, T(1)) == std::numeric_limits<T>::max()); ATOMICS_CHECK(load(&x) == std::numeric_limits<T>::min());
}

// An op on N bytes must leave every neighbouring byte untouched.
template<typename T> void testNeighbours() {
	alignas(8) uint8_t buf[24];
	for (auto& b : buf) b = 0xA5;
	T* p = reinterpret_cast<T*>(buf + 8);
	store(p, T(0));
	(void)fetchAdd(p, T(1)); (void)fetchOr(p, T(2)); (void)fetchXor(p, T(1)); (void)exchange(p, T(7));
	T e = T(7); (void)compareExchange(p, e, T(3)); (void)fetchNand(p, T(1)); (void)fetchMax(p, T(1));
	(void)increment(p); (void)decrement(p); (void)bitTestAndSet(p, 0); (void)bitTestAndReset(p, 0);
	store(p, std::numeric_limits<T>::max()); (void)fetchAdd(p, T(1));
	for (size_t i = 0; i < 24; ++i) if (i < 8 || i >= 8 + sizeof(T)) ATOMICS_CHECK(buf[i] == 0xA5);
}

template<typename T> void testContention() {
	AtomicValue<T> v(T(0));
	constexpr int N = 100000, THREADS = 4;
	std::vector<std::thread> ts;
	for (int t = 0; t < THREADS; ++t) ts.emplace_back([&] { for (int i = 0; i < N; ++i) (void)v.fetchAdd(T(1)); });
	for (auto& t : ts) t.join();
	ATOMICS_CHECK(v.load() == static_cast<T>(static_cast<uint64_t>(N) * THREADS));
	// CAS-loop increments must also be exact
	AtomicValue<T> c(T(0));
	ts.clear();
	for (int t = 0; t < THREADS; ++t) ts.emplace_back([&] { for (int i = 0; i < N; ++i) { T e = c.load(); while (!c.compareExchange(e, static_cast<T>(e + 1))) {} } });
	for (auto& t : ts) t.join();
	ATOMICS_CHECK(c.load() == static_cast<T>(static_cast<uint64_t>(N) * THREADS));
}


	inline int runAll() {

	testType<uint8_t>();  testType<int8_t>();  testType<uint16_t>(); testType<int16_t>();
	testType<uint32_t>(); testType<int32_t>(); testType<uint64_t>(); testType<int64_t>();
	testNeighbours<uint8_t>(); testNeighbours<uint16_t>(); testNeighbours<uint32_t>(); testNeighbours<uint64_t>();
	testContention<uint8_t>(); testContention<uint16_t>(); testContention<uint32_t>(); testContention<uint64_t>();

	// bool / pointer / float words
	AtomicValue<bool> flag(false); flag.store(true); ATOMICS_CHECK(flag.load()); ATOMICS_CHECK(flag.exchange(false)); ATOMICS_CHECK(!flag.load());
	static_assert(sizeof(AtomicValue<bool>) == 1);
	int target = 0; AtomicValue<int*> ptr(nullptr); int* ex = nullptr; ATOMICS_CHECK(ptr.compareExchange(ex, &target)); ATOMICS_CHECK(ptr.load() == &target);
	AtomicValue<float> f(1.5f); ATOMICS_CHECK(f.exchange(2.5f) == 1.5f); ATOMICS_CHECK(f.load() == 2.5f);

		return g_fail;
	}
#undef ATOMICS_CHECK
}

class AtomicsAllWidthsAreExact final
	: public Hades::Runtime::IFixture<AtomicsAllWidthsAreExact, Hades::Runtime::NullDeviceAdapter> {
public:
	explicit AtomicsAllWidthsAreExact(Hades::Runtime::NullDeviceAdapter& ro_Adapter) noexcept
		: IFixture(ro_Adapter) {
	}

	void startupImpl() noexcept {
	}

	void executeImpl() noexcept {
		if (AtomicsCheck::runAll() != 0) std::abort();
	}

	void resetImpl(Hades::Runtime::NullDeviceAdapter& ro_Adapter) noexcept {
		(void)ro_Adapter;
	}

	void teardownImpl() noexcept {
	}

	uint64_t getDeterminismHashImpl() noexcept {
		return 0;
	}
};

