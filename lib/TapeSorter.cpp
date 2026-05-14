#include "TapeSorter.hpp"
#include "FileTape.hpp"

#include <filesystem>
#include <string>
#include <queue>
#include <memory>
#include <iostream>

TapeSorter::TapeSorter(size_t memoryLimitInBytes, const std::string& tmpDirectory, const TapeConfig& tapeConfig)
    : memoryLimitInBytes(memoryLimitInBytes), tmpDirectory(tmpDirectory), tapeConfig(tapeConfig) {
    
    if (!std::filesystem::exists(tmpDirectory)) {
        std::filesystem::create_directories(tmpDirectory);
    }
}

struct Node {
    int32_t value;
    size_t blockIndex;

    bool operator > (const Node& o) const {
        return value > o.value;
    }
};

void TapeSorter::sort(ITape& input, ITape& output) {
    size_t cntMaxElements = memoryLimitInBytes / sizeof(int32_t);

    if (cntMaxElements == 0) {
        throw std::runtime_error("The memory limit does not even hold one element");
    }

    std::vector<int32_t> buffer(cntMaxElements);

    input.moveFirst();

    int blockIndex = 0;
    std::vector<std::string> tmpFilenames;

    while (!input.isEnd()) {
        buffer.clear();

        while (!input.isEnd() && buffer.size() < cntMaxElements) {
            buffer.push_back(input.read());

            input.moveNext();
        }

        if (buffer.empty()) {
            break;
        }

        std::sort(buffer.begin(), buffer.end());

        std::string tmpFilename = tmpDirectory + "/tmp_tape_" + std::to_string(blockIndex++) + ".bin";

        tmpFilenames.push_back(tmpFilename);

        FileTape tmpTape(tmpFilename, tapeConfig);

        for (size_t i = 0; i < buffer.size(); ++i) {
            tmpTape.write(buffer[i]);

            if (i < buffer.size() - 1) {
                tmpTape.moveNext();
            }
        }
    }

    if (tmpFilenames.empty()) {
        return;
    }

    {
        std::vector<std::unique_ptr<FileTape> > tmpTapes;
        std::priority_queue<Node, std::vector<Node>, std::greater<Node> > heap;

        for (size_t i = 0; i < tmpFilenames.size(); ++i) {
            auto tape = std::make_unique<FileTape>(tmpFilenames[i], tapeConfig);

            tape->moveFirst();

            if (!tape->isEnd()) {
                heap.push({tape->read(), i});
            }

            tmpTapes.push_back(std::move(tape));
        }

        output.moveFirst();

        while (!heap.empty()) {
            Node node = heap.top();
            heap.pop();

            output.write(node.value);

            if (tmpTapes[node.blockIndex]->moveNext()) {
                heap.push({tmpTapes[node.blockIndex]->read(), node.blockIndex});
            }

            output.moveNext();
        }
    }

    for (const auto& filename : tmpFilenames) {
        if (std::filesystem::exists(filename)) {
            std::filesystem::remove(filename);
        }
    }
}