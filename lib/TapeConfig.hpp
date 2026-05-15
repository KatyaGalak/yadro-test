#pragma once
#include <string>
#include <map>
#include <fstream>

struct TapeConfig {
    int delay_read_ms = 0;

    int delay_write_ms = 0;

    int delay_shift_ms = 0;

    int delay_move_first_ms = 0;

    static TapeConfig load_from_file(const std::string& filename) {
        TapeConfig tape_config;

        std::ifstream file(filename);

        if (!file.is_open()) {
            return tape_config;
        }

        std::map<std::string, int*> registry = {
            {"read",   &tape_config.delay_read_ms},
            {"write",  &tape_config.delay_write_ms},
            {"shift",  &tape_config.delay_shift_ms},
            {"move_first", &tape_config.delay_move_first_ms}
        };

        std::string read_line;
        while (std::getline(file, read_line)) {
            size_t equals_sign = read_line.find('=');

            if (equals_sign == std::string::npos) {
                continue;
            }

            std::string key = read_line.substr(0, equals_sign);
            std::string value = read_line.substr(equals_sign + 1);

            if (registry.count(key)) {
                try {
                    *registry[key] = std::stoi(value);
                } catch(const std::invalid_argument& e) {
                    throw std::runtime_error("Invalid number format for key '" + key + "': " + value + ". " + e.what());
                }
            }
        }

        return tape_config;
    }

};