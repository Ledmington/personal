#pragma once

#include <string>
#include <vector>
#include <random>

template <typename T>
std::string get_string(const std::vector<T>& v) {
	std::ostringstream oss;
	oss << '[';
	if (v.size() > 0) {
		oss << v.at(0);
		for (size_t i{1}; i < v.size(); i++) {
			oss << ", " << v.at(i);
		}
	}
	oss << ']';
	return oss.str();
}

template <typename T>
bool contains(const std::vector<T>& v, const T& x) {
	for (size_t i{0}; i < v.size(); i++) {
		if (v.at(i) == x) {
			return true;
		}
	}
	return false;
}

template <typename T>
void remove_from(std::vector<T>& v, const T c) {
	const auto it = std::find(v.begin(), v.end(), c);
#ifndef NDEBUG
	const size_t count = std::count(v.begin(), v.end(), c);
#endif
	assert(count > 0);
	v.erase(it);
	assert(static_cast<size_t>(std::count(v.begin(), v.end(), c)) == count - 1);
}

template <typename T>
void remove_from(std::vector<T>& v, const size_t index) {
	assert(index < v.size());
	v.erase(v.begin() + index);
}

template <typename T>
T choose_random(std::mt19937& rnd, const std::vector<T>& v) {
	assert(v.size() > 0);

	if (v.size() == 1) {
		return v.at(0);
	}

	std::uniform_int_distribution<size_t> dist{0, v.size() - 1};
	return v.at(dist(rnd));
}

template <typename T>
size_t count(const std::vector<T>& v, const T& c) {
	size_t count = 0;
	for (size_t i{0}; i < v.size(); i++) {
		if (v.at(i) == c) {
			count++;
		}
	}
	return count;
}
