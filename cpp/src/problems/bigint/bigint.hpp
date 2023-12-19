#include <string>
#include <vector>

#include <exception>

using namespace std;

class BigInt
{
private:
    // true - negative false - positive
    bool sign = false;

    // reversed order of digis
    // e.g. 123 -> 321
    std::vector<char> chars;

    BigInt(size_t capacity) : sign(false), chars(capacity)
    {
        chars.assign(capacity, 0);
    }

    bool isZero() const { return this->chars.size() == 1 && this->chars.front() == 0; }

public:
    BigInt(BigInt &&other) : sign(other.sign), chars(std::move(other.chars))
    {
    }

    BigInt(std::string_view s) : sign{s.size() && s[0] == '-'}, chars(std::max(s.size(), (size_t)1) + (sign ? -1 : 0))
    {
        if (!s.size())
        {
            return;
        }

        if (s[0] == '-')
        {
            this->sign = true;
            s.remove_prefix(1);
        }

        auto ri = this->chars.capacity() - 1;
        for (auto digit : s)
        {
            if (isdigit(digit))
            {
                this->chars[ri--] = digit - '0';
            }
            else
            {
                throw std::runtime_error("Invalid input");
            }
        }
    }

    BigInt operator*(const BigInt &other)
    {
        if (this->isZero() || other.isZero())
        {
            return BigInt("");
        }

        // result = this * other
        BigInt result(this->chars.size() + other.chars.size());

        int dio = 0, dit = 0;
        for (dio = 0; dio < other.chars.size(); dio++)
        {
            int digito = other.chars[dio];
            if (digito == 0)
                continue;

            int carry = 0;
            for (dit = 0; dit < this->chars.size() || carry; dit++)
            {
                int digitt = this->chars[dit];
                int digit = result.chars[dio + dit] + (dit < this->chars.size() ? digito * digitt : 0) + carry;

                result.chars[dio + dit] = digit % 10;
                carry = digit / 10;
            }
        }

        result.chars.resize(dio + dit - 1);
        result.sign = this->sign ^ other.sign;

        return result;
    }

    bool isNegative() const { return this->sign == true; }

    std::string print() const
    {
        std::string result;

        if (this->sign)
        {
            result += '-';
        }

        for (int i = this->chars.size() - 1; i >= 0; i--)
            result += this->chars[i] + '0';

        std::cout << result;

        if (this->chars.size())
            std::cout << std::endl;

        return result;
    }
};
