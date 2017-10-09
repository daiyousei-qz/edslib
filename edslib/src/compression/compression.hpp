#pragma once
#include "../arena.hpp"
#include "../binary/bit-ops.hpp"
#include "../ext/type-utils.hpp"
#include <vector>

// LWZ implementation
// ASSUMES MSB-first behavior
namespace eds::compression
{
	namespace detail
	{
		static constexpr int kMinCodeWidth = 8;
		static constexpr int kMaxCodeWidth = 16;
		static constexpr int kCodeWidthIncrementalStep = 1;

		struct TierNode
		{
			uint32_t code;
			int length;

			uint8_t value;
			TierNode* parent;
			TierNode* children[256];
		};

		class LzwDictionary
		{
		public:
			LzwDictionary()
			{
				for (auto i = 0; i < 256; ++i)
				{
					root_nodes_[i] = NewNode(nullptr, i);
				}
			}

			int CodeWidth() { return code_width_; }
			bool AllowGrowth() { return next_code_ < (1 << kMaxCodeWidth); }

			TierNode* LookupRoot(uint8_t b)
			{
				return root_nodes_[b];
			}

			void ReserveWidth()
			{
				if (next_code_ >= (1 << code_width_) && AllowGrowth())
				{
					code_width_ += kCodeWidthIncrementalStep;
				}
			}

			TierNode* UpdateNode(TierNode* prefix, uint8_t b)
			{
				return (prefix->children[b] = NewNode(prefix, b));
			}

		private:
			TierNode* NewNode(TierNode* prefix, uint8_t value)
			{
				auto node = arena_.Construct<TierNode>();
				node->code = next_code_++;
				node->length = prefix == nullptr ? 1 : prefix->length + 1;
				node->value = value;
				node->parent = prefix;
				std::fill_n(&node->children[0], 256, nullptr);

				return node;
			}

			Arena arena_;
			TierNode* root_nodes_[256];

			int code_width_ = kMinCodeWidth;
			uint32_t next_code_ = 0;
		};

		// expands node into the sequence it encodes
		inline void ExpandDecodedSeq(std::vector<uint8_t>& output, TierNode* node)
		{
			output.resize(output.size() + node->length);

			auto it = output.rbegin();
			auto p = node;
			while (p)
			{
				*it = p->value;

				p = p->parent;
				++it;
			}
		}
	}

	template <typename TIter>
	inline auto EncodeLzw(TIter begin, TIter end)
	{
		static_assert(eds::type::is_iterator_of_v<TIter, uint8_t>, "TIter must be an iterator type of uint8_t");

		using namespace std;
		using namespace eds::compression::detail;

		LzwDictionary dict;
		BitEmitter emit;

		for (auto p = begin; p != end;)
		{
			auto node = dict.LookupRoot(*p);
			++p;

			while (p != end && node->children[*p] != nullptr)
			{
				node = node->children[*p];
				++p;
			}

			emit.Write(node->code, dict.CodeWidth());
			if (p != end && dict.AllowGrowth())
			{
				dict.ReserveWidth();
				dict.UpdateNode(node, *p);
			}
		}

		return emit.Export();
	}

	template <typename TIter>
	inline auto DecodeLzw(TIter begin, TIter end)
	{
		static_assert(eds::type::is_iterator_of_v<TIter, uint8_t>, "TIter must be an iterator type of uint8_t");

		using namespace std;
		using namespace eds::compression::detail;

		LzwDictionary dict;
		std::vector<TierNode*> code_lookup;
		for (auto i = 0; i < 256; ++i)
		{
			code_lookup.push_back(dict.LookupRoot(i));
		}

		BitReader<TIter> reader{ begin, end };
		vector<uint8_t> result;
		TierNode* last_seq_tail = nullptr;
		while (reader.RemainingSize() >= dict.CodeWidth())
		{
			// load next code
			auto code = reader.Read(dict.CodeWidth());

			// append decoded data to result
			// after expansion, result[old_size] would be the first element of the sequence generated
			auto old_size = result.size();
			if (code < code_lookup.size())
			{
				ExpandDecodedSeq(result, code_lookup[code]);
			}
			else if (last_seq_tail && code == code_lookup.size())
			{
				ExpandDecodedSeq(result, last_seq_tail);
				result.push_back(result[old_size]);
			}
			else
			{
				throw 0; // not a valid lzw stream
			}

			// update dictionary
			if (last_seq_tail && dict.AllowGrowth())
			{
				auto new_node = dict.UpdateNode(last_seq_tail, result[old_size]);
				code_lookup.push_back(new_node);
			}
			dict.ReserveWidth();

			last_seq_tail = code_lookup[code];
		}

		return result;
	}

}