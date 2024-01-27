#include <algorithm>
#include <string>
#include <numeric>
#include <variant>

#include <gtest/gtest.h>

template <class... Ts>
struct overload : Ts...
{
    using Ts::operator()...;
};
template <class... Ts>
overload(Ts...) -> overload<Ts...>;

TEST(cpp17, var_expressions) {
    const std::vector<int> x{1, 2, 3};
    
    if (const auto it = std::find(x.begin(), x.end(), 2); true) {
        ASSERT_TRUE(it != x.end());
    }
}

TEST(cpp17, bindings) {
    std::map<int, std::string> dict{ {1, "one"}, {2, "two"}, {3, "buckle my shoe"}};
    ASSERT_TRUE(dict.find(1) != dict.end());

    auto [it, inserted] = dict.insert({4, "four"});
    ASSERT_EQ(inserted, true);
}

TEST(cpp17, variants)
{
    std::variant<int, double, std::string> v;

    // Default constructor sets everything to 0
    ASSERT_EQ(std::get<int>(v), 0);
    ASSERT_EQ(v.index(), 0);

    // Storing a double value moves index to 1
    v = 7.0;
    ASSERT_EQ(v.index(), 1);
    ASSERT_EQ(std::get<1>(v), 7.0);

    // Nothing stored at index 0 (int) as we just stored the double
    EXPECT_THROW(std::get<0>(v) == 7, std::bad_variant_access);

    // Storing an int moves index back to 0
    v = 8;
    ASSERT_EQ(std::get<0>(v), 8);
    ASSERT_EQ(v.index(), 0);

    // And a string move it to index 2
    v = "Alin";
    ASSERT_EQ(std::get<2>(v), "Alin");
    ASSERT_EQ(std::holds_alternative<std::string>(v), true);
    ASSERT_EQ(std::holds_alternative<int>(v), false);

    // Visitor pattern
    std::visit(overload{[](const int &i)
                        { std::cout << "int: " << i; },
                        [](const double &f)
                        { std::cout << "float: " << f; },
                        [](const std::string &s)
                        { std::cout << "string: " << s; }},
               v);
}

TEST(cpp17, optional)
{
    // std::optional manages an optional contained value, i.e. a value that may or may not be present.
    // https://en.cppreference.com/w/cpp/utility/optional

    std::optional<int> option;

    ASSERT_EQ(option.has_value(), false);
    ASSERT_EQ(option, std::nullopt);

    option = 5;
    ASSERT_EQ(option.value(), 5);

    // Setter using value()
    option.value() = 22.0;
    ASSERT_EQ(option.value(), 22);

    option.reset();
    ASSERT_EQ(option.value_or(5), 5);
}

TEST(cpp17, any)
{
    // Type-safe container for single values of any copy constructible type
    // https://en.cppreference.com/w/cpp/utility/any

    std::any any_obj = 1;
    std::cout << any_obj.type().name() << ": " << std::any_cast<int>(any_obj) << '\n';

    EXPECT_TRUE(any_obj.has_value());
    ASSERT_EQ(std::any_cast<int>(any_obj), 1);

    any_obj.reset();
    EXPECT_FALSE(any_obj.has_value());

    any_obj = "Alin";
    ASSERT_EQ(std::any_cast<const char *>(any_obj), "Alin");
}

TEST(cpp17, string_view)
{
    // A non-owning reference to a string
    // https://en.cppreference.com/w/cpp/string/basic_string_view
    std::wstring_view str{L" This 虎老 is a test\n"};

    ASSERT_EQ(str.back(), '\n');

    for (auto c : str)
        std::wcout << c;

    ASSERT_EQ(str.find(L"This"), 1);
    ASSERT_EQ(str.find_last_of(L"t"), str.length() - 2);

    EXPECT_EQ(str.find_first_of(L"虎"), 6);
    EXPECT_EQ(str.find_first_of(L"老"), 7);

    ASSERT_EQ(str.find_first_of(L"x"), std::string_view::npos);
}

TEST(cpp17, invoke)
{
    // Handling basic std::function
    {
        std::function<void(int)> func = [](int x)
        { std::cout << "std::function" << x << std::endl; };
        // same as func(5)
        std::invoke(func, 5);
    }

    // Handling methods
    {
        struct S
        {
            void do_something(int x) { std::cout << "do_smth " << x << " " << this->value << std::endl; }
            int value{0};
        };

        S s;
        // same as s.do_something(5) but invoked through object, reference and pointer
        std::invoke(&S::do_something, s, 5);
        std::invoke(&S::do_something, std::ref(s), 5);
        std::invoke(&S::do_something, &s, 6);

        S t;
        t.value = 8;
        std::invoke(&S::do_something, t, 9);

        // same as s.value = 42
        std::invoke(&S::value, s) = 42;
        ASSERT_EQ(s.value, 42);
        ASSERT_EQ(std::invoke(&S::value, s), 42);

        // same as "auto x = s.value;"
        auto x = std::invoke(&S::value, s);
    }

    // Handling non-static data members
    {
        struct X
        {
            std::function<void(int)> print_param;
        };

        X x;
        std::invoke(&X::print_param, x);

        x.print_param = [](int v)
        { std::cout << "Called with: " << v << std::endl; };

        // Does nothing, just return function
        std::invoke(&X::print_param, x);

        // Calls function
        std::invoke(&X::print_param, x)(4);
    }
}

TEST(cpp17, apply)
{
    std::apply([](auto &&...args)
               { ((std::cout << args << '\n'), ...); },
               std::make_tuple(42, 'a', 4.2));
}

TEST(cpp17, byte)
{
    std::byte a{0};
    std::byte b{0xF};
    ASSERT_EQ(std::to_integer<int>(b), 0xF);
    ASSERT_EQ(std::to_integer<int>(b << 1), 0xF * 2);

    std::byte c = a & b;
    ASSERT_EQ(std::to_integer<int>(c), 0);

    std::byte d = a | a;
    ASSERT_EQ(std::to_integer<int>(d), 0);
}

TEST(cpp17, maps)
{
    std::map<int, std::string> src{{1, "one"}, {2, "two"}, {3, "buckle my shoe"}};
    std::map<int, std::string> dst{{4, "three"}};

    // Remove from src insert in dest
    dst.insert(src.extract(1));
    ASSERT_EQ(src.find(1), src.end());
    ASSERT_NE(dst.find(1), src.end());

    dst.insert(src.extract(2));
    auto x = dst.extract(2);
    ASSERT_EQ(x.key(), 2);
    ASSERT_EQ(x.mapped(), "two");

    // Changing the key of the extracted node and insert it back
    x.key() = 4;
    src.insert(std::move(x));
    EXPECT_TRUE(src.find(4) != src.end());

    // Insert in dst expect new value
    dst.insert(std::move(x));
    EXPECT_TRUE(src.find(4)->second == "two");
}

TEST(cpp17, reduce)
{
    const std::array<int, 3> a{1, 2, 3};
    ASSERT_EQ(std::reduce(std::cbegin(a), std::cend(a)), 6);

    // Using a custom binary op:
    ASSERT_EQ(std::reduce(std::cbegin(a), std::cend(a), 0, std::plus<>{}), 6);

    ASSERT_EQ(std::transform_reduce(a.cbegin(), a.cend(), 0, std::plus{}, [](const int a)
                                    { return a * a; }),
              14);

    ASSERT_EQ(std::transform_reduce(a.cbegin(), a.cend(), 1, std::multiplies{}, [](const int a)
                                    { return a * a; }),
              36);
}