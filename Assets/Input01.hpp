#define mut __attribute__((annotate("kpp-mut")))

#include <algorithm>

namespace Core
{
    struct Point
    {
        float x {};
        float y {};

        Point compute(mut float value)
        {
            ++value;
            mut Point p { x + value, y + value };
            ++p.x;
            return p;
        }
    };

    [[nodiscard]] Point Compute(Point &lhs, float rhs)
    {
        return lhs.compute(rhs);
    }

    constexpr auto LambdaNoArgs = []
    {
        return std::max(42, 24);
    };

    constexpr auto LambdaArgs = [](auto arg)
    {
        return arg;
    };
}

int main(void)
{
    return 0;
}