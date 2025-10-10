#include <gtest/gtest.h>

#include "uncertain_value.hpp"

TEST(UnvertainValueTest, Exact) {
	uncertain_value a(1.0, 0.0);
	EXPECT_TRUE(a.exact());
}

TEST(UnvertainValueTest, NotExact) {
	uncertain_value a(1.0, 2.0);
	EXPECT_FALSE(a.exact());
}

TEST(UnvertainValueTest, DoubleConversionNotExact) {
	uncertain_value a(1.0);
	EXPECT_FALSE(a.exact());
}

TEST(UnvertainValueTest, ContainsZero) {
	uncertain_value a(-1.0, 2.0);
	EXPECT_TRUE(a.contains_zero());
}

TEST(UnvertainValueTest, DoesNotContainZero) {
	uncertain_value a(-2.0, 1.0);
	EXPECT_FALSE(a.contains_zero());
}

TEST(UnvertainValueTest, Addition) {
	uncertain_value a(1.0, 2.0);
	uncertain_value b(3.0, 4.0);
	uncertain_value c = a + b;
	EXPECT_EQ(c, uncertain_value(4.0, 6.0));
}

TEST(UnvertainValueTest, AdditionInPlace) {
	uncertain_value a(1.0, 2.0);
	uncertain_value b(3.0, 4.0);
	a += b;
	EXPECT_EQ(a, uncertain_value(4.0, 6.0));
}

TEST(UnvertainValueTest, Subtraction) {
	uncertain_value a(1.0, 3.0);
	uncertain_value b(2.0, 5.0);
	uncertain_value c = a - b;
	EXPECT_EQ(c, uncertain_value(-1.0, 2.0));
}

TEST(UnvertainValueTest, SubtractionInPlace) {
	uncertain_value a(1.0, 3.0);
	uncertain_value b(2.0, 5.0);
	a -= b;
	EXPECT_EQ(a, uncertain_value(-1.0, 2.0));
}

TEST(UnvertainValueTest, Multiplication) {
	uncertain_value a(1.0, 3.0);
	uncertain_value b(2.0, 5.0);
	uncertain_value c = a * b;
	EXPECT_EQ(c, uncertain_value(2.0, 26.0));
}

TEST(UnvertainValueTest, MultiplicationInPlace) {
	uncertain_value a(1.0, 3.0);
	uncertain_value b(2.0, 5.0);
	a *= b;
	EXPECT_EQ(a, uncertain_value(2.0, 26.0));
}

TEST(UnvertainValueTest, Division) {
	uncertain_value a(10.0, 0.5);
	uncertain_value b(2.0, 0.0625);
	uncertain_value c = a / b;
	EXPECT_EQ(c, uncertain_value(5.0, 0.4140625));
}

TEST(UnvertainValueTest, DivisionInPlace) {
	uncertain_value a(10.0, 0.5);
	uncertain_value b(2.0, 0.0625);
	a /= b;
	EXPECT_EQ(a, uncertain_value(5.0, 0.4140625));
}

TEST(UnvertainValueTest, AbsoluteValue) {
	EXPECT_EQ(std::abs(uncertain_value(2.0, 1.0)), uncertain_value(2.0, 1.0));
	EXPECT_EQ(std::abs(uncertain_value(-2.0, 1.0)), uncertain_value(2.0, 1.0));
}
