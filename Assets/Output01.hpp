#define mut __attribute__((annotate("kpp-mut")))

namespace Core
{
    struct Point
    {
        float x {};
        float y {};

        [[nodiscard]] Point compute(mut float value) const noexcept
        {
            ++value;
            mut Point p { x + value, y + value };
            ++p.x;
            return p;
        }
    };

    [[nodiscard]] Point Compute(const Point &lhs, const float rhs) noexcept
    {
        return lhs.compute(rhs);
    }

    constexpr auto LambdaNoArgs = [] noexcept
    {
        return std::max(42, 24);
    };

    constexpr auto LambdaArgs = [](const auto arg) noexcept
    {
        return arg;
    };
}

[[nodiscard]] int main(void) noexcept
{
    return 0;
}
