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

#ifndef SOL_STACK_FIELD_HPP
#define SOL_STACK_FIELD_HPP

#include <sol/stack_core.hpp>
#include <sol/stack_push.hpp>
#include <sol/stack_get.hpp>
#include <sol/stack_check_get.hpp>

namespace sol { namespace stack {
	template <typename T, bool global, bool raw, typename>
	struct field_getter {
		static constexpr int default_table_index = meta::conditional_t < meta::is_c_str_v<T>
#if SOL_LUA_VESION_I_ >= 503
			|| (std::is_integral_v<T> && !std::is_same_v<T, bool>)
#endif // integer  global keys 5.3 or better
			|| (raw && std::is_void_v<std::remove_pointer_t<T>>),
			                std::integral_constant<int, -1>, std::integral_constant<int, -2> > ::value;

		template <typename Key>
		void get(lua_State* L, Key&& key, int tableindex = default_table_index) {
			if constexpr (std::is_same_v<T, update_if_empty_t> || std::is_same_v<T, override_value_t> || std::is_same_v<T, create_if_nil_t>) {
				(void)L;
				(void)key;
				(void)tableindex;
			}
			else if constexpr (std::is_same_v<T, env_key_t>) {
				(void)key;
#if SOL_LUA_VESION_I_ < 502
				// Use lua_setfenv
				lua_getfenv(L, tableindex);
#else
				// Use upvalues as explained in Lua 5.2 and beyond's manual
				if (lua_getupvalue(L, tableindex, 1) == nullptr) {
					push(L, lua_nil);
				}
#endif
			}
			else if constexpr (std::is_same_v<T, metatable_key_t>) {
				(void)key;
				if (lua_getmetatable(L, tableindex) == 0)
					push(L, lua_nil);
			}
			else if constexpr (raw) {
				if constexpr (std::is_integral_v<T> && !std::is_same_v<bool, T>) {
					lua_rawgeti(L, tableindex, static_cast<lua_Integer>(key));
				}
#if SOL_LUA_VESION_I_ >= 502
				else if constexpr (std::is_void_v<std::remove_pointer_t<T>>) {
					lua_rawgetp(L, tableindex, key);
				}
#endif // Lua 5.2.x+
				else {
					push(L, std::forward<Key>(key));
					lua_rawget(L, tableindex);
				}
			}
			else {
				if constexpr (meta::is_c_str_v<T>) {
					if constexpr (global) {
						(void)tableindex;
						lua_getglobal(L, &key[0]);
					}
					else {
						lua_getfield(L, tableindex, &key[0]);
					}
				}
#if SOL_LUA_VESION_I_ >= 503
				else if constexpr (std::is_integral_v<T> && !std::is_same_v<bool, T>) {
					lua_geti(L, tableindex, static_cast<lua_Integer>(key));
				}
#endif // Lua 5.3.x+
				else {
					push(L, std::forward<Key>(key));
					lua_gettable(L, tableindex);
				}
			}
		}
	};

	template <typename... Args, bool b, bool raw, typename C>
	struct field_getter<std::tuple<Args...>, b, raw, C> {
		template <std::size_t... I, typename Keys>
		void apply(std::index_sequence<0, I...>, lua_State* L, Keys&& keys, int tableindex) {
			get_field<b, raw>(L, std::get<0>(std::forward<Keys>(keys)), tableindex);
			void(detail::swallow { (get_field<false, raw>(L, std::get<I>(std::forward<Keys>(keys))), 0)... });
			reference saved(L, -1);
			lua_pop(L, static_cast<int>(sizeof...(I)));
			saved.push();
		}

		template <typename Keys>
		void get(lua_State* L, Keys&& keys) {
			apply(std::make_index_sequence<sizeof...(Args)>(), L, std::forward<Keys>(keys), lua_absindex(L, -1));
		}

		template <typename Keys>
		void get(lua_State* L, Keys&& keys, int tableindex) {
			apply(std::make_index_sequence<sizeof...(Args)>(), L, std::forward<Keys>(keys), tableindex);
		}
	};

	template <typename A, typename B, bool b, bool raw, typename C>
	struct field_getter<std::pair<A, B>, b, raw, C> {
		template <typename Keys>
		void get(lua_State* L, Keys&& keys, int tableindex) {
			get_field<b, raw>(L, std::get<0>(std::forward<Keys>(keys)), tableindex);
			get_field<false, raw>(L, std::get<1>(std::forward<Keys>(keys)));
			reference saved(L, -1);
			lua_pop(L, static_cast<int>(2));
			saved.push();
		}

		template <typename Keys>
		void get(lua_State* L, Keys&& keys) {
			get_field<b, raw>(L, std::get<0>(std::forward<Keys>(keys)));
			get_field<false, raw>(L, std::get<1>(std::forward<Keys>(keys)));
			reference saved(L, -1);
			lua_pop(L, static_cast<int>(2));
			saved.push();
		}
	};

	template <typename T, bool global, bool raw, typename>
	struct field_setter {
		static constexpr int default_table_index
			= meta::conditional_t < (meta::is_c_str_v<T> || meta::is_string_of_v<T, char>) || (std::is_integral_v<T> && !std::is_same_v<T, bool>)
			|| (std::is_integral_v<T> && !std::is_same_v<T, bool>) || (raw && std::is_void_v<std::remove_pointer_t<T>>),
			std::integral_constant<int, -2>, std::integral_constant<int, -3> > ::value;

		template <typename Key, typename Value>
		void set(lua_State* L, Key&& key, Value&& value, int tableindex = default_table_index) {
			if constexpr (std::is_same_v<T, update_if_empty_t> || std::is_same_v<T, override_value_t>) {
				(void)L;
				(void)key;
				(void)value;
				(void)tableindex;
			}
			else if constexpr (std::is_same_v<T, metatable_key_t>) {
				(void)key;
				push(L, std::forward<Value>(value));
				lua_setmetatable(L, tableindex);
			}
			else if constexpr (raw) {
				if constexpr (std::is_integral_v<T> && !std::is_same_v<bool, T>) {
					push(L, std::forward<Value>(value));
					lua_rawseti(L, tableindex, static_cast<lua_Integer>(key));
				}
#if SOL_LUA_VESION_I_ >= 502
				else if constexpr (std::is_void_v<std::remove_pointer_t<T>>) {
					push(L, std::forward<Value>(value));
					lua_rawsetp(L, tableindex, std::forward<Key>(key));
				}
#endif // Lua 5.2.x
				else {
					push(L, std::forward<Key>(key));
					push(L, std::forward<Value>(value));
					lua_rawset(L, tableindex);
				}
			}
			else {
				if constexpr (meta::is_c_str_v<T> || meta::is_string_of_v<T, char>) {
					if constexpr (global) {
						push(L, std::forward<Value>(value));
						lua_setglobal(L, &key[0]);
						(void)tableindex;
					}
					else {
						push(L, std::forward<Value>(value));
						lua_setfield(L, tableindex, &key[0]);
					}
				}
#if SOL_LUA_VESION_I_ >= 503
				else if constexpr (std::is_integral_v<T> && !std::is_same_v<bool, T>) {
					push(L, std::forward<Value>(value));
					lua_seti(L, tableindex, static_cast<lua_Integer>(key));
				}
#endif // Lua 5.3.x
				else {
					push(L, std::forward<Key>(key));
					push(L, std::forward<Value>(value));
					lua_settable(L, tableindex);
				}
			}
		}
	};

	template <typename... Args, bool b, bool raw, typename C>
	struct field_setter<std::tuple<Args...>, b, raw, C> {
		template <bool g, std::size_t I, typename Keys, typename Value>
		void apply(std::index_sequence<I>, lua_State* L, Keys&& keys, Value&& value, int tableindex) {
			I < 1 ? set_field<g, raw>(L, std::get<I>(std::forward<Keys>(keys)), std::forward<Value>(value), tableindex)
				 : set_field<g, raw>(L, std::get<I>(std::forward<Keys>(keys)), std::forward<Value>(value));
		}

		template <bool g, std::size_t I0, std::size_t I1, std::size_t... I, typename Keys, typename Value>
		void apply(std::index_sequence<I0, I1, I...>, lua_State* L, Keys&& keys, Value&& value, int tableindex) {
			I0 < 1 ? get_field<g, raw>(L, std::get<I0>(std::forward<Keys>(keys)), tableindex)
				  : get_field<g, raw>(L, std::get<I0>(std::forward<Keys>(keys)), -1);
			apply<false>(std::index_sequence<I1, I...>(), L, std::forward<Keys>(keys), std::forward<Value>(value), -1);
		}

		template <bool g, std::size_t I0, std::size_t... I, typename Keys, typename Value>
		void top_apply(std::index_sequence<I0, I...>, lua_State* L, Keys&& keys, Value&& value, int tableindex) {
			apply<g>(std::index_sequence<I0, I...>(), L, std::forward<Keys>(keys), std::forward<Value>(value), tableindex);
			lua_pop(L, static_cast<int>(sizeof...(I)));
		}

		template <typename Keys, typename Value>
		void set(lua_State* L, Keys&& keys, Value&& value, int tableindex = -3) {
			top_apply<b>(std::make_index_sequence<sizeof...(Args)>(), L, std::forward<Keys>(keys), std::forward<Value>(value), tableindex);
		}
	};

	template <typename A, typename B, bool b, bool raw, typename C>
	struct field_setter<std::pair<A, B>, b, raw, C> {
		template <typename Keys, typename Value>
		void set(lua_State* L, Keys&& keys, Value&& value, int tableindex = -1) {
			get_field<b, raw>(L, std::get<0>(std::forward<Keys>(keys)), tableindex);
			set_field<false, raw>(L, std::get<1>(std::forward<Keys>(keys)), std::forward<Value>(value), lua_gettop(L));
			lua_pop(L, 1);
		}
	};
}} // namespace sol::stack

#endif // SOL_STACK_FIELD_HPP
