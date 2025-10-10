#include "uncertain_value.hpp"

#include <cassert>
#include <cmath>
#include <iomanip>
#include <ostream>

uncertain_value::uncertain_value(const double value, const double error)
	: value_(value), error_(std::abs(error)) {
	assert(std::isfinite(value));
	assert(std::isfinite(error));
}

uncertain_value::uncertain_value(const double value)
	: value_(value),
	  error_(std::abs(value) * std::numeric_limits<double>::epsilon()) {}

[[nodiscard]] uncertain_value operator+(const uncertain_value &a,
										const uncertain_value &b) {
	return uncertain_value(a.value_ + b.value_, a.error_ + b.error_);
}

uncertain_value &uncertain_value::operator+=(const uncertain_value &other) {
	this->value_ += other.value_;
	this->error_ += other.error_;
	return *this;
}

[[nodiscard]] uncertain_value operator-(const uncertain_value &a,
										const uncertain_value &b) {
	return uncertain_value(a.value_ - b.value_, a.error_ - b.error_);
}

uncertain_value &uncertain_value::operator-=(const uncertain_value &other) {
	this->value_ -= other.value_;
	this->error_ = std::abs(this->error_ - other.error_);
	return *this;
}

[[nodiscard]] uncertain_value operator*(const uncertain_value &a,
										const uncertain_value &b) {
	return uncertain_value(a.value_ * b.value_,
						   std::abs(a.value_) * b.error_ +
							   std::abs(b.value_) * a.error_ +
							   a.error_ * b.error_);
}

uncertain_value &uncertain_value::operator*=(const uncertain_value &other) {
	const double new_value = this->value_ * other.value_;
	const double new_error = std::abs(this->value_) * other.error_ +
							 std::abs(other.value_) * this->error_ +
							 this->error_ * other.error_;
	this->value_ = new_value;
	this->error_ = new_error;
	return *this;
}

[[nodiscard]] uncertain_value operator/(const uncertain_value &a,
										const uncertain_value &b) {
	assert(!b.contains_zero());
	return uncertain_value(
		a.value_ / b.value_,
		(a.error_ / std::abs(b.value_)) +
			((std::abs(a.value_) * b.error_) / (b.value_ * b.value_)) +
			((a.error_ * b.error_) / (b.value_ * b.value_)));
}

uncertain_value &uncertain_value::operator/=(const uncertain_value &other) {
	assert(!other.contains_zero());
	const double new_value = this->value_ / other.value_;
	const double new_error =
		(this->error_ / std::abs(other.value_)) +
		((std::abs(this->value_) * other.error_) /
		 (other.value_ * other.value_)) +
		((this->error_ * other.error_) / (other.value_ * other.value_));
	this->value_ = new_value;
	this->error_ = new_error;
	return *this;
}

bool operator==(const uncertain_value &a, const uncertain_value &b) {
	return a.value_ == b.value_ && a.error_ == b.error_;
}

bool uncertain_value::exact() const {
	return error_ == 0.0;
}

double uncertain_value::lower() const {
	return value_ - error_;
}

double uncertain_value::upper() const {
	return value_ - error_;
}

bool uncertain_value::contains_zero() const {
	return error_ >= std::abs(value_);
}

[[nodiscard]] uncertain_value uncertain_value::absolute_value() const {
	return uncertain_value(std::abs(value_), error_);
}

std::ostream &operator<<(std::ostream &os, const uncertain_value &x) {
	os << std::setprecision(20) << x.value_ << "±" << x.error_;
	return os;
}

namespace std {

uncertain_value abs(const uncertain_value &val) {
	return val.absolute_value();
}

}  // namespace std
