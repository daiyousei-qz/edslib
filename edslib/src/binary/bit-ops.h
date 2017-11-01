#pragma once
#include "../ext/type-utils.h"
#include <cstddef>
#include <cassert>
#include <algorithm>
#include <vector>

// Bit operations
namespace eds
{
	template<typename TIter>
	class BitReader
	{
		static_assert(type::Constraint<TIter>(type::is_iterator_of<uint8_t>), "TIter must be an iterator type of uint8_t");

	public:
		BitReader(TIter begin, TIter end)
			: begin_(begin), end_(end), cursor_(begin) { }

		bool Exhausted() const { return cursor_ == end_; }

		int Offset() const { return offset_; }
		int RemainingSize() const { return std::distance(cursor_, end_) * 8 - offset_; }

		void Reset()
		{
			offset_ = 0;
			cursor_ = begin_;
		}

		uint32_t Read(int len)
		{
			assert(len > 0 && len <= 32);

			uint32_t result = 0;
			while (len != 0)
			{
				auto x = std::min(len, 8 - offset_);

				len -= x;
				result <<= x;
				result |= ReadInternal(x);
			}

			return result;
		}

	private:
		// read a sequence of bits from the current byte
		uint32_t ReadInternal(int len)
		{
			using namespace std;

			static constexpr uint8_t bitmasks[] = {
				/*dummy*/0, 0x01, 0x03, 0x07, 0x0F, 0x1F, 0x3F, 0x7F, 0xFF
			};

			auto data = *cursor_;
			data >>= 8 - len - offset_;
			data &= bitmasks[len];
			
			offset_ += len;
			if (offset_ == 8)
			{
				++cursor_;
				offset_ = 0;
			}

			return data;
		}

		TIter begin_;
		TIter end_;

		int offset_ = 0;
		TIter cursor_;
	};

	class BitEmitter
	{
	public:
		const auto& Data() const
		{
			return data_;
		}

		auto Export()
		{
			return std::move(data_);
		}

		void Reset()
		{
			offset_ = 0;
			data_.clear();
		}

		void Write(uint32_t data, int len)
		{
			assert(len > 0 && len <= 32);

			while (len != 0)
			{
				data &= ~0 >> (32 - len);

				auto x = std::min(8 - offset_, len);
				auto t = data >> (len - x);

				WriteInternal(t, x);
				len -= x;
			}
		}

	private:
		void WriteInternal(uint8_t data, int len)
		{
			if (data_.empty() || offset_ == 8)
			{
				data_.push_back(0);
				offset_ = 0;
			}

			data_.back() |= data << 8 - offset_ - len;
			offset_ += len;
		}

		int offset_ = 0;
		std::vector<uint8_t> data_;
	};
}