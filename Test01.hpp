// Main entry point
[[nodiscard]] int main(void) noexcept
{
    { // Return from main
        foo();
        return (0 + 1) * 2;
    }
}
