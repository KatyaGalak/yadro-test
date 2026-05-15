#include <iostream>
#include <string>

#include "TapeConfig.hpp"
#include "FileTape.hpp"
#include "TapeSorter.hpp"

void print_binary_file(const std::string& file_name) {
    std::ifstream file(file_name, std::ios::binary);
    int32_t value;
    std::cout << file_name << ": ";

    while (file.read(reinterpret_cast<char*>(&value), sizeof(value))) {
        std::cout << value << " ";
    }
    std::cout << std::endl;
}

int main(int argc, char* argv[]) {
    if (argc != 4) {
        std::cerr << "[ERROR] There must be 3 required command line arguments:" << std::endl;
        std::cerr << argv[0] << " <input_file> <output_file> <memory_limit_bytes>" << std::endl;

        std::cerr << "Example: " << argv[0] << " input.bin output.bin 1024" << std::endl;

        return 1;
    }

    std::string input_file_name = argv[1];
    std::string output_file_name = argv[2];
    size_t memory_limit = 0;

    try {
        memory_limit = std::stoull(argv[3]);
    } catch (const std::exception&) {
        std::cerr << "[ERROR] Invalid memory limit value: " << argv[3] << std::endl;
        return 1;
    }

    try {
        TapeConfig tape_config = TapeConfig::load_from_file("config/config.txt");

        FileTape input_tape(input_file_name, tape_config);

        FileTape output_tape(output_file_name, tape_config);

        TapeSorter tapeSorter(memory_limit, "tmp", tape_config);

        tapeSorter.sort(input_tape, output_tape);

        // print_binary_file(input_file_name);
        // print_binary_file(output_file_name);

    } catch (const std::exception& e) {
        std::cerr << "[ERROR] " << e.what() << std::endl;
        return 1;
    }
}