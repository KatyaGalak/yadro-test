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

    ASSERT_FALSE(tape.isEnd());

    EXPECT_NO_THROW(tape.moveFirst());
}

TEST_F(TapeTest, SequentialRead) {
    PrepareFile(kTestIn, {10, 20});
    FileTape tape(kTestIn, config);

    ASSERT_EQ(tape.read(), 10);
    ASSERT_TRUE(tape.moveNext());
    ASSERT_EQ(tape.read(), 20);
    ASSERT_FALSE(tape.moveNext());
    ASSERT_TRUE(tape.isEnd());
}

TEST_F(TapeTest, PreviousMovement) {
    PrepareFile(kTestIn, {100, 200, 300});
    FileTape tape(kTestIn, config);

    tape.moveNext();
    tape.moveNext();
    ASSERT_EQ(tape.read(), 300);
    
    ASSERT_TRUE(tape.movePrevious());
    ASSERT_EQ(tape.read(), 200);
}

TEST_F(TapeTest, WriteAndOverwriteValue) {
    FileTape tape(kTestIn, config);
    
    tape.write(1023);
    tape.moveFirst();
    ASSERT_EQ(tape.read(), 1023);

    tape.write(934155);
    tape.moveFirst();
    ASSERT_EQ(tape.read(), 934155);
}

TEST_F(TapeTest, ThrowReadEnd) {
    PrepareFile(kTestIn, {1});
    FileTape tape(kTestIn, config);

    tape.moveNext();
    ASSERT_TRUE(tape.isEnd());
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

    out.moveFirst();
    std::vector<int32_t> result;
    while (!out.isEnd()) {
        result.push_back(out.read());
        out.moveNext();
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

    out.moveFirst();
    for (int32_t i = 1; i <= 10; ++i) {
        ASSERT_EQ(out.read(), i);
        out.moveNext();
    }
}

TEST_F(TapeTest, SingleElementSort) {
    PrepareFile(kTestIn, {40});
    FileTape in(kTestIn, config);
    FileTape out(kTestOut, config);

    TapeSorter sorter(4, kTmpDir, config);
    sorter.sort(in, out);

    out.moveFirst();
    ASSERT_EQ(out.read(), 40);
}

TEST_F(TapeTest, SortedWithDuplicates) {
    PrepareFile(kTestIn, {5, 1, 5, 2, 1, 3});
    FileTape in(kTestIn, config);
    FileTape out(kTestOut, config);

    TapeSorter sorter(8, kTmpDir, config); 
    sorter.sort(in, out);

    out.moveFirst();

    std::vector<int32_t> result;
    while (!out.isEnd()) {
        result.push_back(out.read());
        out.moveNext();
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

    out.moveFirst();
    for (int32_t i = 1; i <= 11; ++i) {
        ASSERT_EQ(out.read(), i);
        out.moveNext();
    }
}

TEST_F(TapeTest, NegativeNumbers) {
    PrepareFile(kTestIn, {0, -52, 10, -123, 50});
    FileTape in(kTestIn, config);
    FileTape out(kTestOut, config);

    TapeSorter sorter(8, kTmpDir, config); 
    sorter.sort(in, out);

    out.moveFirst();
    std::vector<int32_t> result;
    while (!out.isEnd()) {
        result.push_back(out.read());
        out.moveNext();
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

    out.moveFirst();
    std::vector<int32_t> result;

    while (!out.isEnd()) {
        result.push_back(out.read());
        out.moveNext();
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

    out.moveFirst();
    EXPECT_EQ(out.read(), 1); out.moveNext();
    EXPECT_EQ(out.read(), 2); out.moveNext();
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

    out2.moveFirst();

    EXPECT_EQ(out2.read(), 3);
    EXPECT_EQ(out2.moveNext(), true);
    EXPECT_EQ(out2.read(), 4);
}

TEST_F(TapeTest, InvalidConfigFormat) {
    std::string configPath = "bad_config.txt";
    {
        std::ofstream os(configPath);
        os << "read=not_a_number\nwrite=10";
    }

    EXPECT_THROW({
        TapeConfig::loadFromFile(configPath);
    }, std::runtime_error);

    std::filesystem::remove(configPath);
}