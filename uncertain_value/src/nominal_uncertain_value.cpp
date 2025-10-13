#include <uncertain_value/nominal_uncertain_value.hpp>

#include <cassert>
#include <cmath>
#include <iomanip>
#include <iostream>

#define UV_ASSERT(expr)                                                   \
	do {                                                                  \
		if (!(expr)) {                                                    \
			throw std::runtime_error(std::string("UV_ASSERT failed: (") + \
									 #expr + "), function " + __func__ +  \
									 ", file " + __FILE__ + ", line " +   \
									 std::to_string(__LINE__));           \
		}                                                                 \
	} while (false)

nominal_uncertain_value::nominal_uncertain_value(const double value,
												 const double error)
	: value_(value), error_(std::abs(error)) {
	UV_ASSERT(std::isfinite(value));
	UV_ASSERT(std::isfinite(error));
}

nominal_uncertain_value::nominal_uncertain_value(const double value)
	: value_(value),
	  error_(std::abs(value) * std::numeric_limits<double>::epsilon()) {}

nominal_uncertain_value::nominal_uncertain_value() : value_(), error_() {}

nominal_uncertain_value nominal_uncertain_value::operator-() const {
	return nominal_uncertain_value(-value_, error_);
}

[[nodiscard]] nominal_uncertain_value operator+(
	const nominal_uncertain_value &a, const nominal_uncertain_value &b) {
	return nominal_uncertain_value(a.value_ + b.value_, a.error_ + b.error_);
}

nominal_uncertain_value &nominal_uncertain_value::operator+=(
	const nominal_uncertain_value &other) {
	this->value_ += other.value_;
	this->error_ += other.error_;
	UV_ASSERT(std::isfinite(this->value_));
	UV_ASSERT(std::isfinite(this->error_));
	return *this;
}

[[nodiscard]] nominal_uncertain_value operator-(
	const nominal_uncertain_value &a, const nominal_uncertain_value &b) {
	return nominal_uncertain_value(a.value_ - b.value_, a.error_ - b.error_);
}

nominal_uncertain_value &nominal_uncertain_value::operator-=(
	const nominal_uncertain_value &other) {
	this->value_ -= other.value_;
	this->error_ = std::abs(this->error_ - other.error_);
	UV_ASSERT(std::isfinite(this->value_));
	UV_ASSERT(std::isfinite(this->error_));
	return *this;
}

[[nodiscard]] nominal_uncertain_value operator*(
	const nominal_uncertain_value &a, const nominal_uncertain_value &b) {
	return nominal_uncertain_value(a.value_ * b.value_,
								   std::abs(a.value_) * b.error_ +
									   std::abs(b.value_) * a.error_ +
									   a.error_ * b.error_);
}

nominal_uncertain_value &nominal_uncertain_value::operator*=(
	const nominal_uncertain_value &other) {
	const double new_value = this->value_ * other.value_;
	const double new_error = std::abs(this->value_) * other.error_ +
							 std::abs(other.value_) * this->error_ +
							 this->error_ * other.error_;
	this->value_ = new_value;
	this->error_ = new_error;
	UV_ASSERT(std::isfinite(this->value_));
	UV_ASSERT(std::isfinite(this->error_));
	return *this;
}

[[nodiscard]] nominal_uncertain_value operator/(
	const nominal_uncertain_value &a, const nominal_uncertain_value &b) {
	// assert(!b.contains_zero());
	return nominal_uncertain_value(
		a.value_ / b.value_,
		(a.error_ / std::abs(b.value_)) +
			((std::abs(a.value_) * b.error_) / (b.value_ * b.value_)) +
			((a.error_ * b.error_) / (b.value_ * b.value_)));
}

nominal_uncertain_value &nominal_uncertain_value::operator/=(
	const nominal_uncertain_value &other) {
	// assert(!other.contains_zero());
	const double new_value = this->value_ / other.value_;
	const double new_error =
		(this->error_ / std::abs(other.value_)) +
		((std::abs(this->value_) * other.error_) /
		 (other.value_ * other.value_)) +
		((this->error_ * other.error_) / (other.value_ * other.value_));
	this->value_ = new_value;
	this->error_ = new_error;
	UV_ASSERT(std::isfinite(this->value_));
	UV_ASSERT(std::isfinite(this->error_));
	return *this;
}

[[nodiscard]] nominal_uncertain_value operator+(
	const nominal_uncertain_value &a, const double &b) {
	return nominal_uncertain_value(a.value_ + b, a.error_);
}

nominal_uncertain_value &nominal_uncertain_value::operator+=(
	const double &other) {
	this->value_ += other;
	return *this;
}

[[nodiscard]] nominal_uncertain_value operator-(
	const nominal_uncertain_value &a, const double &b) {
	return nominal_uncertain_value(a.value_ - b, a.error_);
}

nominal_uncertain_value &nominal_uncertain_value::operator-=(
	const double &other) {
	this->value_ -= other;
	return *this;
}

[[nodiscard]] nominal_uncertain_value operator*(
	const nominal_uncertain_value &a, const double &b) {
	return nominal_uncertain_value(a.value_ * b, a.error_);
}

nominal_uncertain_value &nominal_uncertain_value::operator*=(
	const double &other) {
	this->value_ *= other;
	return *this;
}

[[nodiscard]] nominal_uncertain_value operator/(
	const nominal_uncertain_value &a, const double &b) {
	return nominal_uncertain_value(a.value_ / b, a.error_);
}

nominal_uncertain_value &nominal_uncertain_value::operator/=(
	const double &other) {
	this->value_ /= other;
	return *this;
}

bool operator==(const nominal_uncertain_value &a,
				const nominal_uncertain_value &b) {
	return std::abs(a.value_ - b.value_) <= 0.0;
}

bool operator<(const nominal_uncertain_value &a,
			   const nominal_uncertain_value &b) {
	return a.value_ < b.value_;
}

bool operator>(const nominal_uncertain_value &a,
			   const nominal_uncertain_value &b) {
	return a.value_ > b.value_;
}

bool operator<=(const nominal_uncertain_value &a,
				const nominal_uncertain_value &b) {
	return a.value_ <= b.value_;
}

bool operator>=(const nominal_uncertain_value &a,
				const nominal_uncertain_value &b) {
	return a.value_ >= b.value_;
}

bool nominal_uncertain_value::exact() const {
	return std::abs(error_) <= 0.0;
}

double nominal_uncertain_value::lower() const {
	return value_ - error_;
}

double nominal_uncertain_value::upper() const {
	return value_ + error_;
}

double nominal_uncertain_value::value() const {
	return value_;
}

double nominal_uncertain_value::error() const {
	return error_;
}

bool nominal_uncertain_value::contains_zero() const {
	return error_ >= std::abs(value_);
}

[[nodiscard]] nominal_uncertain_value nominal_uncertain_value::abs() const {
	return nominal_uncertain_value(std::abs(value_), error_);
}

[[nodiscard]] nominal_uncertain_value nominal_uncertain_value::sqrt() const {
	assert(value_ >= 0.0);
	const double new_value = std::sqrt(value_);
	const double new_error = error_ / (2.0 * new_value);
	return nominal_uncertain_value(new_value, new_error);
}

std::ostream &operator<<(std::ostream &os, const nominal_uncertain_value &x) {
	os << x.value_ << "±" << x.error_;
	return os;
}

namespace std {

nominal_uncertain_value abs(const nominal_uncertain_value &val) {
	return val.abs();
}

nominal_uncertain_value sqrt(const nominal_uncertain_value &val) {
	return val.sqrt();
}

bool isfinite(const nominal_uncertain_value &) {
	return true;
}

}  // namespace std
