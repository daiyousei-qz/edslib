#pragma once
#include <cstdint>

namespace eds::binary
{
    class DataBlobPtr
    {
    private:
        uint8_t* data_;
    };
}