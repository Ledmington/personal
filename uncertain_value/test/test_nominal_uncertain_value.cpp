#include <uncertain_value/nominal_uncertain_value.hpp>

#include <gtest/gtest.h>

#include <random>

TEST(NominalUncertainValueTest, Exact) {
	nominal_uncertain_value a(1.0, 0.0);
	EXPECT_TRUE(a.exact());
}

TEST(NominalUncertainValueTest, NotExact) {
	nominal_uncertain_value a(1.0, 2.0);
	EXPECT_FALSE(a.exact());
}

TEST(NominalUncertainValueTest, DoubleConversionNotExact) {
	nominal_uncertain_value a(1.0);
	EXPECT_FALSE(a.exact());
}

TEST(NominalUncertainValueTest, LowerUpper) {
	nominal_uncertain_value a(-1.0, 2.0);
	EXPECT_EQ(a.lower(), -3.0);
	EXPECT_EQ(a.upper(), 1.0);
}

TEST(NominalUncertainValueTest, ContainsZero) {
	nominal_uncertain_value a(-1.0, 2.0);
	EXPECT_TRUE(a.contains_zero());
}

TEST(NominalUncertainValueTest, DoesNotContainZero) {
	nominal_uncertain_value a(-2.0, 1.0);
	EXPECT_FALSE(a.contains_zero());
}

TEST(NominalUncertainValueTest, Addition) {
	nominal_uncertain_value a(1.0, 2.0);
	nominal_uncertain_value b(3.0, 4.0);
	nominal_uncertain_value c = a + b;
	EXPECT_EQ(c, nominal_uncertain_value(4.0, 6.0));
}

TEST(NominalUncertainValueTest, AdditionInPlace) {
	nominal_uncertain_value a(1.0, 2.0);
	nominal_uncertain_value b(3.0, 4.0);
	a += b;
	EXPECT_EQ(a, nominal_uncertain_value(4.0, 6.0));
}

TEST(NominalUncertainValueTest, AdditionWithDouble) {
	nominal_uncertain_value a(1.0, 2.0);
	nominal_uncertain_value c = a + 1.5;
	EXPECT_EQ(c, nominal_uncertain_value(2.5, 2.0));
}

TEST(NominalUncertainValueTest, AdditionWithDoubleInPlace) {
	nominal_uncertain_value a(1.0, 2.0);
	a += 1.5;
	EXPECT_EQ(a, nominal_uncertain_value(2.5, 2.0));
}

TEST(NominalUncertainValueTest, Subtraction) {
	nominal_uncertain_value a(1.0, 3.0);
	nominal_uncertain_value b(2.0, 5.0);
	nominal_uncertain_value c = a - b;
	EXPECT_EQ(c, nominal_uncertain_value(-1.0, 2.0));
}

TEST(NominalUncertainValueTest, SubtractionInPlace) {
	nominal_uncertain_value a(1.0, 3.0);
	nominal_uncertain_value b(2.0, 5.0);
	a -= b;
	EXPECT_EQ(a, nominal_uncertain_value(-1.0, 2.0));
}

TEST(NominalUncertainValueTest, SubtractionWithDouble) {
	nominal_uncertain_value a(1.0, 3.0);
	nominal_uncertain_value c = a - 1.5;
	EXPECT_EQ(c, nominal_uncertain_value(-0.5, 3.0));
}

TEST(NominalUncertainValueTest, SubtractionWithDoubleInPlace) {
	nominal_uncertain_value a(1.0, 3.0);
	a -= 1.5;
	EXPECT_EQ(a, nominal_uncertain_value(-0.5, 3.0));
}

TEST(NominalUncertainValueTest, Multiplication) {
	nominal_uncertain_value a(1.0, 3.0);
	nominal_uncertain_value b(2.0, 5.0);
	nominal_uncertain_value c = a * b;
	EXPECT_EQ(c, nominal_uncertain_value(2.0, 26.0));
}

TEST(NominalUncertainValueTest, MultiplicationInPlace) {
	nominal_uncertain_value a(1.0, 3.0);
	nominal_uncertain_value b(2.0, 5.0);
	a *= b;
	EXPECT_EQ(a, nominal_uncertain_value(2.0, 26.0));
}

TEST(NominalUncertainValueTest, MultiplicationWithDouble) {
	nominal_uncertain_value a(1.0, 3.0);
	nominal_uncertain_value c = a * 2.0;
	EXPECT_EQ(c, nominal_uncertain_value(2.0, 3.0));
}

TEST(NominalUncertainValueTest, MultiplicationWithDoubleInPlace) {
	nominal_uncertain_value a(1.0, 3.0);
	a *= 2.0;
	EXPECT_EQ(a, nominal_uncertain_value(2.0, 3.0));
}

TEST(NominalUncertainValueTest, Division) {
	nominal_uncertain_value a(10.0, 0.5);
	nominal_uncertain_value b(2.0, 0.0625);
	nominal_uncertain_value c = a / b;
	EXPECT_EQ(c, nominal_uncertain_value(5.0, 0.4140625));
}

TEST(NominalUncertainValueTest, DivisionInPlace) {
	nominal_uncertain_value a(10.0, 0.5);
	nominal_uncertain_value b(2.0, 0.0625);
	a /= b;
	EXPECT_EQ(a, nominal_uncertain_value(5.0, 0.4140625));
}

TEST(NominalUncertainValueTest, DivisionWithDouble) {
	nominal_uncertain_value a(10.0, 0.5);
	nominal_uncertain_value c = a / 2.0;
	EXPECT_EQ(c, nominal_uncertain_value(5.0, 0.5));
}

TEST(NominalUncertainValueTest, DivisionWithDoubleInPlace) {
	nominal_uncertain_value a(10.0, 0.5);
	a /= 2.0;
	EXPECT_EQ(a, nominal_uncertain_value(5.0, 0.5));
}

TEST(NominalUncertainValueTest, Negation) {
	EXPECT_EQ(-nominal_uncertain_value(1.0, 2.0),
			  nominal_uncertain_value(-1.0, 2.0));
	EXPECT_EQ(-nominal_uncertain_value(-1.0, 2.0),
			  nominal_uncertain_value(1.0, 2.0));
}

TEST(NominalUncertainValueTest, AbsoluteValue) {
	EXPECT_EQ(std::abs(nominal_uncertain_value(2.0, 1.0)),
			  nominal_uncertain_value(2.0, 1.0));
	EXPECT_EQ(std::abs(nominal_uncertain_value(-2.0, 1.0)),
			  nominal_uncertain_value(2.0, 1.0));
	EXPECT_EQ(std::abs(nominal_uncertain_value(1.0, 2.0)),
			  nominal_uncertain_value(1.0, 2.0));
	EXPECT_EQ(std::abs(nominal_uncertain_value(-1.0, 2.0)),
			  nominal_uncertain_value(1.0, 2.0));
}

TEST(NominalUncertainValueTest, SquareRoot) {
	EXPECT_EQ(std::sqrt(nominal_uncertain_value(4.0, 1.0)),
			  nominal_uncertain_value(2.0, 1.0));
	EXPECT_EQ(std::sqrt(nominal_uncertain_value(1.0, 2.0)),
			  nominal_uncertain_value(1.0, 2.0));
}

TEST(NominalUncertainValueTest, IsFinite) {
	EXPECT_TRUE(std::isfinite(nominal_uncertain_value(4.0, 1.0)));
}

TEST(NominalUncertainValueTest, Printable) {
	nominal_uncertain_value x{-1.5, 0.5};
	std::ostringstream oss;
	oss << x;
	EXPECT_EQ(oss.str(), "-1.5±0.5");
}

TEST(NominalUncertainValueTest, IsFloatingPointType) {
	EXPECT_TRUE(std::is_floating_point<nominal_uncertain_value>::value);
	EXPECT_TRUE(std::is_floating_point_v<nominal_uncertain_value>);
}

TEST(NominalUncertainValueTest, RandomNumberGeneration) {
	std::mt19937 rnd{42};
	std::uniform_real_distribution<nominal_uncertain_value> dist{-1.0, 1.0};

	const nominal_uncertain_value x = dist(rnd);

	EXPECT_EQ(x.error(), 0.0);
	EXPECT_TRUE(x.value() >= -1.0);
	EXPECT_TRUE(x.value() <= 1.0);
}

TEST(NominalUncertainValueTest, ComparisonGreater) {
	EXPECT_TRUE(nominal_uncertain_value(1.0, 0.5) > 0);
	EXPECT_TRUE(nominal_uncertain_value(1.0, 0.5) > 0.0f);
	EXPECT_TRUE(nominal_uncertain_value(1.0, 0.5) > 0.0);
}

TEST(NominalUncertainValueTest, ComparisonGreaterOrEqual) {
	EXPECT_TRUE(nominal_uncertain_value(1.0, 0.5) >= 0);
	EXPECT_TRUE(nominal_uncertain_value(1.0, 0.5) >= 0.0f);
	EXPECT_TRUE(nominal_uncertain_value(1.0, 0.5) >= 0.0);
}

TEST(NominalUncertainValueTest, ComparisonLess) {
	EXPECT_TRUE(nominal_uncertain_value(-1.0, 0.5) < 0);
	EXPECT_TRUE(nominal_uncertain_value(-1.0, 0.5) < 0.0f);
	EXPECT_TRUE(nominal_uncertain_value(-1.0, 0.5) < 0.0);
}

TEST(NominalUncertainValueTest, ComparisonLessOrEqual) {
	EXPECT_TRUE(nominal_uncertain_value(-1.0, 0.5) <= 0);
	EXPECT_TRUE(nominal_uncertain_value(-1.0, 0.5) <= 0.0f);
	EXPECT_TRUE(nominal_uncertain_value(-1.0, 0.5) <= 0.0);
}

TEST(NominalUncertainValueTest, MultiplicationInfinityCheck) {
	nominal_uncertain_value x(1.0, 1e180);
	EXPECT_THROW({ x *= x; }, std::runtime_error);
}

TEST(NominalUncertainValueTest, DivisionInfinityCheck) {
	nominal_uncertain_value x(1.0, 1e180);
	EXPECT_THROW({ x /= x; }, std::runtime_error);
}

TEST(NominalUncertainValueTest, AdditionInfinityCheck) {
	nominal_uncertain_value x(1.0, std::numeric_limits<double>::max());
	EXPECT_THROW({ x += x; }, std::runtime_error);
}
