#include <inaccel/coral.h>
#include <stdlib.h>

int main(void) {
	int size = 1024;

	float *a = (float *) inaccel_alloc(size * sizeof(float));
	if (!a) {
		perror("inaccel_alloc");
		return EXIT_FAILURE;
	}
	float *b = (float *) inaccel_alloc(size * sizeof(float));
	if (!b) {
		perror("inaccel_alloc");
		return EXIT_FAILURE;
	}
	float *c = (float *) inaccel_alloc(size * sizeof(float));
	if (!c) {
		perror("inaccel_alloc");
		return EXIT_FAILURE;
	}

	/* ... */

	inaccel_request request = inaccel_request_create("vector.addition");
	if (!request) {
		perror("inaccel_request_create");
		return EXIT_FAILURE;
	}

	if (inaccel_request_arg_array(request, size * sizeof(float), a, 0)) {
		perror("inaccel_request_arg_array");
		return EXIT_FAILURE;
	}
	if (inaccel_request_arg_array(request, size * sizeof(float), b, 1)) {
		perror("inaccel_request_arg_array");
		return EXIT_FAILURE;
	}
	if (inaccel_request_arg_array(request, size * sizeof(float), c, 2)) {
		perror("inaccel_request_arg_array");
		return EXIT_FAILURE;
	}
	if (inaccel_request_arg_scalar(request, sizeof(int), &size, 3)) {
		perror("inaccel_request_arg_scalar");
		return EXIT_FAILURE;
	}

	inaccel_request_fprint(stdout, request);
	fprintf(stdout, "\n");

	inaccel_response response = inaccel_response_create();
	if (!response) {
		perror("inaccel_response_create");
		return EXIT_FAILURE;
	}

	if (inaccel_submit(request, response)) {
		perror("inaccel_submit");
		return EXIT_FAILURE;
	}

	switch (inaccel_response_wait(response)) {
	case -1:
		perror("inaccel_response_wait");
		return EXIT_FAILURE;
	case 0:
		break;
	default:
		inaccel_response_fprint(stderr, response);
		fprintf(stderr, "\n");
		return EXIT_FAILURE;
	}

	inaccel_response_release(response);

	inaccel_request_release(request);

	/* ... */

	inaccel_free(c);
	inaccel_free(b);
	inaccel_free(a);

	return EXIT_SUCCESS;
}
