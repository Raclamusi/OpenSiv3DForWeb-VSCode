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

#ifndef SOL_STATE_DEFAULT_HPP
#define SOL_STATE_DEFAULT_HPP

#include <sol/trampoline.hpp>
#include <sol/stack.hpp>
#include <sol/error.hpp>
#include <sol/function.hpp>
#include <sol/object.hpp>
#include <sol/lua_value.hpp>

#if SOL_IS_ON(SOL_PRINT_ERRORS_I_)
#include <iostream>
#endif

namespace sol {
	inline void register_main_thread(lua_State* L) {
#if SOL_LUA_VESION_I_ < 502
		if (L == nullptr) {
			lua_pushnil(L);
			lua_setglobal(L, detail::default_main_thread_name());
			return;
		}
		lua_pushthread(L);
		lua_setglobal(L, detail::default_main_thread_name());
#else
		(void)L;
#endif
	}

	inline int default_at_panic(lua_State* L) {
#if SOL_IS_OFF(SOL_EXCEPTIONS_I_)
		(void)L;
		return -1;
#else
		size_t messagesize;
		const char* message = lua_tolstring(L, -1, &messagesize);
		if (message) {
			std::string err(message, messagesize);
			lua_settop(L, 0);
#if SOL_IS_ON(SOL_PRINT_ERRORS_I_)
			std::cerr << "[sol3] An error occurred and panic has been invoked: ";
			std::cerr << err;
			std::cerr << std::endl;
#endif
			throw error(err);
		}
		lua_settop(L, 0);
		throw error(std::string("An unexpected error occurred and panic has been invoked"));
#endif // Printing Errors
	}

	inline int default_traceback_error_handler(lua_State* L) {
		std::string msg = "An unknown error has triggered the default error handler";
		optional<string_view> maybetopmsg = stack::unqualified_check_get<string_view>(L, 1, no_panic);
		if (maybetopmsg) {
			const string_view& topmsg = maybetopmsg.value();
			msg.assign(topmsg.data(), topmsg.size());
		}
		luaL_traceback(L, L, msg.c_str(), 1);
		optional<string_view> maybetraceback = stack::unqualified_check_get<string_view>(L, -1, no_panic);
		if (maybetraceback) {
			const string_view& traceback = maybetraceback.value();
			msg.assign(traceback.data(), traceback.size());
		}
#if SOL_IS_ON(SOL_PRINT_ERRORS_I_)
		// std::cerr << "[sol3] An error occurred and was caught in traceback: ";
		// std::cerr << msg;
		// std::cerr << std::endl;
#endif // Printing
		return stack::push(L, msg);
	}

	inline void set_default_state(lua_State* L, lua_CFunction panic_function = &default_at_panic,
	     lua_CFunction traceback_function = c_call<decltype(&default_traceback_error_handler), &default_traceback_error_handler>,
	     exception_handler_function exf = detail::default_exception_handler) {
		lua_atpanic(L, panic_function);
		protected_function::set_default_handler(object(L, in_place, traceback_function));
		set_default_exception_handler(L, exf);
		register_main_thread(L);
		stack::luajit_exception_handler(L);
		lua_value::set_lua_state(L);
	}

	inline std::size_t total_memory_used(lua_State* L) {
		std::size_t kb = lua_gc(L, LUA_GCCOUNT, 0);
		kb *= 1024;
		kb += lua_gc(L, LUA_GCCOUNTB, 0);
		return kb;
	}

	inline protected_function_result script_pass_on_error(lua_State*, protected_function_result result) {
		return result;
	}

	inline protected_function_result script_throw_on_error(lua_State* L, protected_function_result result) {
		type t = type_of(L, result.stack_index());
		std::string err = "sol: ";
		err += to_string(result.status());
		err += " error";
#if SOL_IS_ON(SOL_EXCEPTIONS_I_)
		std::exception_ptr eptr = std::current_exception();
		if (eptr) {
			err += " with a ";
			try {
				std::rethrow_exception(eptr);
			}
			catch (const std::exception& ex) {
				err += "std::exception -- ";
				err.append(ex.what());
			}
			catch (const std::string& message) {
				err += "thrown message -- ";
				err.append(message);
			}
			catch (const char* message) {
				err += "thrown message -- ";
				err.append(message);
			}
			catch (...) {
				err.append("thrown but unknown type, cannot serialize into error message");
			}
		}
#endif // serialize exception information if possible
		if (t == type::string) {
			err += ": ";
			string_view serr = stack::unqualified_get<string_view>(L, result.stack_index());
			err.append(serr.data(), serr.size());
		}
#if SOL_IS_ON(SOL_PRINT_ERRORS_I_)
		std::cerr << "[sol3] An error occurred and has been passed to an error handler: ";
		std::cerr << err;
		std::cerr << std::endl;
#endif
		// replacing information of stack error into pfr
		int target = result.stack_index();
		if (result.pop_count() > 0) {
			stack::remove(L, target, result.pop_count());
		}
		stack::push(L, err);
		int top = lua_gettop(L);
		int towards = top - target;
		if (towards != 0) {
			lua_rotate(L, top, towards);
		}
#if SOL_IS_OFF(SOL_EXCEPTIONS_I_)
		return result;
#else
		// just throw our error
		throw error(detail::direct_error, err);
#endif // If exceptions are allowed
	}

	inline protected_function_result script_default_on_error(lua_State* L, protected_function_result pfr) {
#if SOL_IS_ON(SOL_DEFAULT_PASS_ON_ERROR_I_)
		return script_pass_on_error(L, std::move(pfr));
#else
		return script_throw_on_error(L, std::move(pfr));
#endif
	}

	namespace stack {
		inline error get_traceback_or_errors(lua_State* L) {
			int p = default_traceback_error_handler(L);
			sol::error err = stack::get<sol::error>(L, -p);
			lua_pop(L, p);
			return err;
		}
	} // namespace stack
} // namespace sol

#endif // SOL_STATE_DEFAULT_HPP
