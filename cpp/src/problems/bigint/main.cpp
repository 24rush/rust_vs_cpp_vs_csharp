
#include <gtest/gtest.h>

#include "bigint.hpp"

TEST(BigInt, sign)
{
    BigInt a("123");
    EXPECT_FALSE(a.isNegative());

    BigInt b("-123");
    EXPECT_TRUE(b.isNegative());
}

TEST(BigInt, ctor)
{
    EXPECT_NO_THROW(BigInt("123"));
    EXPECT_NO_THROW(BigInt("-123"));
    EXPECT_NO_THROW(BigInt(""));

    EXPECT_THROW(BigInt("1w23"), std::runtime_error);
}

TEST(BigInt, print)
{
    EXPECT_EQ(BigInt("123").print(), "123");
}

TEST(BigInt, multiply)
{
    BigInt a("1");
    BigInt b("20");
    BigInt c("-20");

    EXPECT_EQ((a * b).print(), "20");
    EXPECT_EQ((a * c).print(), "-20");

    BigInt d("134234");
    BigInt e("4658791");

    EXPECT_EQ((d * e).print(), "625368151094");

    BigInt f("0");
    BigInt g("4658791");

    EXPECT_EQ((f * g).print(), "0");
}