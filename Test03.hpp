[[nodiscard]] int main(void) noexcept(1 + 2 == 3) { int x = 4; return x; }

[[nodiscard]] int main1(void) noexcept(true) { int x = 4; return x; }
[[nodiscard]] int main2(void) noexcept(false) { int x = 4; return x; }
[[nodiscard]] int main3(void) noexcept { int x = 4; return x; }
[[nodiscard]] int main4(void) throw() { int x = 4; return x; }
[[nodiscard]] int main5(void) throw(...) { int x = 4; return x; }
[[nodiscard]] int main6(void) { int x = 4; return x; }
[[nodiscard]] int main7(void) { int x = 4; return x; }
