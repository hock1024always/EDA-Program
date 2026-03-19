#include <gtest/gtest.h>
#include "verilog_parser.h"
#include "circuit_model.h"

using namespace eda;

class VerilogParserTest : public ::testing::Test {
protected:
    void SetUp() override {
        parser = std::make_unique<VerilogParser>();
        circuit = std::make_unique<CircuitModel>();
    }

    std::unique_ptr<VerilogParser> parser;
    std::unique_ptr<CircuitModel> circuit;
};

TEST_F(VerilogParserTest, ParseSimpleModule) {
    std::string verilog = R"(
module test_module(
    input a,
    input b,
    output y
);

wire temp;

NAND2X1 g1 (.A(a), .B(b), .Y(temp));
INVX1 g2 (.A(temp), .Y(y));

endmodule
)";

    bool result = parser->parseString(verilog, *circuit);

    EXPECT_TRUE(result);
    EXPECT_EQ(circuit->name, "test_module");
}

TEST_F(VerilogParserTest, ParseModuleHeader) {
    std::string verilog = R"(
module adder(
    input [3:0] a,
    input [3:0] b,
    output [3:0] sum
);
    // Empty module
endmodule
)";

    bool result = parser->parseString(verilog, *circuit);
    EXPECT_TRUE(result);
}

TEST_F(VerilogParserTest, ParseMultipleInstances) {
    std::string verilog = R"(
module test;
    input a, b, c;
    output y;
    wire w1, w2;

    AND2X1 u1 (.A(a), .B(b), .Y(w1));
    OR2X1 u2 (.A(w1), .B(c), .Y(w2));
    INVX1 u3 (.A(w2), .Y(y));
endmodule
)";

    bool result = parser->parseString(verilog, *circuit);
    EXPECT_TRUE(result);
}

TEST_F(VerilogParserTest, ParseWithComments) {
    std::string verilog = R"(
// This is a comment
module test;
    /* Multi-line
       comment */
    input a;
    output b;
endmodule
)";

    bool result = parser->parseString(verilog, *circuit);
    EXPECT_TRUE(result);
}

TEST_F(VerilogParserTest, InvalidModule) {
    std::string verilog = "not_a_module test; endmodule";

    bool result = parser->parseString(verilog, *circuit);
    EXPECT_FALSE(result);
    EXPECT_FALSE(parser->getLastError().empty());
}
