#pragma once
#include <thread>

namespace Spectra::Benchmark {
	struct ExecutionConfig final {
		const float m_ContentionRatio;
		const size_t m_Threads;

		ExecutionConfig(float v_Contention = 0.5f, size_t v_Threads = std::thread::hardware_concurrency())
			: m_ContentionRatio(v_Contention), m_Threads(v_Threads) {}
	};

	struct BenchResults final {
		double m_StartupTime;
		double m_ShutdownTime;
		double m_ReadTime;
		double m_WriteTime;

		BenchResults( ): m_StartupTime(-1.f) , m_ShutdownTime(-1.f), m_ReadTime(-1.f), m_WriteTime(-1.f){}
	};

    template<typename T>
    class BenchQueue final {
        struct Node {
            T m_Value;
            Node* m_Next;
        };

    public:
        explicit BenchQueue(size_t v_ReserveCount = 128) noexcept
            : m_Capacity(v_ReserveCount) {
            // allocate raw buffer for all nodes
            m_Memory = static_cast<Node*>(
                ::operator new(sizeof(Node) * m_Capacity, std::align_val_t{ alignof(Node) }));
            // initialize free list
            m_FreeList = nullptr;
            for (size_t i = 0; i < m_Capacity; ++i) {
                Node* n = &m_Memory[i];
                n->next = m_FreeList;
                m_FreeList = n;
            }
        }

        ~BenchQueue() noexcept {
            clear();
            ::operator delete(m_Memory, std::align_val_t{ alignof(Node) });
            m_Memory = nullptr;
            m_Head = m_Tail = m_FreeList = nullptr;
            m_Size = 0;
        }

        BenchQueue(const BenchQueue&) = delete;
        BenchQueue& operator=(const BenchQueue&) = delete;
        BenchQueue(BenchQueue&&) = delete;
        BenchQueue& operator=(BenchQueue&&) = delete;

        // --- Core Emplace ---
        template<typename... Args>
        T& emplace(Args&&... u_Args) noexcept {
            assert(m_FreeList && "BenchQueue out of capacity");
            Node* node = m_FreeList;
            m_FreeList = node->next;

            new (&node->value) T(std::forward<Args>(u_Args)...);
            node->next = nullptr;

            if (!m_Head) {
                m_Head = m_Tail = node;
            } else {
                m_Tail->next = node;
                m_Tail = node;
            }

            ++m_Size;
            return node->value;
        }

        // --- Access / Iteration ---
        T* front() noexcept { return m_Head ? &m_Head->value : nullptr; }
        const T* front() const noexcept { return m_Head ? &m_Head->value : nullptr; }

        bool empty() const noexcept { return m_Size == 0; }
        size_t size() const noexcept { return m_Size; }

        struct Iterator {
            Node* ptr;
            Iterator& operator++() noexcept { ptr = ptr->next; return *this; }
            bool operator!=(const Iterator& rhs) const noexcept { return ptr != rhs.ptr; }
            T& operator*() noexcept { return ptr->value; }
        };

        Iterator begin() noexcept { return Iterator{ m_Head }; }
        Iterator end() noexcept { return Iterator{ nullptr }; }

        // --- Destruction / Cleanup ---
        void clear() noexcept {
            Node* n = m_Head;
            while (n) {
                Node* next = n->next;
                n->value.~T();
                // return to free list
                n->next = m_FreeList;
                m_FreeList = n;
                n = next;
            }
            m_Head = m_Tail = nullptr;
            m_Size = 0;
        }

    private:
        Node* m_Memory = nullptr;     // backing store
        Node* m_FreeList = nullptr;   // nodes available
        Node* m_Head = nullptr;       // first active node
        Node* m_Tail = nullptr;       // last active node
        size_t m_Capacity = 0;
        size_t m_Size = 0;
    };

}


