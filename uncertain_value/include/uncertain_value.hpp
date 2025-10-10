#pragma once

#include <ostream>

class uncertain_value {
   public:
	uncertain_value(double value, double error);
	uncertain_value(double value);

	friend uncertain_value operator+(const uncertain_value &a,
									 const uncertain_value &b);
	uncertain_value &operator+=(const uncertain_value &other);
	friend uncertain_value operator-(const uncertain_value &a,
									 const uncertain_value &b);
	uncertain_value &operator-=(const uncertain_value &other);
	friend uncertain_value operator*(const uncertain_value &a,
									 const uncertain_value &b);
	uncertain_value &operator*=(const uncertain_value &other);
	friend uncertain_value operator/(const uncertain_value &a,
									 const uncertain_value &b);
	uncertain_value &operator/=(const uncertain_value &other);
	friend bool operator==(const uncertain_value &a, const uncertain_value &b);
	friend std::ostream &operator<<(std::ostream &os, const uncertain_value &x);

	uncertain_value absolute_value() const;

	bool exact() const;
	double lower() const;
	double upper() const;
	bool contains_zero() const;

   private:
	double value_;
	double error_;
};

namespace std {

uncertain_value abs(const uncertain_value &val);

}
