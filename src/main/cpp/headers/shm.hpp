/**
 * Copyright © 2018-2023 InAccel
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef INACCEL_SHM_HPP
#define INACCEL_SHM_HPP

#if __GNUC__ >= 9 && __cplusplus >= 201703L
#include <memory_resource>
#endif
#include <vector>

#include "shm.h"

namespace inaccel {

	template<typename T>
	class allocator : public std::allocator<T> {

	public:

		typedef T *pointer;

		typedef size_t size_type;

		typedef T value_type;

		template<typename U>
		struct rebind {

			typedef inaccel::allocator<U> other;

		};

		pointer allocate(size_type n, const void *hint = 0) {
			pointer p = (pointer) inaccel_alloc(n * sizeof(value_type));
			if (!p) {
				throw std::bad_alloc();
			}
			return p;
		}

		void deallocate(pointer p, size_type n) {
			inaccel_free(p);
		}

	};

	static void *alloc(size_t size) {
		return inaccel_alloc(size);
	}

	static void free(void *ptr) {
		inaccel_free(ptr);
	}

	static void *realloc(void *ptr, size_t size) {
		return inaccel_realloc(ptr, size);
	}

	template<typename T>
	using vector = std::vector<T, inaccel::allocator<T>>;

#if __GNUC__ >= 9 && __cplusplus >= 201703L
	namespace pmr {

		class memory_resource : public std::pmr::memory_resource {

		private:

			virtual void *do_allocate(size_t bytes, size_t alignment) {
				void *p = inaccel_alloc(bytes);
				if (!p) {
					throw std::bad_alloc();
				}
				return p;
			}

			virtual void do_deallocate(void *p, size_t bytes, size_t alignment) {
				inaccel_free(p);
			}

			virtual bool do_is_equal(const std::pmr::memory_resource &other) const noexcept {
				return dynamic_cast<const memory_resource *>(&other);
			}

		};

		static std::pmr::memory_resource *resource() noexcept {
			static inaccel::pmr::memory_resource resource;

			return &resource;
		}

	}
#endif

}

#endif // INACCEL_SHM_HPP
