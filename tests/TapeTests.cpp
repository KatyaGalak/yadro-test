#include <gtest/gtest.h>
#include <filesystem>

#include "TapeConfig.hpp"
#include "FileTape.hpp"
#include "TapeSorter.hpp"

class TapeTest : public ::testing::Test {
protected:
    const std::string kTestIn = "input_test.bin";
    const std::string kTestOut = "output_test.bin";
    const std::string kTmpDir = "tmp_tests";

    TapeConfig config;

    void SetUp() override {
        TearDown();
    }

    void TearDown() override {
        if (std::filesystem::exists(kTestIn)) {
            std::filesystem::remove(kTestIn);
        }

        if (std::filesystem::exists(kTestOut)) {
            std::filesystem::remove(kTestOut);
        }

        if (std::filesystem::exists(kTmpDir)) {
            std::filesystem::remove_all(kTmpDir);
        }
    }

    void PrepareFile(const std::string& path, const std::vector<int32_t>& data) {
        std::ofstream os(path, std::ios::binary);

        for (int32_t val : data) {
            os.write(reinterpret_cast<const char*>(&val), sizeof(val));
        }
    }
};

TEST_F(TapeTest, InitializationAndEndCheck) {
    PrepareFile(kTestIn, {1, 2, 3});
    FileTape tape(kTestIn, config);

    ASSERT_FALSE(tape.is_end());

    EXPECT_NO_THROW(tape.move_first());
}

TEST_F(TapeTest, SequentialRead) {
    PrepareFile(kTestIn, {10, 20});
    FileTape tape(kTestIn, config);

    ASSERT_EQ(tape.read(), 10);
    ASSERT_TRUE(tape.move_next());
    ASSERT_EQ(tape.read(), 20);
    ASSERT_FALSE(tape.move_next());
    ASSERT_TRUE(tape.is_end());
}

TEST_F(TapeTest, PreviousMovement) {
    PrepareFile(kTestIn, {100, 200, 300});
    FileTape tape(kTestIn, config);

    tape.move_next();
    tape.move_next();
    ASSERT_EQ(tape.read(), 300);

    ASSERT_TRUE(tape.move_previous());
    ASSERT_EQ(tape.read(), 200);
}

TEST_F(TapeTest, WriteAndOverwriteValue) {
    FileTape tape(kTestIn, config);

    tape.write(1023);
    tape.move_first();
    ASSERT_EQ(tape.read(), 1023);

    tape.write(934155);
    tape.move_first();
    ASSERT_EQ(tape.read(), 934155);
}

TEST_F(TapeTest, ThrowReadEnd) {
    PrepareFile(kTestIn, {1});
    FileTape tape(kTestIn, config);

    tape.move_next();
    ASSERT_TRUE(tape.is_end());
    EXPECT_THROW(tape.read(), std::runtime_error);
}

TEST_F(TapeTest, SortEmptyTape) {
    PrepareFile(kTestIn, {});
    FileTape in(kTestIn, config);
    FileTape out(kTestOut, config);

    TapeSorter sorter(1024, kTmpDir, config);
    EXPECT_NO_THROW(sorter.sort(in, out));
}

TEST_F(TapeTest, SortWithinMemory) {
    PrepareFile(kTestIn, {5, 1, 4, 2, 3});
    FileTape in(kTestIn, config);
    FileTape out(kTestOut, config);

    TapeSorter sorter(100, kTmpDir, config);
    sorter.sort(in, out);

    out.move_first();
    std::vector<int32_t> result;
    while (!out.is_end()) {
        result.push_back(out.read());
        out.move_next();
    }

    std::vector<int32_t> expected = {1, 2, 3, 4, 5};

    ASSERT_EQ(result, expected);
}

TEST_F(TapeTest, SortOutOfMemory) {
    PrepareFile(kTestIn, {10, 9, 8, 7, 6, 5, 4, 3, 2, 1});
    FileTape in(kTestIn, config);
    FileTape out(kTestOut, config);

    TapeSorter sorter(8, kTmpDir, config);
    sorter.sort(in, out);

    out.move_first();
    for (int32_t i = 1; i <= 10; ++i) {
        ASSERT_EQ(out.read(), i);
        out.move_next();
    }
}

TEST_F(TapeTest, SingleElementSort) {
    PrepareFile(kTestIn, {40});
    FileTape in(kTestIn, config);
    FileTape out(kTestOut, config);

    TapeSorter sorter(4, kTmpDir, config);
    sorter.sort(in, out);

    out.move_first();
    ASSERT_EQ(out.read(), 40);
}

TEST_F(TapeTest, SortedWithDuplicates) {
    PrepareFile(kTestIn, {5, 1, 5, 2, 1, 3});
    FileTape in(kTestIn, config);
    FileTape out(kTestOut, config);

    TapeSorter sorter(8, kTmpDir, config);
    sorter.sort(in, out);

    out.move_first();

    std::vector<int32_t> result;
    while (!out.is_end()) {
        result.push_back(out.read());
        out.move_next();
    }

    std::vector<int32_t> expected = {1, 1, 2, 3, 5, 5};
    ASSERT_EQ(result, expected);
}

TEST_F(TapeTest, NonMultipleBufferSize) {
    std::vector<int32_t> data = {11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1};
    PrepareFile(kTestIn, data);
    FileTape in(kTestIn, config);
    FileTape out(kTestOut, config);

    TapeSorter sorter(12, kTmpDir, config);
    sorter.sort(in, out);

    out.move_first();
    for (int32_t i = 1; i <= 11; ++i) {
        ASSERT_EQ(out.read(), i);
        out.move_next();
    }
}

TEST_F(TapeTest, NegativeNumbers) {
    PrepareFile(kTestIn, {0, -52, 10, -123, 50});
    FileTape in(kTestIn, config);
    FileTape out(kTestOut, config);

    TapeSorter sorter(8, kTmpDir, config);
    sorter.sort(in, out);

    out.move_first();
    std::vector<int32_t> result;
    while (!out.is_end()) {
        result.push_back(out.read());
        out.move_next();
    }

    std::vector<int32_t> expected = {-123, -52, 0, 10, 50};
    ASSERT_EQ(result, expected);
}

TEST_F(TapeTest, MaxAndMinValues) {
    int32_t max = std::numeric_limits<int32_t>::max();
    int32_t min = std::numeric_limits<int32_t>::min();

    PrepareFile(kTestIn, {max, 0, min, 1, -1});
    FileTape in(kTestIn, config);
    FileTape out(kTestOut, config);

    TapeSorter sorter(8, kTmpDir, config);
    sorter.sort(in, out);

    out.move_first();
    std::vector<int32_t> result;

    while (!out.is_end()) {
        result.push_back(out.read());
        out.move_next();
    }

    std::vector<int32_t> expected = {min, -1, 0, 1, max};
    ASSERT_EQ(result, expected);
}

TEST_F(TapeTest, MinimumMemoryLimit) {
    PrepareFile(kTestIn, {3, 2, 1});
    FileTape in(kTestIn, config);
    FileTape out(kTestOut, config);

    TapeSorter sorter(4, kTmpDir, config);
    sorter.sort(in, out);

    out.move_first();
    EXPECT_EQ(out.read(), 1); out.move_next();
    EXPECT_EQ(out.read(), 2); out.move_next();
    EXPECT_EQ(out.read(), 3);
}

TEST_F(TapeTest, RepeatSorter) {
    TapeSorter sorter(8, kTmpDir, config);

    PrepareFile(kTestIn, {2, 1});
    FileTape in1(kTestIn, config);
    FileTape out1(kTestOut, config);
    sorter.sort(in1, out1);

    PrepareFile(kTestIn, {4, 3});
    FileTape in2(kTestIn, config);
    FileTape out2(kTestOut, config);
    sorter.sort(in2, out2);

    out2.move_first();

    EXPECT_EQ(out2.read(), 3);
    EXPECT_EQ(out2.move_next(), true);
    EXPECT_EQ(out2.read(), 4);
}

TEST_F(TapeTest, InvalidConfigFormat) {
    std::string config_path = "bad_config.txt";
    
    {
        std::ofstream os(config_path);
        os << "read=not_a_number\nwrite=10";
    }

    EXPECT_THROW({
        TapeConfig::load_from_file(config_path);
    }, std::runtime_error);

    std::filesystem::remove(config_path);
}

TEST_F(TapeTest, ValidConfigParsing) {
    std::string valid_config_path = "valid_config.txt";

    {
        std::ofstream os(valid_config_path);
        os << "read=31\nwrite=25\nshift=6\nmove_first=901";
    }

    TapeConfig cfg = TapeConfig::load_from_file(valid_config_path);

    EXPECT_EQ(cfg.delay_read_ms, 31);
    EXPECT_EQ(cfg.delay_write_ms, 25);
    EXPECT_EQ(cfg.delay_shift_ms, 6);
    EXPECT_EQ(cfg.delay_move_first_ms, 901);

    std::filesystem::remove(valid_config_path);
}

TEST_F(TapeTest, InvalidConfigValue) {
    std::string invalid_config_path = "invalid_value_config.txt";
    {
        std::ofstream os(invalid_config_path);
        os << "read=gjfnhjg66rgh\n";
        os << "write=217";
    }

    EXPECT_THROW({
        try {
            TapeConfig::load_from_file(invalid_config_path);
        } catch (const std::exception&) {
            std::filesystem::remove(invalid_config_path);

            throw;
        }
    }, std::exception);

    if (std::filesystem::exists(invalid_config_path)) {
        std::filesystem::remove(invalid_config_path);
    }
}

TEST_F(TapeTest, ConfigWithLinesWithoutKey) {
    std::string broken_path = "broken_lines.txt";

    {
        std::ofstream os(broken_path);

        os << "read=19\n";
        os << "66fdrghh gygyg fff -+8\n";
        os << "write=888";
    }

    TapeConfig cfg = TapeConfig::load_from_file(broken_path);

    EXPECT_EQ(cfg.delay_read_ms, 19);
    EXPECT_EQ(cfg.delay_write_ms, 888);

    std::filesystem::remove(broken_path);
}