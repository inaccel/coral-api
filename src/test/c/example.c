#include <inaccel/coral.h>

int main() {
	int size = 1024;

	float *a = (float *) inaccel_alloc(size * sizeof(float));
	float *b = (float *) inaccel_alloc(size * sizeof(float));
	float *c = (float *) inaccel_alloc(size * sizeof(float));

	inaccel_request request = inaccel_request_create("vector.addition");

	inaccel_request_arg_array(request, size * sizeof(float), a, 0);
	inaccel_request_arg_array(request, size * sizeof(float), b, 1);
	inaccel_request_arg_array(request, size * sizeof(float), c, 2);
	inaccel_request_arg_scalar(request, sizeof(int), &size, 3);

	inaccel_request_fprint(stdout, request);
	fprintf(stdout, "\n");

	inaccel_response response = inaccel_response_create();

	inaccel_submit(request, response);

	inaccel_response_wait(response);

	inaccel_response_release(response);

	inaccel_request_release(request);

	inaccel_free(c);
	inaccel_free(b);
	inaccel_free(a);
}
