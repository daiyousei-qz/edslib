/*=================================================================================
*  Copyright (c) 2016 Edward Cheng
*
*  edslib is an open-source library in C++ and licensed under the MIT License.
*  Refer to: https://opensource.org/licenses/MIT
*================================================================================*/

#pragma once
#include "lang-utils.hpp"
#include <deque>
#include <cassert>
#include <type_traits>
#include <memory>

namespace eds
{
	class Arena final : NonCopyable, NonMovable
	{
	private:
		struct Block
		{
			Block *next;
			// size of the block
			size_t size;
			// avalible space offset
			size_t offset;
			// counter of allocation failure on this block
			size_t counter;

			char* DataAddress()
			{
				return reinterpret_cast<char*>(this + 1);
			}
		};

		struct DestructionHandle
		{
			void* pointer;
			void(*cleaner)(void*);
		};

		struct CtorDummy { };

		static constexpr size_t kDefaultAlignment = alignof(nullptr_t);
		static constexpr size_t kFailureToleranceCount = 8;
		static constexpr size_t kFailureCounterThreshold = 1024;
		static constexpr size_t kBigChunkThreshold = 2048;
		static constexpr size_t kDefaultPoolBlockSize = 4096 - sizeof(Block);
		static constexpr float kPoolBlockGrowthFactor = 1.5;

	public:
		Arena(CtorDummy = {}) { }

		~Arena()
		{
			for (auto handle : dtors_)
			{
				handle.cleaner(handle.pointer);
			}

			FreeBlocks(pooled_head_);
			FreeBlocks(big_node_);
		}
		
		using Ptr = std::unique_ptr<Arena>;
		using SharedPtr = std::shared_ptr<Arena>;

		static Arena::Ptr Create()
		{
			return std::make_unique<Arena>();
		}

		static Arena::SharedPtr CreateShared()
		{
			return std::make_shared<Arena>();
		}

		void* Allocate(size_t sz)
		{
			// ensure aligned
			if (sz % kDefaultAlignment)
			{
				sz += kDefaultAlignment - sz % kDefaultAlignment;
			}

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

		template <typename T, typename ...TArgs>
		T* Construct(TArgs&& ...args)
		{
			auto ptr =Allocate(sizeof(T));
			new (ptr) T(std::forward<TArgs>(args)...);

			if constexpr(!std::is_trivially_destructible_v<T>)
			{
				dtors_.push_back({ ptr, [](void* p) { reinterpret_cast<T*>(p)->~T(); } });
			}

			return  reinterpret_cast<T*>(ptr);
		}

		size_t GetByteAllocated() const
		{
			return CalculateUsage(pooled_head_, false) + CalculateUsage(big_node_, false);
		}

		size_t GetByteUsed() const
		{
			return CalculateUsage(pooled_head_, true) + CalculateUsage(big_node_, true);
		}

	private:

		Block* NewBlock(size_t capacity)
		{
			// allocate memory
			void* p = malloc(sizeof(Block) + capacity);
			Block* block = reinterpret_cast<Block*>(p);

			// initialize block
			block->next = nullptr;
			block->size = capacity;
			block->offset = 0;
			block->counter = 0;

			return block;
		}

		Block* NewPoolBlock()
		{
			auto block = NewBlock(next_block_sz_);
			next_block_sz_ *= kPoolBlockGrowthFactor;

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
			for (Block *p = list; p != nullptr; p = p->next)
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
					void *addr = cur->DataAddress() + cur->offset;
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
					Block *next = cur->next != nullptr
						? cur->next
						: (cur->next = NewPoolBlock());
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
			cur->next = big_node_;
			big_node_ = cur;

			return cur->DataAddress();
		}

	private:
		size_t next_block_sz_ = kDefaultPoolBlockSize;

		// blocks for memory pool
		Block* pooled_head_ = nullptr;
		Block* pooled_current_ = nullptr;
		// blocks for big chunk allocation
		Block* big_node_ = nullptr;

		std::deque<DestructionHandle> dtors_;
	};

}