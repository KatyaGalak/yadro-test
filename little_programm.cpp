#include <fstream>
#include <vector>
#include <cstdint>

int main() {
    std::ofstream out("input.bin", std::ios::binary);
    std::vector<int32_t> data = {100, 5, 20, 1, 30};
    for (int32_t x : data) {
        out.write(reinterpret_cast<const char*>(&x), sizeof(int32_t));
    }
    return 0;
}