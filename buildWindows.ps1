cmake -S . -B Build
if ($?) {
    cmake --build .\Build\
    if ($?) {
        .\Build\Kompiler\Tests\Debug\KompilerTests.exe
    }
}