#include <iostream>
#include <string>

#include "TapeConfig.hpp"
#include "FileTape.hpp"
#include "TapeSorter.hpp"

void printBinaryFile(const std::string& fileName) {
    std::ifstream file(fileName, std::ios::binary);
    int32_t value;
    std::cout << fileName << ": ";
    while (file.read(reinterpret_cast<char*>(&value), sizeof(value))) {
        std::cout << value << " ";
    }
    std::cout << std::endl;
}

int main(int argc, char* argv[]) {
    // TODO проверка аргументов

    std::string inputFileName = argv[1];
    std::string outputFileName = argv[2];
    size_t memoryLimit = 0;

    try {
        memoryLimit = std::stoull(argv[3]);
    } catch (const std::exception&) {
        std::cerr << "[ERROR] Invalid memory limit value: " << argv[3] << std::endl;
        return 1;
    }
    
    try {
        TapeConfig tapeConfig = TapeConfig::loadFromFile("config/config.txt");

        FileTape inputTape(inputFileName, tapeConfig);

        FileTape outputTape(outputFileName, tapeConfig);

        TapeSorter tapeSorter(memoryLimit, "tmp", tapeConfig);

        tapeSorter.sort(inputTape, outputTape);

        printBinaryFile(inputFileName);
        printBinaryFile(outputFileName);
    } catch (const std::exception& e) {
        std::cerr << "[ERROR] " << e.what() << std::endl;
        return 1;
    }
}