/*=================================================================================
*  Copyright (c) 2016 Edward Cheng
*
*  edslib is an open-source library in C++ and licensed under the MIT License.
*  Refer to: https://opensource.org/licenses/MIT
*================================================================================*/

#pragma once
#include "../lang-utils.h"
#include <deque>
#include <cassert>
#include <algorithm>
#include <type_traits>
#include <memory>
#include <cstdint>

namespace eds
{
	constexpr size_t kDefaultWorkspaceSize = 4096;

    template <size_t BufferSize = kDefaultWorkspaceSize>
    class StackWorkspaceMemoryProvider final : NonCopyable, NonMovable
    {
    public:
        StackWorkspaceMemoryProvider() {}
        StackWorkspaceMemoryProvider(size_t sz) {}

        void* Allocate(size_t sz) noexcept
        {
            if (offset_ + sz > BufferSize)
            {
                return nullptr;
            }

            void* result = &buffer_[offset_];
            offset_ += sz;

            return result;
        }

        void Clear() noexcept
        {
            offset_ = 0;
        }

        size_t GetByteAllocated() const noexcept
        {
            return BufferSize;
        }

        size_t GetByteUsed() const noexcept
        {
            return offset_;
        }

    private:
        uint8_t buffer_[BufferSize];

        size_t offset_ = 0;
    };

    class HeapWorkspaceMemoryProvider final : NonCopyable, NonMovable
    {
    public:
        HeapWorkspaceMemoryProvider() : HeapWorkspaceMemoryProvider(kDefaultWorkspaceSize) {}
        HeapWorkspaceMemoryProvider(size_t sz)
        {
            buffer_size_ = sz;
            buffer_      = std::make_unique<uint8_t[]>(sz);

            offset_ = 0;
        }

        void* Allocate(size_t sz) noexcept
        {
            if (offset_ + sz > buffer_size_)
            {
                return nullptr;
            }

            void* result = &buffer_[offset_];
            offset_ += sz;

            return result;
        }

        void Clear() noexcept
        {
            offset_ = 0;
        }

        size_t GetByteAllocated() const noexcept
        {
            return buffer_size_;
        }

        size_t GetByteUsed() const noexcept
        {
            return offset_;
        }

    private:
        size_t buffer_size_;
        std::unique_ptr<uint8_t[]> buffer_;

        size_t offset_;
    };

    class HeapGrowableMemoryProvider final : NonCopyable, NonMovable
    {
    public:
        HeapGrowableMemoryProvider() {}
        HeapGrowableMemoryProvider(size_t sz)
            : next_block_sz_(sz) {}

        ~HeapGrowableMemoryProvider()
        {
            Clear();
        }

        void* Allocate(size_t sz)
        {
            if (sz > kBigChunkThreshold)
            {
                // big chunk of memory should be allocated directly from allocater
                return AllocBigChunk(sz);
            }
            else
            {
                // small chunk should be allocated from the internal pool
                return AllocSmallChunk(sz);
            }
        }

        void Clear() noexcept
        {
            FreeBlocks(pooled_head_);
            FreeBlocks(big_node_);

            pooled_head_    = nullptr;
            pooled_current_ = nullptr;
            big_node_       = nullptr;
        }

        size_t GetByteAllocated() const noexcept
        {
            return CalculateUsage(pooled_head_, false) + CalculateUsage(big_node_, false);
        }

        size_t GetByteUsed() const noexcept
        {
            return CalculateUsage(pooled_head_, true) + CalculateUsage(big_node_, true);
        }

    private:
        struct Block
        {
            Block* next;
            // size of the block
            size_t size;
            // avalible space offset
            size_t offset;
            // counter of allocation failure on this block
            size_t counter;

            uint8_t* DataAddress()
            {
                return reinterpret_cast<uint8_t*>(this + 1);
            }
        };

        static constexpr size_t kFailureToleranceCount   = 8;
        static constexpr size_t kFailureCounterThreshold = 1024;
        static constexpr size_t kBigChunkThreshold       = 2048;
        static constexpr size_t kDefaultPoolBlockSize    = 4096 - sizeof(Block);
        static constexpr size_t kMaximumPoolBlockSize    = 16 * 4096 - sizeof(Block);
        static constexpr float kPoolBlockGrowthFactor    = 2;

        Block* NewBlock(size_t capacity)
        {
            // allocate memory
            void* p      = malloc(sizeof(Block) + capacity);
            Block* block = reinterpret_cast<Block*>(p);

            // initialize block
            block->next    = nullptr;
            block->size    = capacity;
            block->offset  = 0;
            block->counter = 0;

            return block;
        }

        Block* NewPoolBlock()
        {
            auto size  = next_block_sz_;
            auto block = NewBlock(size);
            next_block_sz_ =
                std::min(static_cast<size_t>(kPoolBlockGrowthFactor * size), kMaximumPoolBlockSize);

            return block;
        }

        void FreeBlocks(Block* list) const
        {
            Block* p = list;
            while (p != nullptr)
            {
                auto next = p->next;

                free(p);
                p = next;
            }
        }

        size_t CalculateUsage(Block* list, bool used) const
        {
            size_t sum = 0;
            for (Block* p = list; p != nullptr; p = p->next)
            {
                if (used)
                {
                    sum += p->offset;
                }
                else
                {
                    sum += p->size;
                }
            }

            return sum;
        }

        void* AllocSmallChunk(size_t sz)
        {
            // lazy initialization
            if (pooled_current_ == nullptr)
            {
                pooled_head_ = pooled_current_ = NewPoolBlock();
            }

            // find a chunk of memory in the memory pool
            Block* cur = pooled_current_;
            while (true)
            {
                size_t available_sz = cur->size - cur->offset;
                if (available_sz >= sz)
                {
                    // enough memory in the current Block
                    void* addr = cur->DataAddress() + cur->offset;
                    cur->offset += sz;
                    return addr;
                }
                else
                {
                    if (available_sz < kFailureCounterThreshold)
                    {
                        // failed to allocate and available size is small enough
                        cur->counter += 1;
                    }

                    // roll to next Block, allocate one if neccessary
                    Block* next = cur->next != nullptr ? cur->next : (cur->next = NewPoolBlock());
                    // if the current Block has beening failing for too many times, drop it
                    if (cur->counter > kFailureToleranceCount)
                    {
                        pooled_current_ = next;
                    }

                    cur = next;
                }
            }
        }

        void* AllocBigChunk(size_t sz)
        {
            // note this works even if big_node_ == nullptr
            Block* cur = NewBlock(sz);
            cur->next  = big_node_;
            big_node_  = cur;

            return cur->DataAddress();
        }

    private:
        size_t next_block_sz_ = kDefaultPoolBlockSize;

        // blocks for memory pool
        Block* pooled_head_    = nullptr;
        Block* pooled_current_ = nullptr;
        // blocks for big chunk allocation
        Block* big_node_ = nullptr;
    };

    template <typename MemoryProvider, size_t AlignmentSize = alignof(std::max_align_t)>
    class BasicArena final : MemoryProvider, NonCopyable, NonMovable
    {
    public:
        static_assert(AlignmentSize >= alignof(std::max_align_t));

        BasicArena() {}
		~BasicArena()
		{
			Clear();
		}

		using Ptr = std::unique_ptr<BasicArena>;
		using SharedPtr = std::shared_ptr<BasicArena>;

        void* Allocate(size_t sz) noexcept
        {
            return MemoryProvider::Allocate(RoundToAlign(sz, AlignmentSize));
        }

        template <typename T, typename... TArgs>
        T* Construct(TArgs&&... args)
        {
            static_assert(alignof(T) <= AlignmentSize);

            constexpr auto alloc_size = CalcAllocSize<T>();
            auto ptr                  = MemoryProvider::Allocate(alloc_size);
            if (ptr == nullptr)
            {
                return nullptr;
            }

            if constexpr (std::is_trivially_destructible_v<T>)
            {
                new (ptr) T(std::forward<TArgs>(args)...);

                return reinterpret_cast<T*>(ptr);
            }
            else
            {
                auto handle_ptr = reinterpret_cast<DestructionHandle*>(ptr);
                auto object_ptr = GetObjectPtr(handle_ptr);

                new (object_ptr) T(std::forward<TArgs>(args)...);

                handle_ptr->next     = head_;
                handle_ptr->destruct = [](void* p) { reinterpret_cast<T*>(p)->~T(); };
                head_                = handle_ptr;

                return reinterpret_cast<T*>(object_ptr);
            }
        }

        void Clear()
        {
            for (auto handle = head_; handle->next != nullptr; handle = handle->next)
            {
                handle->destruct(GetObjectPtr(handle));
            }

            head_ = GetPivotHandle();
            MemoryProvider::Clear();
        }

        const MemoryProvider& GetProvider() const noexcept
        {
            return *this;
        }

    private:
        struct DestructionHandle
        {
            DestructionHandle* next;
            void (*destruct)(void*);
        };

        static DestructionHandle* GetPivotHandle() noexcept
        {
            static DestructionHandle handle{nullptr, [](void*) {}};

            return &handle;
        }

        static void* GetObjectPtr(DestructionHandle* handle) noexcept
        {
            return AdvancePtr(handle, RoundToAlign(sizeof(DestructionHandle), AlignmentSize));
        }

        template <typename T>
        constexpr static size_t CalcAllocSize() noexcept
        {
            constexpr auto aligned_size = RoundToAlign(sizeof(T), AlignmentSize);

            if constexpr (std::is_trivially_destructible_v<T>)
            {
                return aligned_size;
            }
            else
            {
                return aligned_size + RoundToAlign(sizeof(DestructionHandle), AlignmentSize);
            }
        }

        DestructionHandle* head_ = GetPivotHandle();
    };

    using Workspace = BasicArena<StackWorkspaceMemoryProvider<>>;
    using Arena     = BasicArena<HeapGrowableMemoryProvider>;

} // namespace eds