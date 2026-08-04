// SpectraLauncher.cpp
// Kerbecs v0.2 smoke test.
// Covers: KERBECS_PERSISTENT / KERBECS_GLOBAL macros, Region::allocate/
// construct/destroy, ShadowedMemory<T> raw-pointer parity, shadowStateOf
// after destroy, and operator+ bounds checking (BufferOverflow, non-fatal).

#include "SpectraLauncher.h"

#include <cstdio>

#include "KerbecsStatic.h"

// =========================================================
// Minimal test harness
// =========================================================

static int g_Passed = 0;
static int g_Failed = 0;

#define TEST(name, expr)                                          \
    do {                                                          \
        if (expr) {                                               \
            printf("  [PASS] %s\n", name);                       \
            ++g_Passed;                                           \
        } else {                                                  \
            printf("  [FAIL] %s  (line %d)\n", name, __LINE__);  \
            ++g_Failed;                                           \
        }                                                         \
    } while (0)

static void section(const char* name) {
	printf("\n=== %s ===\n", name);
}

// =========================================================
// KERBECS_PERSISTENT / KERBECS_GLOBAL - constructed during static init,
// before main() runs.
// =========================================================

KERBECS_PERSISTENT(int, g_PersistentInt, 42);
KERBECS_GLOBAL(double, g_GlobalDouble, 3.14);

struct TrackedObject {
	int   m_Value;
	bool* m_Destroyed;
	TrackedObject(int v_Value, bool* p_Destroyed) : m_Value(v_Value), m_Destroyed(p_Destroyed) {}
	~TrackedObject() { if (m_Destroyed) *m_Destroyed = true; }
};

static void testStaticMacros() {
	section("KERBECS_PERSISTENT / KERBECS_GLOBAL");

	TEST("KERBECS_PERSISTENT handle non-null", static_cast<int*>(g_PersistentInt) != nullptr);
	TEST("KERBECS_PERSISTENT value constructed", *g_PersistentInt == 42);

	TEST("KERBECS_GLOBAL handle non-null", static_cast<double*>(g_GlobalDouble) != nullptr);
	TEST("KERBECS_GLOBAL value constructed", *g_GlobalDouble == 3.14);

	// Mutate through the handle - raw-pointer parity (operator*).
	*g_PersistentInt = 100;
	TEST("KERBECS_PERSISTENT mutation via operator*", *g_PersistentInt == 100);
}

// =========================================================
// Region::allocate / construct / destroy - exercised directly against the
// same shared globalRegion() the KERBECS_GLOBAL macro uses.
// =========================================================

static void testRegionLifecycle() {
	section("Region::allocate / construct / destroy");

	using namespace Kerbecs;
	auto& region = StaticSupport::globalRegion();

	// Scalar allocate -> construct -> destroy -> shadowStateOf.
	{
		ShadowedMemory<int> handle = region.allocate<int>();
		TEST("allocate<int>: non-null", static_cast<int*>(handle) != nullptr);

		bool constructed = region.construct(handle, 7);
		TEST("construct<int>: succeeds", constructed);
		TEST("construct<int>: value visible through handle", *handle == 7);

		bool destroyed = region.destroy(handle);
		TEST("destroy<int>: succeeds", destroyed);

		auto state = shadowStateOf(handle, sizeof(int));
		TEST("shadowStateOf after destroy: DESTROYED", state == Shadow::Utils::MemoryState::DESTROYED);
	}

	// Destructor actually runs on Region::destroy.
	{
		bool destroyedFlag = false;
		ShadowedMemory<TrackedObject> handle = region.allocate<TrackedObject>();
		TEST("allocate<TrackedObject>: non-null", static_cast<TrackedObject*>(handle) != nullptr);

		region.construct(handle, 9, &destroyedFlag);
		TEST("construct<TrackedObject>: value visible", handle->m_Value == 9);
		TEST("construct<TrackedObject>: dtor not yet run", !destroyedFlag);

		region.destroy(handle);
		TEST("destroy<TrackedObject>: dtor ran", destroyedFlag);
	}

	// operator+ bounds checking - BufferOverflow is non-fatal, returns a
	// null handle and bumps KerbecsStats::m_TotalViolations instead of
	// trapping (only ShadowedMemory<T>::_check()'s generation mismatch on
	// dereference traps).
	{
		ShadowedMemory<int> handle = region.allocate<int>();
		region.construct(handle, 55);

		ShadowedMemory<int> same = handle + 0;
		TEST("operator+0: in bounds", static_cast<int*>(same) != nullptr);
		TEST("operator+0: value", *same == 55);

		auto& stats = Runtime::stats();
		size_t violationsBefore = stats.m_TotalViolations.load();
		ShadowedMemory<int> outOfBounds = handle + 1; // one past a single-int allocation
		TEST("operator+1: out of bounds returns null handle", static_cast<int*>(outOfBounds) == nullptr);
		TEST("operator+1: violation counted", stats.m_TotalViolations.load() > violationsBefore);

		region.destroy(handle);
	}
}

// =========================================================
// Entry Point
// =========================================================

int main() {
	printf("Kerbecs v0.2 - Smoke Test\n");
	printf("=========================\n");

	testStaticMacros();
	testRegionLifecycle();

	printf("\n=========================\n");
	printf("Results: %d passed, %d failed\n", g_Passed, g_Failed);

	return (g_Failed == 0) ? 0 : 1;
}
