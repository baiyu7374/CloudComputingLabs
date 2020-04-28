#include <iostream>
// #include <random>

#include "ThreadPool.h"

// std::random_device rd;
// std::mt19939 mt(rd());
// std::uniform_int_distribution<int> dist(-1000, 1000);

// auto rnd = std::bind(dist, mt);

void simulate_hard_computation() {
	std::this_thread::sleep_for(std::chrono::milliseconds(200));
}

// Simple function that adds multiplies two numbers and prints the result
void multiply(const int a, const int b) {
  simulate_hard_computation();
  const int res = a * b;
  std::cout << a << " * " << b << " = " << res << std::endl;
}

// Same as before but now we have an output parameter
void multiply_output(int & out, const int a, const int b) {
  simulate_hard_computation();
  out = a * b;
  std::cout << a << " * " << b << " = " << out << std::endl;
}

// Same as before but now we have an output parameter
int multiply_return(const int a, const int b) {
  simulate_hard_computation();
  const int res = a * b;
  std::cout << a << " * " << b << " = " << res << std::endl;
  return res;
}

void example(int num) {
	ThreadPool pool(num);

	pool.init();

	// Submit multiplication table
	for (int i = 1; i < 3; i++) {
		for (int j = 1; j < 10; j++) {
			pool.submit(multiply, i, j);	// void
		}
	}

	int output_ref;
	auto future1 = pool.submit(multiply_output, std::ref(output_ref), 5, 6);

	// Wait for ... to finish
	future1.get();	// void
	std::cout << "Reference output: " << output_ref << std::endl;

	auto future2 = pool.submit(multiply_return, 5, 3);
	int res = future2.get();  // int
	std::cout << "Return output: " << res << std::endl;

	pool.shutdown();
}

int main(int argc, char* argv[])
{
	// example(num);
	for (int i = 0; i < argc; i++) std::cout << argv[i] << std::endl;
}