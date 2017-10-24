/*=================================================================================
*  Copyright (c) 2016 Edward Cheng
*
*  edslib is an open-source library in C++ and licensed under the MIT License.
*  Refer to: https://opensource.org/licenses/MIT
*================================================================================*/

#pragma once
#include "text-utils.hpp"
#include <stdexcept>
#include <functional>
#include <string>
#include <sstream>
#include <iostream>

namespace eds::text
{
	//=====================================================================================
	// Declarations

	///<summary><c>FormatError</c> is thrown when failed to construct a formated string.</summary>
	class FormatError : public std::runtime_error
	{
	public:
		explicit FormatError(const char *msg) : std::runtime_error(msg) { }
	};

	///<summary>Create a formatted string in ASCII.</summary>
	///<param name='formatter'>A text template to construct the formatted string.</param>
	///<param name='args'>Variadic parameters referred in <c>formatter</c>.</param>
	template<typename ... TArgs>
	std::string Format(const char *formatter, const TArgs &...args);

	template<typename ...TArgs>
	void PrintFormatted(const char* formatter, const TArgs& ...args);


	//=====================================================================================
	// Implementations

	namespace detail
	{
		inline void FormatAssert(bool pred, const char *msg = "unknown format error.")
		{
			if (!pred)
			{
				throw FormatError(msg);
			}
		}

		// format string example "{}{1}"
		// "{}" => automatic incremental id in args
		// "{n}" => explicit id in args
		// no whitespace allowed in brace pair
		// double typed brace to generate raw brace character
		// currently counts of args is limited under 10
		template<typename TStream, typename ... TArgs>
		void FormatInternal(TStream& output, const char* formatter, const TArgs& ...args)
		{
			static_assert(sizeof...(args) < 11, "more than 10 args not supported.");

			// special case: empty formatter
			if (!*formatter) return;

			// wrap args into function delegates
			// helper[i](buf); should write (i+1)th argument into the buffer
			constexpr auto arg_count = sizeof...(args);
			std::function<void(TStream&)> helpers[10]
				= { ([&](auto& ss) { ss << args; })... };

			size_t next_id = 0;
			for (auto p = formatter; *p;)
			{
				if (ConsumeIf(p, '{'))
				{
					if (ConsumeIf(p, '{'))
					{
						output.put('{');
					}
					else
					{
						size_t id;
						if (ConsumeIf(p, '}'))
						{
							// empty brace pair
							// id should be incremental
							id = next_id++;
						}
						else
						{
							assert(isdigit(*p));

							// id explicitly specified, should in [0, 10)
							id = Consume(p) - '0';
							FormatAssert(id >= 0 && id <= 9, "argument id must be within [0,10).");
							FormatAssert(id < arg_count, "not enough arguments.");

							FormatAssert(ConsumeIf(p, '}'), "invalid argument reference.");
						}

						helpers[id](output);
					}
				}
				else if (ConsumeIf(p, '}'))
				{
					if (ConsumeIf(p, '}'))
					{
						output.put('}');
					}
					else
					{
						FormatAssert(false, "an isolated closing brace is not allowed.");
					}
				}
				else
				{
					output.put(Consume(p));
				}
			}
		}

	} // namespace detail

	template<typename ...TArgs>
	inline std::string Format(const char *formatter, const TArgs& ...args)
	{
		std::stringstream buf;
		detail::FormatInternal(buf, formatter, args...);

		return buf.str();
	}

	template<typename ...TArgs>
	inline void PrintFormatted(const char* formatter, const TArgs& ...args)
	{
		detail::FormatInternal(std::cout, formatter, args...);
	}

} // namespace eds