#pragma once

#include <fstream>
#include <string>

#include "ITape.hpp"
#include "TapeConfig.hpp"

class FileTape : public ITape {
private:
    std::string filename_;
    std::fstream file_stream_;

    size_t current_position_;
    size_t file_size_;

    TapeConfig tape_config_;

    void sleep(int time_ms) const;

public:
    FileTape(const std::string& filename, const TapeConfig& tapeConfig);
    ~FileTape() override;

    int32_t read() override;
    void write(int32_t write_value) override;

    bool move_next() override;
    bool move_previous() override;
    void move_first() override;

    bool is_end() const override;
};