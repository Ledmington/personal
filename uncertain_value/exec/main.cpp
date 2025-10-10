#include <iomanip>
#include <iostream>
#include <vector>

#include "uncertain_value.hpp"

template <typename T>
std::ostream& operator<<(std::ostream& os, const std::vector<T>& v) {
	os << "[";
	if (!v.empty()) {
		os << v[0];
		for (auto i = 1; i < v.size(); i++) {
			os << ", " << v[i];
		}
	}
	os << "]";
	return os;
}

template <typename T>
T sum(const std::vector<T>& v) {
	T s(0);
	for (auto i = 0; i < v.size(); i++) {
		s += v[i];
	}
	return s;
}

template <typename T>
T stable_sum(const std::vector<T>& v) {
	// Kahan algorithm

	T s = 0;
	T c = 0;
	for (auto x : v) {
		const T y = x - c;
		const T t = s + y;
		c = (t - s) - y;
		s = t;
	}
	return s;
}

void vector_sum() {
	const int N = 1000000;
	std::vector<uncertain_value> v(N, 1e-8);

	// std::cout << "Input: " << v << "\n";
	std::cout << "Standard sum: " << std::setprecision(20) << sum(v) << "\n";
	std::cout << "Stable sum  : " << std::setprecision(20) << stable_sum(v)
			  << "\n";
	std::cout << "Expected    : " << N * 1e-8 << "\n";
}

int main() {
	vector_sum();

	return 0;
}
