#include <inaccel/coral>
#include <iostream>

int main() {
	int size = 1024;

	inaccel::vector<float> a(size), b(size), c(size);

	inaccel::request request("vector.addition");

	request.arg(a).arg(b).arg(c).arg(size);

	std::cout << request << std::endl;

	std::future<void> response = inaccel::submit(request);

	response.get();
}
