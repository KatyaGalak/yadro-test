#pragma once

#include "ITape.hpp"
#include "TapeConfig.hpp"

#include <string>
#include <vector>

class TapeSorter {
private:
    size_t memory_limit_;

    std::string tmp_directory_;

    TapeConfig tape_config_;

public:
    TapeSorter(size_t memory_limit, const std::string& tmp_directory, const TapeConfig& tape_config);

    void sort(ITape& input, ITape& output);
};