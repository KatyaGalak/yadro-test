#include "FileTape.hpp"

#include <filesystem>
#include <thread>
#include <chrono>

FileTape::FileTape(const std::string& filename, const TapeConfig& tape_config)
    : filename_(filename), tape_config_(tape_config), current_position_(0) {

    if (!std::filesystem::exists(filename_)) {
        std::ofstream create_file(filename_, std::ios::binary);
    }

    file_stream_.open(filename_, std::ios::in | std::ios::out | std::ios::binary);

    if (!file_stream_.is_open()) {
        throw std::runtime_error("[ERROR] Couldn't open or create a tape file_stream_:" + filename_);
    }

    file_stream_.seekg(0, std::ios::end);
    file_size_ = static_cast<size_t>(file_stream_.tellg());

    file_stream_.seekg(0, std::ios::beg);
}

FileTape::~FileTape() {
    if (file_stream_.is_open())
        file_stream_.close();
}

void FileTape::sleep(int time_ms) const {
    if (time_ms > 0) {
        std::this_thread::sleep_for(std::chrono::milliseconds(time_ms));
    }
}

int32_t FileTape::read() {

    if (is_end()) {
        throw std::runtime_error("[ERROR] Attempt to read at end of tape: " + filename_);
    }

    sleep(tape_config_.delay_read_ms);

    int32_t readValue = 0;

    file_stream_.clear();
    file_stream_.seekg(current_position_);
    file_stream_.read(reinterpret_cast<char*>(&readValue), sizeof(readValue));

    return readValue;
}

void FileTape::write(int32_t write_value) {
    sleep(tape_config_.delay_write_ms);

    file_stream_.clear();
    file_stream_.seekg(current_position_);

    file_stream_.write(reinterpret_cast<char*>(&write_value), sizeof(write_value));

    size_t sizeAfterAdd = current_position_ + sizeof(int32_t);
    if (sizeAfterAdd > file_size_) {
        file_size_ = sizeAfterAdd;

        file_stream_.flush();
    }
}

bool FileTape::move_next() {
    sleep(tape_config_.delay_shift_ms);

    if (current_position_ < file_size_) {
        current_position_ += sizeof(int32_t);
        return current_position_ < file_size_;
    }

    return false;
}

bool FileTape::move_previous() {
    sleep(tape_config_.delay_shift_ms);

    if (current_position_ >= sizeof(int32_t)) {
        current_position_ -= sizeof(int32_t);

        return true;
    }

    return false;
}

void FileTape::move_first() {
    sleep(tape_config_.delay_move_first_ms);

    current_position_ = 0;
    file_stream_.clear();

    file_stream_.seekg(0);
}

bool FileTape::is_end() const {
    return current_position_ >= file_size_;
}