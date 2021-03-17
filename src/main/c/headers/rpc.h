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

#ifndef RPC_H
#define RPC_H

#include <stdio.h>
#include <time.h>
#include <unistd.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 *  @details An accelerator request.
 */
typedef struct _inaccel_request *inaccel_request;

/**
 *  @details An accelerator response.
 */
typedef struct _inaccel_response *inaccel_response;

/**
 *  @details Used to set the array argument value for a specific argument of an
 *           accelerator.
 *
 *  @param request A valid request object.
 *  @param size Specifies the size of the argument value. The size specified
 *              will be the size in bytes of the array that must be allocated
 *              for the argument.
 *  @param value A pointer to data that should be used as the argument value for
 *               argument specified by index.
 *  @param index The argument index. Arguments to the accelerator are referred
 *               by indices that go from 0 for the leftmost argument to n - 1,
 *               where n is the total number of arguments declared by an
 *               accelerator.
 *  @returns On success, returns 0. On failure, it returns -1, and errno is set
 *           to indicate the cause of the error.
 */
int inaccel_request_arg_array(inaccel_request request, size_t size, const void *value, unsigned index);

/**
 *  @details Used to set the scalar argument value for a specific argument of an
 *           accelerator.
 *
 *  @param request A valid request object.
 *  @param size Specifies the size of the argument value. The size specified
 *              will be the size of argument type.
 *  @param value A pointer to data that should be used as the argument value for
 *               argument specified by index.
 *  @param index The argument index. Arguments to the accelerator are referred
 *               by indices that go from 0 for the leftmost argument to n - 1,
 *               where n is the total number of arguments declared by an
 *               accelerator.
 *  @returns On success, returns 0. On failure, it returns -1, and errno is set
 *           to indicate the cause of the error.
 */
int inaccel_request_arg_scalar(inaccel_request request, size_t size, const void *value, unsigned index);

/**
 *  @details Creates a request object.
 *
 *  @param accelerator An accelerator identifier.
 *  @returns On success, returns a valid non-zero request object. On failure, it
 *           returns a NULL value and errno is set to indicate the cause of the
 *           error.
 */
inaccel_request inaccel_request_create(const char *accelerator);

/**
 *  @details Write request output to stream.
 *
 *  @param stream Pointer to a FILE object that identifies an output stream.
 *  @param request A valid request object.
 *  @returns On success, the total number of characters written is returned. If
 *           a writing error occurs, the error indicator (ferror) is set and a
 *           negative number is returned.
 */
int inaccel_request_fprint(FILE *stream, const inaccel_request request);

/**
 *  @details Decrements the request reference count. The request object is
 *           deleted once the request object is no longer needed by any
 *           submitted task.
 *
 *  @param request A valid request object.
 */
void inaccel_request_release(inaccel_request request);

/**
 *  @details Write request output to sized buffer.
 *
 *  @param s Pointer to a buffer where the resulting C-string is stored. The
 *           buffer should have a size of at least n characters.
 *  @param n Maximum number of bytes to be used in the buffer. The generated
 *           string has a length of at most n-1, leaving space for the
 *           additional terminating null character.
 *  @param request A valid request object.
 *  @returns The number of characters that would have been written if n had been
 *           sufficiently large, not counting the terminating null character. If
 *           an encoding error occurs, a negative number is returned. Notice
 *           that only when this returned value is non-negative and less than n,
 *           the string has been completely written.
 */
int inaccel_request_snprint(char *s, size_t n, const inaccel_request request);

/**
 *  @details Creates a response object.
 *
 *  @returns On success, returns a valid non-zero response object. On failure,
 *           it returns a NULL value and errno is set to indicate the cause of
 *           the error.
 */
inaccel_response inaccel_response_create();

/**
 *  @details Decrements the response reference count. The response object is
 *           deleted once the response object is no longer needed by any
 *           submitted task.
 *
 *  @param response A valid response object.
 */
void inaccel_response_release(inaccel_response response);

/**
 *  @details Waits on the host thread for the accelerator request represented by
 *           the response object to complete.
 *
 *  @param response A valid response object.
 *  @returns On success, returns 0. On failure, it returns -1, and errno is set
 *           to indicate the cause of the error.
 */
int inaccel_response_wait(inaccel_response response);

/**
 *  @details Waits on the host thread for timeout, or the accelerator request
 *           represented by the response objects to complete (if the latter
 *           happens first).
 *
 *  @param response A valid response object.
 *  @param sec Represents the elapsed time, in whole seconds.
 *  @param nsec Captures rest of the elapsed time, represented as the number of
 *              nanoseconds.
 *  @returns On success, returns 0. On failure, it returns -1, and errno is set
 *           to indicate the cause of the error.
 */
int inaccel_response_wait_for(inaccel_response response, time_t sec, long nsec);

/**
 *  @details Schedules the request to be executed as accelerator(args...) with
 *           the response object representing the execution of the accelerator.
 *           Response objects should not be reused across submissions.
 *
 *  @param request A valid request object.
 *  @param response A valid response object.
 *  @returns On success, returns 0. On failure, it returns -1, and errno is set
 *           to indicate the cause of the error.
 */
int inaccel_submit(inaccel_request request, inaccel_response response);

#ifdef __cplusplus
}
#endif

#endif // RPC_H
