#pragma once

#include "ITape.hpp"
#include "TapeConfig.hpp"

#include <string>
#include <vector>

class TapeSorter {
private:
    size_t memoryLimitInBytes;

    std::string tmpDirectory;

    TapeConfig tapeConfig;

public:
    TapeSorter(size_t memoryLimitInBytes, const std::string& tmpDirectory, const TapeConfig& tapeConfig);

    void sort(ITape& input, ITape& output);
};