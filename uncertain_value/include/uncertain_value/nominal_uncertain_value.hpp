#pragma once

#include <ostream>
#include <random>

class nominal_uncertain_value {
   public:
	nominal_uncertain_value(double value, double error);
	nominal_uncertain_value(double value);
	nominal_uncertain_value();

	nominal_uncertain_value operator-() const;

	// with nominal_uncertain_value
	friend nominal_uncertain_value operator+(const nominal_uncertain_value &a,
											 const nominal_uncertain_value &b);
	nominal_uncertain_value &operator+=(const nominal_uncertain_value &other);
	friend nominal_uncertain_value operator-(const nominal_uncertain_value &a,
											 const nominal_uncertain_value &b);
	nominal_uncertain_value &operator-=(const nominal_uncertain_value &other);
	friend nominal_uncertain_value operator*(const nominal_uncertain_value &a,
											 const nominal_uncertain_value &b);
	nominal_uncertain_value &operator*=(const nominal_uncertain_value &other);
	friend nominal_uncertain_value operator/(const nominal_uncertain_value &a,
											 const nominal_uncertain_value &b);
	nominal_uncertain_value &operator/=(const nominal_uncertain_value &other);

	// with double
	friend nominal_uncertain_value operator+(const nominal_uncertain_value &a,
											 const double &b);
	nominal_uncertain_value &operator+=(const double &other);
	friend nominal_uncertain_value operator-(const nominal_uncertain_value &a,
											 const double &b);
	nominal_uncertain_value &operator-=(const double &other);
	friend nominal_uncertain_value operator*(const nominal_uncertain_value &a,
											 const double &b);
	nominal_uncertain_value &operator*=(const double &other);
	friend nominal_uncertain_value operator/(const nominal_uncertain_value &a,
											 const double &b);
	nominal_uncertain_value &operator/=(const double &other);

	friend bool operator==(const nominal_uncertain_value &a,
						   const nominal_uncertain_value &b);
	friend bool operator<(const nominal_uncertain_value &a,
						  const nominal_uncertain_value &b);
	friend bool operator>(const nominal_uncertain_value &a,
						  const nominal_uncertain_value &b);
	friend bool operator<=(const nominal_uncertain_value &a,
						   const nominal_uncertain_value &b);
	friend bool operator>=(const nominal_uncertain_value &a,
						   const nominal_uncertain_value &b);

	friend std::ostream &operator<<(std::ostream &os,
									const nominal_uncertain_value &x);

	nominal_uncertain_value abs() const;
	nominal_uncertain_value sqrt() const;

	bool exact() const;
	double lower() const;
	double upper() const;
	double value() const;
	double error() const;
	bool contains_zero() const;

   private:
	double value_;
	double error_;
};

namespace std {

template <>
struct is_floating_point<nominal_uncertain_value> : std::true_type {};

nominal_uncertain_value abs(const nominal_uncertain_value &val);
nominal_uncertain_value sqrt(const nominal_uncertain_value &val);

bool isfinite(const nominal_uncertain_value &);

template <>
class uniform_real_distribution<nominal_uncertain_value> {
   public:
	uniform_real_distribution(const double &low, const double &high)
		: dist_(low, high) {}

	nominal_uncertain_value operator()(std::mt19937 &rnd) {
		return nominal_uncertain_value(dist_(rnd), 0.0);
	}

   private:
	std::uniform_real_distribution<double> dist_;
};

}  // namespace std
