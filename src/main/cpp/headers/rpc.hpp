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

#ifndef INACCEL_RPC_HPP
#define INACCEL_RPC_HPP

#include <cstring>
#include <future>

#include "rpc.h"
#include "shm.hpp"

namespace inaccel {

	class exception : public std::exception {

	private:

		std::string msg;

	public:

		explicit exception(const std::string &arg) : std::exception(), msg(arg) {}

		virtual ~exception() noexcept {}

		virtual const char *what() const noexcept {
			return msg.c_str();
		}

	};

	class request;

	static std::future<void> submit(const inaccel::request &request);

	static std::ostream &operator<<(std::ostream &os, const inaccel::request &request);

	class request {

		friend std::future<void> submit(const inaccel::request &request);

		friend std::ostream &operator<<(std::ostream &os, const inaccel::request &request);

	private:

		inaccel_request c;

		unsigned index = 0;

	public:

		request(const std::string &accelerator) {
			c = inaccel_request_create(accelerator.c_str());
			if (!c) {
				throw std::runtime_error(std::strerror(errno));
			}
		}

		request(const request &request) = delete;

		~request() {
			inaccel_request_release(c);
		}

		template <typename T>
		request &arg(const T &value) {
			arg(value, index);

			index++;

			return *this;
		}

		template <typename T>
		request &arg(const T &value, unsigned index) {
			int error = inaccel_request_arg_scalar(c, sizeof(T), &value, index);
			if (error) {
				throw std::runtime_error(std::strerror(errno));
			}

			return *this;
		}

		template <typename T>
		request &arg(const std::vector<T> &value) {
			arg(value, index);

			index++;

			return *this;
		}

		template <typename T>
		request &arg(const std::vector<T> &value, unsigned index) {
			int error = inaccel_request_arg_scalar(c, value.size() * sizeof(T), value.data(), index);
			if (error) {
				throw std::runtime_error(std::strerror(errno));
			}

			return *this;
		}

		template <typename T>
		request &arg(const typename std::vector<T>::iterator &first, const typename std::vector<T>::iterator &last) {
			arg<T>(first, last, index);

			index++;

			return *this;
		}

		template <typename T>
		request &arg(const typename std::vector<T>::iterator &first, const typename std::vector<T>::iterator &last, unsigned index) {
			int error = inaccel_request_arg_scalar(c, (last - first) * sizeof(T), &(*first), index);
			if (error) {
				throw std::runtime_error(std::strerror(errno));
			}

			return *this;
		}

		template <typename T>
		request &arg_scalar(const T *first, const T *last) {
			arg_scalar(first, last, index);

			index++;

			return *this;
		}

		template <typename T>
		request &arg_scalar(const T *first, const T *last, unsigned index) {
			int error = inaccel_request_arg_scalar(c, (last - first) * sizeof(T), first, index);
			if (error) {
				throw std::runtime_error(std::strerror(errno));
			}

			return *this;
		}

		template <typename T>
		request &arg(const inaccel::vector<T> &value) {
			arg(value, index);

			index++;

			return *this;
		}

		template <typename T>
		request &arg(const inaccel::vector<T> &value, unsigned index) {
			int error = inaccel_request_arg_array(c, value.size() * sizeof(T), value.data(), index);
			if (error) {
				throw std::runtime_error(std::strerror(errno));
			}

			return *this;
		}

		template <typename T>
		request &arg(const typename inaccel::vector<T>::iterator &first, const typename inaccel::vector<T>::iterator &last) {
			arg<T>(first, last, index);

			index++;

			return *this;
		}

		template <typename T>
		request &arg(const typename inaccel::vector<T>::iterator &first, const typename inaccel::vector<T>::iterator &last, unsigned index) {
			int error = inaccel_request_arg_array(c, (last - first) * sizeof(T), &(*first), index);
			if (error) {
				throw std::runtime_error(std::strerror(errno));
			}

			return *this;
		}

		template <typename T>
		request &arg_array(const T *first, const T *last) {
			arg_array(first, last, index);

			index++;

			return *this;
		}

		template <typename T>
		request &arg_array(const T *first, const T *last, unsigned index) {
			int error = inaccel_request_arg_array(c, (last - first) * sizeof(T), first, index);
			if (error) {
				throw std::runtime_error(std::strerror(errno));
			}

			return *this;
		}

		request &operator=(const request &request) = delete;

	};

	static std::future<void> submit(const inaccel::request &request) {
		inaccel_response cresponse = inaccel_response_create();
		if (!cresponse) {
			throw std::runtime_error(std::strerror(errno));
		}

		int error = inaccel_submit(request.c, cresponse);
		if (error) {
			int errsv = errno;

			inaccel_response_release(cresponse);

			throw std::runtime_error(std::strerror(errsv));
		}

		return std::async(std::launch::deferred, [] (inaccel_response cresponse) {
			int error = inaccel_response_wait(cresponse);
			if (error == -1) {
				int errsv = errno;

				inaccel_response_release(cresponse);

				throw std::runtime_error(std::strerror(errsv));
			} else if (error) {
				int n = inaccel_response_snprint(NULL, 0, cresponse);
				if (n < 0) {
					int errsv = errno;

					inaccel_response_release(cresponse);

					throw std::runtime_error(std::strerror(errsv));
				}

				std::vector<char> s(n + 1);
				if (inaccel_response_snprint(s.data(), s.capacity(), cresponse) != n) {
					int errsv = errno;

					inaccel_response_release(cresponse);

					throw std::runtime_error(std::strerror(errsv));
				}

				inaccel_response_release(cresponse);

				throw exception(std::string(s.begin(), s.end()));
			}

			inaccel_response_release(cresponse);
		}, cresponse);
	}

	static std::ostream &operator<<(std::ostream &os, const inaccel::request &request) {
		int n = inaccel_request_snprint(NULL, 0, request.c);
		if (n < 0) {
			throw std::runtime_error(std::strerror(errno));
		}

		std::vector<char> s(n + 1);
		if (inaccel_request_snprint(s.data(), s.capacity(), request.c) != n) {
			throw std::runtime_error(std::strerror(errno));
		}

		return os << std::string(s.begin(), s.end());
	}

}

#endif // INACCEL_RPC_HPP
