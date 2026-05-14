#include "FileTape.hpp"

#include <filesystem>
#include <thread>
#include <chrono>

FileTape::FileTape(const std::string& filename, const TapeConfig& tapeConfig)
    : filename(filename), tapeConfig(tapeConfig), currentPosition(0) {

    if (!std::filesystem::exists(filename)) {
        std::ofstream create_file(filename, std::ios::binary);
    }

    file.open(filename, std::ios::in | std::ios::out | std::ios::binary);

    if (!file.is_open()) {
        throw std::runtime_error("[ERROR] Couldn't open or create a tape file:" + filename);
    }
    
    file.seekg(0, std::ios::end);
    fileSize = static_cast<size_t>(file.tellg());

    file.seekg(0, std::ios::beg);
}

FileTape::~FileTape() {
    if (file.is_open()) // неизящно
        file.close();
}

void FileTape::sleep(int timeMs) const {
    if (timeMs > 0) {
        std::this_thread::sleep_for(std::chrono::milliseconds(timeMs));
    }
}

int32_t FileTape::read() {

    if (isEnd()) {
        throw std::runtime_error("[ERROR] Attempt to read at end of tape: " + filename);
    }

    sleep(tapeConfig.delayReadMs);

    int32_t readValue = 0;

    file.clear();
    file.seekg(currentPosition);
    file.read(reinterpret_cast<char*>(&readValue), sizeof(readValue));

    return readValue;
}

void FileTape::write(int32_t writeValue) {
    sleep(tapeConfig.delayWriteMs);

    file.clear();
    file.seekg(currentPosition);

    file.write(reinterpret_cast<char*>(&writeValue), sizeof(writeValue));

    size_t sizeAfterAdd = currentPosition + sizeof(int32_t);
    if (sizeAfterAdd > fileSize) {
        fileSize = sizeAfterAdd;

        file.flush();
    }
}

bool FileTape::moveNext() { // точно ли нужен bool
    sleep(tapeConfig.delayShiftMs);

    if (currentPosition < fileSize) {
        currentPosition += sizeof(int32_t);
        return true;
    }

    return false;
}

bool FileTape::movePrevious() {
    sleep(tapeConfig.delayShiftMs);

    if (currentPosition >= sizeof(int32_t)) {
        currentPosition -= sizeof(int32_t);

        return true;
    }

    return false;
}

void FileTape::moveFirst() {
    sleep(tapeConfig.delayMoveFirstMs);

    currentPosition = 0;
    file.clear();

    file.seekg(0);
}

bool FileTape::isEnd() const {
    return currentPosition >= fileSize;
}