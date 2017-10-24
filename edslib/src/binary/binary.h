#pragma once
#include <vector>
#include <cassert>
#include <type_traits>
#include <complex>

namespace eds::binary
{
	using MemoryBuffer = std::vector<uint8_t>;

	class BinaryWriter
	{
	public:
		BinaryWriter() = default;
		BinaryWriter(int capacity)
		{
			data_.reserve(capacity);
		}

		void Write(const uint8_t* begin, const uint8_t* end)
		{
			data_.insert(data_.end(), begin, end);
		}
		void Clear()
		{
			data_.clear();
		}

		const auto& Buffer()
		{
			return data_;
		}
		MemoryBuffer Export()
		{
			return std::move(data_);
		}

	private:
		MemoryBuffer data_;
	};

	class BinaryReader
	{
	public:
		BinaryReader() = default;
		BinaryReader(MemoryBuffer buf)
			: data_(std::move(buf)) { }

		void Read(uint8_t* dest, int len)
		{
			assert(len > 0);

			if (cursor_ + len > data_.size())
				throw 0;

			memcpy(dest, data_.data() + cursor_, len);
			cursor_ += len;
		}
		void Reset()
		{
			cursor_ = 0;
		}

		const auto& Buffer()
		{
			return data_;
		}
		void Import(MemoryBuffer buf)
		{
			cursor_ = 0;
			data_ = std::move(buf);
		}

	private:
		int cursor_ = 0;
		MemoryBuffer data_;
	};

	namespace detail
	{
		template<typename T>
		constexpr bool IsRawDataType =
			std::is_fundamental_v<T> || std::is_enum_v<T>;
	}

	// Basic C++ types
	//
	template<
		typename T,
		typename = std::enable_if_t<detail::IsRawDataType<T>>>
		BinaryWriter& operator<<(BinaryWriter& writer, const T& data)
	{
		auto begin = reinterpret_cast<const uint8_t*>(&data);
		auto end = begin + sizeof data;

		writer.Write(begin, end);
		return writer;
	}
	template<
		typename T,
		typename = std::enable_if_t<detail::IsRawDataType<T>>>
		BinaryReader& operator>>(BinaryReader& reader, T& data)
	{
		auto dest = reinterpret_cast<uint8_t*>(&data);
		reader.Read(dest, sizeof data);

		return reader;
	}

	// std::complex
	//
	template<typename T>
	BinaryWriter& operator<<(BinaryWriter& writer, const std::complex<T>& data)
	{
		writer << data.real;
		writer << data.imag;

		return writer;
	}
	template<typename T>
	BinaryReader& operator<<(BinaryReader& reader, std::complex<T>& data)
	{
		reader >> data.real;
		reader >> data.imag;

		return reader;
	}

	// std::pair
	//
	template<typename T, typename U>
	BinaryWriter& operator<<(BinaryWriter& writer, const std::pair<T, U>& data)
	{
		writer << data.first;
		writer << data.second;

		return writer;
	}
	template<typename T, typename U>
	BinaryReader& operator>>(BinaryReader& reader, std::pair<T, U>& data)
	{
		reader >> data.first;
		reader >> data.second;

		return reader;
	}

	// std::basic_string
	//
	template<typename TChar>
	BinaryWriter& operator<<(BinaryWriter& writer, const std::basic_string<TChar>& data)
	{
		auto begin = reinterpret_cast<const uint8_t*>(data.data());
		auto end = reinterpret_cast<const uint8_t*>(data.data() + data.size());

		writer << static_cast<int>(std::distance(begin, end));
		writer.Write(begin, end);

		return writer;
	}
	template<typename TChar>
	BinaryReader& operator>>(BinaryReader& reader, std::basic_string<TChar>& data)
	{
		int data_sz;
		reader >> data_sz;

		data.resize(data_sz / sizeof(TChar));
		auto ptr = reinterpret_cast<uint8_t*>(data.data());
		reader.Read(ptr, data_sz);

		return reader;
	}

	// std::vector
	//
	template<typename T>
	BinaryWriter& operator<<(BinaryWriter& writer, const std::vector<T>& data)
	{
		int sz = data.size();
		writer << sz;
		for (const auto& elem : data)
		{
			writer << elem;
		}

		return writer;
	}
	template<typename T>
	BinaryReader& operator>>(BinaryReader& reader, std::vector<T>& data)
	{
		int sz;
		reader >> sz;

		// TODO: add support for non-default-constructible
		data.resize(sz);
		for (auto& elem : data)
		{
			reader >> elem;
		}

		return reader;
	}
}