#pragma once
#include <type_traits>
#include <cassert>

namespace eds
{
	namespace detail
	{
		// This class could be further optimized with empty base class elimination
		// to avoid store some constant length, but I won't do that.
		template<typename T, bool AllowMutation>
		class BasicArrayRef
		{
		public:
			using ValueType = std::remove_cv_t<T>;
			using StoreType = std::conditional_t<AllowMutation, ValueType, const ValueType>;
			using PointerType = StoreType*;
			using ReferenceType = StoreType&;

			// ctors
			//
			constexpr BasicArrayRef(PointerType ptr, int cnt) noexcept
				: data_(ptr), size_(cnt) { }

			constexpr BasicArrayRef(PointerType begin, PointerType end) noexcept
				: BasicArrayRef(begin, std::distance(begin, end)) { }

			// todo
			template<int N>
			BasicArrayRef(StoreType (&arr)[N]) noexcept
				: BasicArrayRef(arr, N) { }

			constexpr BasicArrayRef() noexcept
				: BasicArrayRef(nullptr, 0) { }

			constexpr BasicArrayRef(const BasicArrayRef& ins) noexcept
				: BasicArrayRef(ins.data_, ins.size_) { }

			constexpr BasicArrayRef(BasicArrayRef&& ins) noexcept
				: BasicArrayRef()
			{
				Swap(*this, ins);
			}
			
			BasicArrayRef& operator=(const BasicArrayRef& ins) noexcept
			{
				data_ = ins.data_;
				size_ = ins.size_;
			}
			BasicArrayRef& operator=(BasicArrayRef&& ins) noexcept
			{
				*this = static_cast<const BasicArrayRef&>(ins);
				ins.data_ = nullptr;
				ins.size_ = 0;
			}

			// members
			//
			PointerType Data() const noexcept { return data_; }
			int Length() const noexcept { return size_; }
			bool Empty() const noexcept { return size_ == 0; }

			PointerType BeginPtr() const noexcept { return data_; }
			PointerType EndPtr() const noexcept { return data_ + size_; }

			ReferenceType Front() const
			{
				assert(size_ > 0);
				return data_[0];
			}
			ReferenceType Back() const
			{
				assert(size_ > 0);
				return data_[size_ - 1];
			}
			ReferenceType At(int index) const
			{
				assert(index >= 0 && index < size_);
				return data_[index];
			}
			ReferenceType operator[](int index) const
			{
				return lhs.At(index);
			}

			constexpr BasicArrayRef Slice(int offset, int len) const
			{
				assert(offset >= 0 && len > 0);
				assert(offset + len <= size_);
				return BasicArrayRef{ data_ + offset, len };
			}
			constexpr BasicArrayRef TakeFront(int count) const
			{
				return Slice(0, count);
			}
			constexpr BasicArrayRef TakeBack(int count) const
			{
				return Slice(size_ - count, count);
			}
			constexpr BasicArrayRef DropFront(int count) const
			{
				return Slice(count, size_ - count);
			}
			constexpr BasicArrayRef DropBack(int count) const
			{
				return Slice(count, size_ - count);
			}

		private:
			PointerType data_;
			int size_;
		};

		template<typename T, bool AllowMutation,
				  typename ParamType = const BasicArrayRef<T, AllowMutation>&>
		inline bool operator==(ParamType lhs, ParamType rhs)
		{
			return std::equal(lhs.BeginPtr(), lhs.EndPtr(), rhs.BeginPtr(), rhs.EndPtr());
		}
		template<typename T, bool AllowMutation,
				  typename ParamType = const BasicArrayRef<T, AllowMutation>&>
		inline bool operator!=(ParamType lhs, ParamType rhs)
		{
			return !(lhs == rhs);
		}
		template<typename T, bool AllowMutation,
				  typename ParamType = const BasicArrayRef<T, AllowMutation>&>
		inline bool operator<(ParamType lhs, ParamType rhs)
		{
			return std::lexicographical_compare(lhs.BeginPtr(), lhs.EndPtr(), rhs.BeginPtr(), rhs.EndPtr());
		}
		template<typename T, bool AllowMutation,
			typename ParamType = const BasicArrayRef<T, AllowMutation>&>
			inline bool operator<=(ParamType lhs, ParamType rhs)
		{
			return !(lhs > rhs);
		}
		template<typename T, bool AllowMutation,
				  typename ParamType = const BasicArrayRef<T, AllowMutation>&>
		inline bool operator>(ParamType lhs, ParamType rhs)
		{
			return rhs < lhs;
		}
		template<typename T, bool AllowMutation,
				  typename ParamType = const BasicArrayRef<T, AllowMutation>&>
		inline bool operator>=(ParamType lhs, ParamType rhs)
		{
			return !(lhs < rhs);
		}
	}

	// an immutable view to some slice of array
	template<typename T>
	using ArrayView = detail::BasicArrayRef<T, false>;

	// a mutable reference to some slice of array
	template<typename T>
	using ArrayRef = detail::BasicArrayRef<T, true>;
}