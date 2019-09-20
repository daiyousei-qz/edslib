#pragma once
#include "../lang-utils.h"
#include "../array-ref.h"
#include <type_traits>
#include <memory>
#include <cassert>

namespace eds::container
{
    // TODO: add initialization from a range of data
    template <typename T>
    class HeapArray : NonCopyable
    {
    public:
        static_assert(!std::is_const_v<T> || !std::is_volatile_v<T>, "T cannot be cv-qualified");

        // ctors
        //

        HeapArray() {}
        HeapArray(int len)
        {
            Initialize(len);
        }
        HeapArray(int len, const T& value)
        {
            Initialize(len, value);
        }

        HeapArray(HeapArray&& other)
        {
            this->swap(other);
        }
        HeapArray& operator=(HeapArray&& other)
        {
            Initialize(0);
            this->swap(other);
        }

        ~HeapArray()
        {
            Destroy();
        }

        // Members
        //

        // test if array is empty
        bool Empty() const noexcept { return size_ == 0; }

        // get volume of the array
        int Size() const noexcept { return size_; }

        T& Front() { return At(0); }
        const T& Front() const { return At(0); }

        // TODO: bound check?
        T& Back() { return At(size_ - 1); }
        const T& Back() const { return At(size_ - 1); }

        // access element at
        T& At(int index)
        {
            assert(index >= 0 && index < size_);
            return ptr_[index];
        }
        const T& At(int index) const
        {
            assert(index >= 0 && index < size_);
            return ptr_[index];
        }

        ArrayRef<T> Ref()
        {
            return ArrayRef<T>{prt_, size_};
        }
        ArrayView<T> View() const
        {
            return ArrayView<T>{ptr_, size_};
        }

        // discard the current instance and initialize a new one
        void Initialize(int len)
        {
            InitializeInternal(len, [&](T* p) { std::uninitialized_value_construct_n(p, len); });
        }
        void Initialize(int len, const T& value)
        {
            InitializeInternal(len, [&](T* p) { std::uninitialized_fill_n(p, len, value); });
        }

        // Language interfaces
        //
        T& operator[](int index) { return At(index); }
        const T& operator[](int index) const { return At(index); }

        T* begin() { return ptr_; }
        T* end() { return ptr_ + size_; }

        const T* begin() const noexcept { return ptr_; }
        const T* end() const noexcept { return ptr_ + size_; }

        void swap(HeapArray& other) noexcept
        {
            std::swap(size_, other.size_);
            std::swap(ptr_, other.ptr_);
        }

    private:
        void Destroy() noexcept
        {
            if (ptr_)
            {
                std::destroy_n(ptr_, size_);
            }

            free(ptr_);
            size_ = 0;
            ptr_  = nullptr;
        }

        template <typename FInit>
        void InitializeInternal(int len, FInit init)
        {
            assert(len >= 0);

            auto new_size = 0;
            T* new_ptr    = nullptr;

            if (len > 0)
            {
                auto tmp = reinterpret_cast<T*>(malloc(sizeof(T) * len));

                try
                {
                    init(tmp);
                    new_size = len;
                    new_ptr  = tmp;
                }
                catch (...)
                {
                    free(tmp);
                    throw;
                }
            }

            Destroy();
            size_ = new_size;
            ptr_  = new_ptr;
        }

        int size_ = 0;
        T* ptr_   = nullptr;
    };

    template <typename T>
    inline bool operator==(const HeapArray<T>& lhs, const HeapArray<T>& rhs)
    {
        return lhs.Size() == rhs.Size() &&
               std::equal(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
    }
    template <typename T>
    inline bool operator!=(const HeapArray<T>& lhs, const HeapArray<T>& rhs)
    {
        return !(lhs == rhs);
    }
    template <typename T>
    inline bool operator<(const HeapArray<T>& lhs, const HeapArray<T>& rhs)
    {
        return std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
    }
    template <typename T>
    inline bool operator>(const HeapArray<T>& lhs, const HeapArray<T>& rhs)
    {
        return rhs < lhs;
    }
    template <typename T>
    inline bool operator<=(const HeapArray<T>& lhs, const HeapArray<T>& rhs)
    {
        return !(lhs > rhs);
    }
    template <typename T>
    inline bool operator>=(const HeapArray<T>& lhs, const HeapArray<T>& rhs)
    {
        return !(lhs < rhs);
    }
}