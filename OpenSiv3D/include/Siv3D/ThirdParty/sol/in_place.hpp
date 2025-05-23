// sol3

// The MIT License (MIT)

// Copyright (c) 2013-2020 Rapptz, ThePhD and contributors

// Permission is hereby granted, free of charge, to any person obtaining a copy of
// this software and associated documentation files (the "Software"), to deal in
// the Software without restriction, including without limitation the rights to
// use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
// the Software, and to permit persons to whom the Software is furnished to do so,
// subject to the following conditions:

// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
// FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
// COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
// IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
// CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#ifndef SOL_IN_PLACE_HPP
#define SOL_IN_PLACE_HPP

#include <cstddef>
#include <utility>

namespace sol {

	using in_place_t = std::in_place_t;
	constexpr std::in_place_t in_place {};
	constexpr std::in_place_t in_place_of {};

	template <typename T>
	using in_place_type_t = std::in_place_type_t<T>;
	template <typename T>
	constexpr std::in_place_type_t<T> in_place_type {};

	template <size_t I>
	using in_place_index_t = std::in_place_index_t<I>;
	template <size_t I>
	constexpr in_place_index_t<I> in_place_index {};

} // namespace sol

#endif // SOL_IN_PLACE_HPP
