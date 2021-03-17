/**
 * Copyright © 2018-2021 InAccel
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

#ifndef SHM_HPP
#define SHM_HPP

#include <vector>

#include "shm.h"

namespace inaccel {

	template<typename T>
	class allocator : public std::allocator<T> {

	public:

		typedef T *pointer;

		typedef size_t size_type;

		template<typename U>
		struct rebind {

			typedef inaccel::allocator<U> other;

		};

		pointer allocate(size_type n, const void *hint = 0) {
			pointer p = (pointer) inaccel_alloc(n * sizeof(T));
			if (!p) {
				throw std::bad_alloc();
			}
			return p;
		}

		void deallocate(pointer p, size_type n) {
			inaccel_free(p);
		}

	};

	template<typename T>
	using vector = std::vector<T, inaccel::allocator<T>>;

}

#endif // SHM_HPP
