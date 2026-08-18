#pragma once
#include "SpectraMemory.h"
#include "SpecMemCompiler.h"
#include "SpecMemDiagnostics.h"
#include <type_traits>
#include <utility>

namespace Spectra::Memory::Utils {
	template<typename R, typename... Args>
	struct Trampoline {
		template<typename L>
		static R invoke(void* ctx, Args... args) {
			return (*static_cast<L*>(ctx))(std::forward<Args>(args)...);
		}
	};

	template<typename A, typename S>
	struct ClosureFunction;

	template<typename A, typename R, typename... Args>
	struct ClosureFunction<A, R(Args...)> {
		using AllocatorType = A;
		using EntryType = R(*)(void*, Args...);
		using DeleterType = void(*)(void*, AllocatorType*);

		void* m_Context = nullptr;
		EntryType m_Entry = nullptr;

		AllocatorType* m_Allocator = nullptr;
		DeleterType m_Deleter = nullptr;

		ClosureFunction() = default;
		ClosureFunction(const ClosureFunction&) = delete;
		ClosureFunction& operator=(const ClosureFunction&) = delete;

		ClosureFunction(ClosureFunction&& u_Other) noexcept
			: m_Context(u_Other.m_Context),
			m_Entry(u_Other.m_Entry),
			m_Allocator(u_Other.m_Allocator),
			m_Deleter(u_Other.m_Deleter) {
			u_Other.m_Context = nullptr;
			u_Other.m_Entry = nullptr;
			u_Other.m_Allocator = nullptr;
			u_Other.m_Deleter = nullptr;
		}

		ClosureFunction& operator=(ClosureFunction&& u_Other) noexcept {
			if (this != &u_Other) {
				if (m_Context && m_Deleter) {
					m_Deleter(m_Context, m_Allocator);
				}

				m_Context = u_Other.m_Context;
				m_Entry = u_Other.m_Entry;
				m_Allocator = u_Other.m_Allocator;
				m_Deleter = u_Other.m_Deleter;

				u_Other.m_Context = nullptr;
				u_Other.m_Entry = nullptr;
				u_Other.m_Allocator = nullptr;
				u_Other.m_Deleter = nullptr;
			}
			return *this;
		}

		template<typename L>
			requires (!std::is_same_v<std::remove_cvref_t<L>, ClosureFunction>)
		explicit ClosureFunction(L&& lambda, AllocatorType* allocator) {
			if (!allocator) {
				m_Context = nullptr;
				m_Entry = nullptr;
				return;
			}
			using LambdaT = std::decay_t<L>;

			LambdaT* stored = allocator->template emplace<LambdaT>(
				std::forward<L>(lambda)
			);

			m_Context = stored;
			m_Allocator = allocator;

			m_Entry = &Trampoline<R, Args...>::template invoke<LambdaT>;

			m_Deleter = [](void* ctx, AllocatorType* alloc) {
				auto* obj = static_cast<LambdaT*>(ctx);

				obj->~LambdaT();
				alloc->deallocate(obj, sizeof(LambdaT));
				};
		}

		~ClosureFunction() {
			if (m_Context && m_Deleter) {
				m_Deleter(m_Context, m_Allocator);
			}
		}

		SPEC_MEM_NODISCARD R operator()(Args... args) const {
			SPEC_MEM_ASSERT(m_Entry != nullptr);
			return m_Entry(m_Context, std::forward<Args>(args)...);
		}

		bool isCallable() const noexcept {
			return m_Context != nullptr && m_Entry != nullptr;
		}
	};

	template<typename>
	struct FunctionView;

	template<typename R, typename... Args>
	struct FunctionView<R(Args...)> {
		using Entry = R(*)(void*, Args...);

		void* m_Context = nullptr;
		Entry m_Entry = nullptr;

		template<typename L>
		FunctionView(L& lambda) noexcept {
			using LambdaT = std::remove_reference_t<L>;

			m_Context = &lambda;
			m_Entry = &Trampoline<R, Args...>::template invoke<LambdaT>;
		}

		// Also allow a view from an existing ClosureFunction.
		template<typename A>
		explicit FunctionView(const ClosureFunction<A, R(Args...)>& fn) {
			m_Context = fn.m_Context;
			m_Entry = fn.m_Entry;
		}

		SPEC_MEM_NODISCARD R operator()(Args... args) const {
			SPEC_MEM_ASSERT(m_Entry != nullptr);
			return m_Entry(m_Context, std::forward<Args>(args)...);
		}
	};
}
