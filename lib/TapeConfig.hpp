#pragma once
#include <string>
#include <map>
#include <fstream>

struct TapeConfig {
    int delayReadMs = 0;

    int delayWriteMs = 0;

    int delayShiftMs = 0;

    int delayMoveFirstMs = 0;

    static TapeConfig loadFromFile(const std::string& filename) {
        TapeConfig tapeConfig;

        std::ifstream file(filename);

        if (!file.is_open()) { // Это неизящно
            return tapeConfig;
        }

        std::map<std::string, int*> registry = {
            {"read",   &tapeConfig.delayReadMs},
            {"write",  &tapeConfig.delayWriteMs},
            {"shift",  &tapeConfig.delayShiftMs},
            {"moveFirst", &tapeConfig.delayMoveFirstMs}
        };

        std::string readLine;
        while (std::getline(file, readLine)) {
            size_t equalsSign = readLine.find('=');

            if (equalsSign == std::string::npos) {
                continue;
            }

            std::string key = readLine.substr(0, equalsSign);
            std::string value = readLine.substr(equalsSign + 1);

            if (registry.count(key)) {
                try {
                    *registry[key] = std::stoi(value);
                } catch(...) {} // надо бы ошибки ловить
            }
        }

        return tapeConfig;
    }
    
};