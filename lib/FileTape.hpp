#pragma once

#include <fstream>
#include <string>

#include "ITape.hpp"
#include "TapeConfig.hpp"

class FileTape : public ITape {
private:
    std::string filename;
    std::fstream file;

    size_t currentPosition;
    size_t fileSize;

    TapeConfig tapeConfig;

    void sleep(int timeMs) const;

public:
    FileTape(const std::string& filename, const TapeConfig& tapeConfig);
    ~FileTape() override;

    int32_t read() override;
    void write(int32_t writeValue) override;

    bool moveNext() override;
    bool movePrevious() override;
    void moveFirst() override;

    bool isEnd() const override;
};