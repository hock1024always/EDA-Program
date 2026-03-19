#include <gtest/gtest.h>
#include "bookshelf_parser.h"
#include "circuit_model.h"
#include <fstream>
#include <filesystem>

using namespace eda;

class BookShelfParserTest : public ::testing::Test {
protected:
    void SetUp() override {
        parser = std::make_unique<BookShelfParser>();
        circuit = std::make_unique<CircuitModel>();

        // Create temporary test directory
        test_dir = "/tmp/eda_test_bookshelf";
        std::filesystem::create_directories(test_dir);
    }

    void TearDown() override {
        // Clean up test files
        std::filesystem::remove_all(test_dir);
    }

    void createTestFiles() {
        // Create .aux file
        std::ofstream aux(test_dir + "/test.aux");
        aux << "RowBasedPlacement : test.nodes test.nets test.wts test.pl test.scl\n";
        aux.close();

        // Create .nodes file
        std::ofstream nodes(test_dir + "/test.nodes");
        nodes << "UCLA nodes 1.0\n";
        nodes << "# Created for testing\n";
        nodes << "NumNodes : 4\n";
        nodes << "NumTerminals : 2\n";
        nodes << "\n";
        nodes << "o0 1 2\n";
        nodes << "o1 1 2\n";
        nodes << "a0 2 4 terminal\n";
        nodes << "a1 2 4 terminal\n";
        nodes.close();

        // Create .nets file
        std::ofstream nets(test_dir + "/test.nets");
        nets << "UCLA nets 1.0\n";
        nets << "NumNets : 2\n";
        nets << "NumPins : 4\n";
        nets << "\n";
        nets << "NetDegree : 2 n1\n";
        nets << "a0 O : 0 0\n";
        nets << "o0 I : 0.5 1\n";
        nets << "NetDegree : 2 n2\n";
        nets << "a1 O : 0 0\n";
        nets << "o1 I : 0.5 1\n";
        nets.close();

        // Create .scl file
        std::ofstream scl(test_dir + "/test.scl");
        scl << "UCLA scl 1.0\n";
        scl << "NumRows : 2\n";
        scl << "\n";
        scl << "CoreRow Horizontal\n";
        scl << "  Coordinate : 0\n";
        scl << "  Height : 12\n";
        scl << "  Sitewidth : 1\n";
        scl << "  Sitespacing : 1\n";
        scl << "  Siteorient : 1\n";
        scl << "  Sitesymmetry : 1\n";
        scl << "  SubrowOrigin : 0 NumSites : 100\n";
        scl << "End\n";
        scl << "\n";
        scl << "CoreRow Horizontal\n";
        scl << "  Coordinate : 12\n";
        scl << "  Height : 12\n";
        scl << "  Sitewidth : 1\n";
        scl << "  Sitespacing : 1\n";
        scl << "  Siteorient : 1\n";
        scl << "  Sitesymmetry : 1\n";
        scl << "  SubrowOrigin : 0 NumSites : 100\n";
        scl << "End\n";
        scl.close();

        // Create .pl file
        std::ofstream pl(test_dir + "/test.pl");
        pl << "UCLA pl 1.0\n";
        pl << "# Created for testing\n";
        pl << "\n";
        pl << "o0          10.0        10.0 : N\n";
        pl << "o1          20.0        20.0 : N\n";
        pl << "a0           0.0         0.0 : N /FIXED\n";
        pl << "a1          50.0         0.0 : N /FIXED\n";
        pl.close();

        // Create empty .wts file
        std::ofstream wts(test_dir + "/test.wts");
        wts << "UCLA wts 1.0\n";
        wts.close();
    }

    std::unique_ptr<BookShelfParser> parser;
    std::unique_ptr<CircuitModel> circuit;
    std::string test_dir;
};

TEST_F(BookShelfParserTest, ParseValidDesign) {
    createTestFiles();

    bool result = parser->parse(test_dir + "/test.aux", *circuit);

    EXPECT_TRUE(result);
    EXPECT_EQ(circuit->getNumModules(), 4);
    EXPECT_EQ(circuit->getNumNets(), 2);
    EXPECT_EQ(circuit->rows.size(), 2);
}

TEST_F(BookShelfParserTest, ModuleTypes) {
    createTestFiles();
    parser->parse(test_dir + "/test.aux", *circuit);

    auto o0 = circuit->getModule("o0");
    auto a0 = circuit->getModule("a0");

    ASSERT_NE(o0, nullptr);
    ASSERT_NE(a0, nullptr);

    EXPECT_EQ(o0->type, ModuleType::STANDARD_CELL);
    EXPECT_FALSE(o0->is_fixed);

    EXPECT_EQ(a0->type, ModuleType::TERMINAL);
    EXPECT_TRUE(a0->is_fixed);
}

TEST_F(BookShelfParserTest, NetConnections) {
    createTestFiles();
    parser->parse(test_dir + "/test.aux", *circuit);

    auto net = circuit->getNet("n1");
    ASSERT_NE(net, nullptr);
    EXPECT_EQ(net->pins.size(), 2);
}

TEST_F(BookShelfParserTest, DieAreaFromRows) {
    createTestFiles();
    parser->parse(test_dir + "/test.aux", *circuit);

    EXPECT_GT(circuit->die_area.width(), 0);
    EXPECT_GT(circuit->die_area.height(), 0);
    EXPECT_EQ(circuit->rows.size(), 2);
}

TEST_F(BookShelfParserTest, ParseNonexistentFile) {
    bool result = parser->parse("/nonexistent/path/test.aux", *circuit);
    EXPECT_FALSE(result);
    EXPECT_FALSE(parser->getLastError().empty());
}
