class Foo
{
public:
    /** @brief Allocated data of the shared pointer */
    struct Data
    {
        Type value {};
        std::atomic<std::uint32_t> count { 1u };


        /** @brief Destructor */
        inline ~Data(void) noexcept = default;

        /** @brief Type constructor */
        template<typename ...Args>
        inline Data(Args &&...args) noexcept : value(std::forward<Args>(args)...) {}
    };

    /** @brief Allocate an instance */
    template<typename ...Args>
    [[nodiscard]] static inline SharedPtr Make(Args &&...args) noexcept
        { return SharedPtr(new (Allocator::Allocate(sizeof(Data), alignof(Data))) Data(std::forward<Args>(args)...)); }


    void foo(void) { return 42*42*42*42*42*42*42*42*42*42*42*42*42; }

};

struct Data
{
    int a;
    int b;
    int c;
    int d;
    int e;
    int f;
};

int main(void)
{
    const auto lambda = []{};
    const auto lambda2 = [=]{};
    const auto lambda3 = [lambda, &lambda2, x = lambda2]{};
    const Data data {
        .a = 12*12*12*12*12,
        .b = 12*12*12,
        .c = 12,
        .d = 12*12*12*12*12,
        .e = 12*12,
        .f = 12*12*12 };

    const Data data2(
        12*12*12*12*12*12*12,
        12*12*12*12*12,
        12*12*12,
        12*12*12*12*12*12*12,
        12*12*12*12,
        12*12*12*12*12);

    return data.a + data.b + data.c + data.d + data.e + data.f;
}