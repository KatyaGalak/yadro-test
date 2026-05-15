#include "TapeSorter.hpp"
#include "FileTape.hpp"

#include <filesystem>
#include <string>
#include <queue>
#include <memory>
#include <iostream>

TapeSorter::TapeSorter(size_t memory_limit, const std::string& tmp_directory, const TapeConfig& tape_config)
    : memory_limit_(memory_limit), tmp_directory_(tmp_directory), tape_config_(tape_config) {
    
    if (!std::filesystem::exists(tmp_directory_)) {
        std::filesystem::create_directories(tmp_directory_);
    }
}

struct Node {
    int32_t value;
    size_t block_index;

    bool operator > (const Node& o) const {
        return value > o.value;
    }
};

void TapeSorter::sort(ITape& input, ITape& output) {
    size_t cnt_max_elements = memory_limit_ / sizeof(int32_t);

    if (cnt_max_elements == 0) {
        throw std::runtime_error("The memory limit does not even hold one element");
    }

    std::vector<int32_t> buffer;
    buffer.reserve(cnt_max_elements);

    input.move_first();

    int block_index = 0;
    std::vector<std::string> tmp_filenames;

    while (!input.is_end()) {
        buffer.clear();

        while (!input.is_end() && buffer.size() < cnt_max_elements) {
            buffer.push_back(input.read());

            input.move_next();
        }

        if (buffer.empty()) {
            break;
        }

        std::sort(buffer.begin(), buffer.end());

        std::string tmpFilename = tmp_directory_ + "/tmp_tape_" + std::to_string(block_index++) + ".bin";

        tmp_filenames.push_back(tmpFilename);

        FileTape tmpTape(tmpFilename, tape_config_);

        for (size_t i = 0; i < buffer.size(); ++i) {
            tmpTape.write(buffer[i]);

            if (i < buffer.size() - 1) {
                tmpTape.move_next();
            }
        }
    }

    if (tmp_filenames.empty()) {
        return;
    }

    {
        std::vector<std::unique_ptr<FileTape> > tmp_tapes;
        std::priority_queue<Node, std::vector<Node>, std::greater<Node> > heap;

        for (size_t i = 0; i < tmp_filenames.size(); ++i) {
            auto tape = std::make_unique<FileTape>(tmp_filenames[i], tape_config_);

            tape->move_first();

            if (!tape->is_end()) {
                heap.push({tape->read(), i});
            }

            tmp_tapes.push_back(std::move(tape));
        }

        output.move_first();

        while (!heap.empty()) {
            Node node = heap.top();
            heap.pop();

            output.write(node.value);

            if (tmp_tapes[node.block_index]->move_next()) {
                heap.push({tmp_tapes[node.block_index]->read(), node.block_index});
            }

            output.move_next();
        }
    }

    for (const auto& filename : tmp_filenames) {
        if (std::filesystem::exists(filename)) {
            std::filesystem::remove(filename);
        }
    }
}