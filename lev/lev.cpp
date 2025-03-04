#include <algorithm>
#include <chrono>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <vector>

uint32_t lev_recursive(const std::string& a, const std::string& b) {
	const uint32_t n = a.length();
	const uint32_t m = b.length();
	if (n == 0) {
		return m;
	}
	if (m == 0) {
		return n;
	}
	if (a[0] == b[0]) {
		return lev_recursive(a.substr(1), b.substr(1));
	}

	return 1 + std::min(std::min(lev_recursive(a.substr(1), b),
								 lev_recursive(a, b.substr(1))),
						lev_recursive(a.substr(1), b.substr(1)));
}

uint32_t lev_matrix(const std::string& a, const std::string& b) {
	const uint32_t n = a.length();
	const uint32_t m = b.length();
	if (n == 0) {
		return m;
	}
	if (m == 0) {
		return n;
	}
	std::vector<std::vector<uint32_t>> d(n + 1, std::vector<uint32_t>(m + 1));
	for (uint32_t i{1}; i <= n; i++) {
		d[i][0] = i;
	}
	for (uint32_t i{1}; i <= m; i++) {
		d[0][i] = i;
	}
	for (uint32_t i{1}; i <= n; i++) {
		for (uint32_t j{1}; j <= m; j++) {
			const uint32_t cost = a[i - 1] == b[j - 1] ? 0 : 1;
			d[i][j] = std::min(std::min(d[i - 1][j] + 1, d[i][j - 1] + 1),
							   d[i - 1][j - 1] + cost);
		}
	}
	return d.at(n).at(m);
}

uint32_t lev_vectors(const std::string& a, const std::string& b) {
	const uint32_t n = a.length();
	const uint32_t m = b.length();
	if (n == 0) {
		return m;
	}
	if (m == 0) {
		return n;
	}
	std::vector<uint32_t> v0(m + 1);
	std::vector<uint32_t> v1(m + 1);
	for (uint32_t i{0}; i <= m; i++) {
		v0[i] = i;
	}
	for (uint32_t i{1}; i <= n; i++) {
		v1[0] = i;
		for (uint32_t j{1}; j <= m; j++) {
			const uint32_t cost = a[i - 1] == b[j - 1] ? 0 : 1;
			v1[j] =
				std::min(std::min(v1[j - 1] + 1, v0[j] + 1), v0[j - 1] + cost);
		}
		// Move v1 into v0
		std::copy(v1.begin(), v1.end(), v0.begin());
	}
	return v1.at(m);
}

int main(int argc, const char** argv) {
	if (argc != 2) {
		std::cerr << "Error: expected filename." << std::endl;
		return -1;
	}

	std::string filename(argv[1]);
	std::ifstream infile(filename);
	std::string line;
	std::vector<std::string> lines;
	while (std::getline(infile, line)) {
		// Ignore empty and blank strings
		if (line.length() == 0 ||
			std::all_of(line.begin(), line.end(),
						[](const char ch) { return std::isspace(ch); })) {
			continue;
		}
		lines.push_back(line);
	}

	const uint32_t n = lines.size();
	std::cout << "Read " << n << " lines." << std::endl;
	std::cout << std::endl;

	uint32_t min_dist = ~0;
	uint32_t a = 0;
	uint32_t b = 0;
	for (uint32_t i{0}; i < n; i++) {
		for (uint32_t j{i + 1}; j < n; j++) {
			const uint32_t dist = lev_vectors(lines[i], lines[j]);
			if (dist < min_dist) {
				min_dist = dist;
				a = i;
				b = j;
			}
		}
	}

	std::cout << "Most similar lines (distance = " << min_dist
			  << "):" << std::endl;
	std::cout << (a + 1) << " : " << lines[a] << std::endl;
	std::cout << (b + 1) << " : " << lines[b] << std::endl;
	std::cout << std::endl;

	return 0;
}
