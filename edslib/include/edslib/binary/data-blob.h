#pragma once
#include <cstdint>
#include <cassert>
#include <type_traits>

namespace eds::binary
{
    // An unsafe wrapper to randomly access a chunk of memory
    // NOTE size of the blob should be tracked by users and no bound check is performed
    class DataBlobPtr
    {
    public:
        DataBlobPtr() : data_(nullptr) {}
        DataBlobPtr(void* ptr) : data_(reinterpret_cast<uint8_t*>(ptr)) {}

        template <typename T>
        T& Access(int offset)
        {
            static_assert(std::is_pod_v<T>);
            assert(offset >= 0);

            return *reinterpret_cast<T*>(data_ + offset);
        }

        template <typename T>
        const T& Access(int offset) const
        {
            static_assert(std::is_pod_v<T>);
            assert(offset >= 0);

            return *reinterpret_cast<const T*>(data_ + offset);
        }

    private:
        uint8_t* data_;
    };
}