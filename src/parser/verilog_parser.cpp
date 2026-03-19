#include "verilog_parser.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <cctype>
#include <stdexcept>

namespace eda {

// Verilog keywords
const std::unordered_set<std::string> VerilogLexer::keywords = {
    "module", "endmodule", "input", "output", "inout", "wire", "reg",
    "assign", "always", "initial", "begin", "end", "if", "else", "case",
    "endcase", "for", "while", "parameter", "localparam", "defparam",
    "posedge", "negedge", "or", "and", "nand", "nor", "xor", "xnor",
    "not", "buf"
};

// Constructor
VerilogLexer::VerilogLexer(const std::string& input)
    : input_(input), position_(0), line_(1), column_(1) {}

void VerilogLexer::skipWhitespace() {
    while (position_ < input_.length() &&
           (input_[position_] == ' ' || input_[position_] == '\t' ||
            input_[position_] == '\n' || input_[position_] == '\r')) {
        if (input_[position_] == '\n') {
            line_++;
            column_ = 1;
        } else {
            column_++;
        }
        position_++;
    }
}

void VerilogLexer::skipComment() {
    // Skip single-line comment
    if (position_ + 1 < input_.length() &&
        input_[position_] == '/' && input_[position_ + 1] == '/') {
        while (position_ < input_.length() && input_[position_] != '\n') {
            position_++;
        }
    }
    // Skip multi-line comment
    else if (position_ + 1 < input_.length() &&
             input_[position_] == '/' && input_[position_ + 1] == '*') {
        position_ += 2;
        while (position_ + 1 < input_.length() &&
               !(input_[position_] == '*' && input_[position_ + 1] == '/')) {
            if (input_[position_] == '\n') {
                line_++;
            }
            position_++;
        }
        position_ += 2; // Skip */
    }
}

char VerilogLexer::peek() const {
    if (position_ < input_.length()) {
        return input_[position_];
    }
    return '\0';
}

char VerilogLexer::advance() {
    if (position_ < input_.length()) {
        column_++;
        return input_[position_++];
    }
    return '\0';
}

bool VerilogLexer::isAtEnd() const {
    return position_ >= input_.length();
}

VerilogToken VerilogLexer::readIdentifier() {
    std::string identifier;
    int start_line = line_;
    int start_col = column_;

    while (position_ < input_.length() &&
           (std::isalnum(input_[position_]) || input_[position_] == '_' ||
            input_[position_] == '$' || input_[position_] == '\\')) {
        identifier += advance();
    }

    // Handle escaped identifiers (\name )
    if (!identifier.empty() && identifier[0] == '\\') {
        while (position_ < input_.length() && peek() != ' ') {
            identifier += advance();
        }
        if (peek() == ' ') advance(); // Skip trailing space
    }

    VerilogTokenType type = keywords.count(identifier) > 0
                            ? VerilogTokenType::Keyword
                            : VerilogTokenType::Identifier;

    VerilogToken token(type, identifier);
    token.line = start_line;
    token.column = start_col;
    return token;
}

VerilogToken VerilogLexer::readNumber() {
    std::string number;
    int start_line = line_;
    int start_col = column_;

    // Handle binary, octal, hex prefixes
    if (peek() == '\'' || peek() == '0' || peek() == '1' ||
        peek() == '2' || peek() == '3' || peek() == '4' ||
        peek() == '5' || peek() == '6' || peek() == '7' ||
        peek() == '8' || peek() == '9') {

        // Check for sized number like 4'b0101
        if (std::isdigit(peek())) {
            while (position_ < input_.length() && std::isdigit(peek())) {
                number += advance();
            }
        }

        // Handle base specifier
        if (peek() == '\'') {
            number += advance();
            if (peek() == 'b' || peek() == 'B' ||
                peek() == 'o' || peek() == 'O' ||
                peek() == 'd' || peek() == 'D' ||
                peek() == 'h' || peek() == 'H') {
                number += advance();
            }

            while (position_ < input_.length() &&
                   (std::isxdigit(peek()) || peek() == 'x' || peek() == 'X' ||
                    peek() == 'z' || peek() == 'Z' || peek() == '_')) {
                number += advance();
            }
        }
    }

    VerilogToken token(VerilogTokenType::Number, number);
    token.line = start_line;
    token.column = start_col;
    return token;
}

VerilogToken VerilogLexer::readString() {
    std::string str;
    int start_line = line_;
    int start_col = column_;

    advance(); // Skip opening quote

    while (position_ < input_.length() && peek() != '"') {
        if (peek() == '\\' && position_ + 1 < input_.length()) {
            advance(); // Skip backslash
            char escaped = advance();
            switch (escaped) {
                case 'n': str += '\n'; break;
                case 't': str += '\t'; break;
                case '\\': str += '\\'; break;
                case '"': str += '"'; break;
                default: str += escaped; break;
            }
        } else {
            str += advance();
        }
    }

    if (peek() == '"') {
        advance(); // Skip closing quote
    }

    VerilogToken token(VerilogTokenType::String, str);
    token.line = start_line;
    token.column = start_col;
    return token;
}

VerilogToken VerilogLexer::nextToken() {
    skipWhitespace();
    skipComment();
    skipWhitespace();

    if (isAtEnd()) {
        return VerilogToken(VerilogTokenType::EndOfInput, "");
    }

    char c = peek();

    // Identifier or keyword
    if (std::isalpha(c) || c == '_' || c == '\\') {
        return readIdentifier();
    }

    // Number
    if (std::isdigit(c)) {
        return readNumber();
    }

    // String literal
    if (c == '"') {
        return readString();
    }

    // Single-character symbols
    int start_line = line_;
    int start_col = column_;
    std::string symbol(1, advance());

    // Multi-character operators
    if (!isAtEnd()) {
        char next = peek();
        if ((symbol == "&" && next == "&") ||
            (symbol == "|" && next == "|") ||
            (symbol == "=" && next == "=") ||
            (symbol == "!" && next == "=") ||
            (symbol == "<" && next == "=") ||
            (symbol == ">" && next == "=") ||
            (symbol == "-" && next == ">") ||
            (symbol == "<" && next == "<") ||
            (symbol == ">" && next == ">")) {
            symbol += advance();
        }
    }

    VerilogToken token(VerilogTokenType::Symbol, symbol);
    token.line = start_line;
    token.column = start_col;
    return token;
}

std::string VerilogLexer::getWholeExpression() {
    skipWhitespace();
    size_t start = position_;

    // Read until semicolon
    while (position_ < input_.length() && input_[position_] != ';') {
        if (input_[position_] == '\n') {
            line_++;
            column_ = 1;
        } else {
            column_++;
        }
        position_++;
    }

    std::string expr = input_.substr(start, position_ - start);

    // Skip the semicolon
    if (position_ < input_.length() && input_[position_] == ';') {
        position_++;
    }

    // Trim whitespace
    size_t first = expr.find_first_not_of(" \t\r\n");
    size_t last = expr.find_last_not_of(" \t\r\n");
    if (first != std::string::npos && last != std::string::npos) {
        return expr.substr(first, last - first + 1);
    }
    return expr;
}

// VerilogParser implementation
VerilogParser::VerilogParser() = default;

bool VerilogParser::parse(const std::string& filename, CircuitModel& circuit) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        last_error_ = "Cannot open file: " + filename;
        return false;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    file.close();

    return parseString(buffer.str(), circuit);
}

bool VerilogParser::parseString(const std::string& content, CircuitModel& circuit) {
    current_circuit_ = &circuit;
    circuit.clear();

    lexer_ = std::make_unique<VerilogLexer>(content);

    try {
        VerilogModule vmodule;
        if (!parseModule(vmodule)) {
            return false;
        }

        return convertToCircuitModel(vmodule);
    }
    catch (const std::exception& e) {
        last_error_ = std::string("Parse error: ") + e.what();
        return false;
    }
}

bool VerilogParser::parseModule(VerilogModule& module) {
    // Parse module header
    if (!parseModuleHeader(module)) {
        return false;
    }

    // Parse port declarations (input, output)
    if (!parsePortDeclarations(module)) {
        return false;
    }

    // Parse wire declarations
    parseWireDeclarations(module);

    // Parse instances and assigns
    parseInstances(module);
    parseAssignStatements(module);

    // Expect endmodule
    auto token = lexer_->nextToken();
    if (token.type != VerilogTokenType::Keyword || token.value != "endmodule") {
        last_error_ = "Expected 'endmodule' at line " + std::to_string(token.line);
        return false;
    }

    return true;
}

bool VerilogParser::parseModuleHeader(VerilogModule& module) {
    // Expect 'module'
    auto token = lexer_->nextToken();
    if (token.type != VerilogTokenType::Keyword || token.value != "module") {
        last_error_ = "Expected 'module' at line " + std::to_string(token.line);
        return false;
    }

    // Get module name
    token = lexer_->nextToken();
    if (token.type != VerilogTokenType::Identifier) {
        last_error_ = "Expected module name at line " + std::to_string(token.line);
        return false;
    }
    module.name = token.value;

    // Parse port list (optional in Verilog-2001)
    token = lexer_->nextToken();
    if (token.type == VerilogTokenType::Symbol && token.value == "(") {
        // Parse port list
        token = lexer_->nextToken();
        while (token.type != VerilogTokenType::Symbol || token.value != ")") {
            if (token.type == VerilogTokenType::Identifier) {
                module.ports.push_back(token.value);
            }
            token = lexer_->nextToken();
            if (token.type == VerilogTokenType::Symbol && token.value == ",") {
                token = lexer_->nextToken();
            }
        }
        token = lexer_->nextToken(); // Skip )
    }

    // Expect ';'
    if (token.type != VerilogTokenType::Symbol || token.value != ";") {
        last_error_ = "Expected ';' after module header at line " + std::to_string(token.line);
        return false;
    }

    return true;
}

bool VerilogParser::parsePortDeclarations(VerilogModule& module) {
    auto token = lexer_->nextToken();

    while (token.type == VerilogTokenType::Keyword &&
           (token.value == "input" || token.value == "output" || token.value == "inout")) {

        std::string direction = token.value;

        // Skip optional width declaration [n:m]
        token = lexer_->nextToken();
        if (token.type == VerilogTokenType::Symbol && token.value == "[") {
            while (token.type != VerilogTokenType::Symbol || token.value != "]") {
                token = lexer_->nextToken();
            }
            token = lexer_->nextToken(); // Skip ]
        }

        // Parse port names
        while (token.type == VerilogTokenType::Identifier) {
            std::string port_name = token.value;

            if (direction == "input") {
                module.inputs.push_back(port_name);
            } else if (direction == "output") {
                module.outputs.push_back(port_name);
            }

            token = lexer_->nextToken();
            if (token.type == VerilogTokenType::Symbol && token.value == ",") {
                token = lexer_->nextToken();
            }
        }

        // Expect ';'
        if (token.type != VerilogTokenType::Symbol || token.value != ";") {
            last_error_ = "Expected ';' after port declaration";
            return false;
        }

        token = lexer_->nextToken();
    }

    // Put back the last token for next parser
    // (In a real implementation, we'd use a lookahead buffer)

    return true;
}

bool VerilogParser::parseWireDeclarations(VerilogModule& module) {
    auto token = lexer_->nextToken();

    while (token.type == VerilogTokenType::Keyword && token.value == "wire") {
        // Skip optional width
        token = lexer_->nextToken();
        if (token.type == VerilogTokenType::Symbol && token.value == "[") {
            while (token.type != VerilogTokenType::Symbol || token.value != "]") {
                token = lexer_->nextToken();
            }
            token = lexer_->nextToken();
        }

        // Parse wire names
        while (token.type == VerilogTokenType::Identifier) {
            module.wires.push_back(token.value);

            token = lexer_->nextToken();
            if (token.type == VerilogTokenType::Symbol && token.value == ",") {
                token = lexer_->nextToken();
            }
        }

        // Expect ';'
        if (token.type != VerilogTokenType::Symbol || token.value != ";") {
            return false;
        }

        token = lexer_->nextToken();
    }

    return true;
}

bool VerilogParser::parseInstances(VerilogModule& module) {
    // Simplified instance parsing
    // Format: cell_type instance_name (.port(wire), ...);

    auto token = lexer_->nextToken();

    while (token.type != VerilogTokenType::EndOfInput &&
           (token.type != VerilogTokenType::Keyword ||
            (token.value != "endmodule" && token.value != "assign"))) {

        if (token.type == VerilogTokenType::Identifier) {
            // Could be a cell instantiation
            std::string cell_type = token.value;

            auto next_token = lexer_->nextToken();
            if (next_token.type == VerilogTokenType::Identifier) {
                std::string instance_name = next_token.value;

                // Parse port connections
                auto paren_token = lexer_->nextToken();
                if (paren_token.type == VerilogTokenType::Symbol &&
                    paren_token.value == "(") {

                    std::vector<std::pair<std::string, std::string>> connections;

                    auto conn_token = lexer_->nextToken();
                    while (conn_token.type != VerilogTokenType::Symbol ||
                           conn_token.value != ")") {

                        if (conn_token.type == VerilogTokenType::Symbol &&
                            conn_token.value == ".") {
                            // Named port connection: .port_name(wire_name)
                            auto port_token = lexer_->nextToken();
                            if (port_token.type == VerilogTokenType::Identifier) {
                                std::string port_name = port_token.value;

                                auto open_paren = lexer_->nextToken();
                                if (open_paren.type == VerilogTokenType::Symbol &&
                                    open_paren.value == "(") {

                                    auto wire_token = lexer_->nextToken();
                                    std::string wire_name = wire_token.value;

                                    connections.push_back({port_name, wire_name});

                                    auto close_paren = lexer_->nextToken();
                                    // Skip )
                                }
                            }
                        }

                        conn_token = lexer_->nextToken();
                        if (conn_token.type == VerilogTokenType::Symbol &&
                            conn_token.value == ",") {
                            conn_token = lexer_->nextToken();
                        }
                    }

                    module.instances.push_back({instance_name, cell_type, connections});

                    // Expect ';'
                    auto semi_token = lexer_->nextToken();
                }
            }
        }

        token = lexer_->nextToken();
    }

    return true;
}

bool VerilogParser::parseAssignStatements(VerilogModule& module) {
    // Simplified - would need full expression parser
    return true;
}

std::pair<double, double> VerilogParser::getCellDimensions(const std::string& cell_type) {
    // Simplified standard cell dimensions
    // In real flow, these come from LEF (Library Exchange Format)

    static const std::map<std::string, std::pair<double, double>> cell_sizes = {
        {"INVX1", {1.0, 2.0}},
        {"BUFX1", {1.0, 2.0}},
        {"NAND2X1", {1.2, 2.0}},
        {"NOR2X1", {1.2, 2.0}},
        {"AND2X1", {1.4, 2.0}},
        {"OR2X1", {1.4, 2.0}},
        {"XOR2X1", {2.0, 2.0}},
        {"DFFX1", {3.0, 2.0}},
        {"MUX2X1", {2.0, 2.0}},
        {"ADDFX1", {3.0, 2.0}}
    };

    auto it = cell_sizes.find(cell_type);
    if (it != cell_sizes.end()) {
        return it->second;
    }

    // Default size for unknown cells
    return {1.0, 2.0};
}

bool VerilogParser::convertToCircuitModel(const VerilogModule& vmodule) {
    current_circuit_->name = vmodule.name;

    // Create modules for inputs (as terminals)
    for (const auto& input : vmodule.inputs) {
        auto module = std::make_shared<Module>(input);
        module->type = ModuleType::TERMINAL;
        module->is_fixed = true;
        module->width = 0.1;
        module->height = 0.1;

        auto pin = std::make_shared<Pin>(input);
        pin->direction = PinDirection::OUTPUT;
        pin->parent_module = module;
        module->addPin(pin);

        current_circuit_->addModule(module);
    }

    // Create modules for outputs (as terminals)
    for (const auto& output : vmodule.outputs) {
        auto module = std::make_shared<Module>(output);
        module->type = ModuleType::TERMINAL;
        module->is_fixed = true;
        module->width = 0.1;
        module->height = 0.1;

        auto pin = std::make_shared<Pin>(output);
        pin->direction = PinDirection::INPUT;
        pin->parent_module = module;
        module->addPin(pin);

        current_circuit_->addModule(module);
    }

    // Create modules for cell instances
    for (const auto& inst : vmodule.instances) {
        const std::string& inst_name = std::get<0>(inst);
        const std::string& cell_type = std::get<1>(inst);
        const auto& connections = std::get<2>(inst);

        auto module = std::make_shared<Module>(inst_name);
        module->type = ModuleType::STANDARD_CELL;
        module->is_fixed = false;
        module->cell_type = cell_type;

        auto [width, height] = getCellDimensions(cell_type);
        module->width = width;
        module->height = height;

        // Create pins for each connection
        for (const auto& conn : connections) {
            const std::string& port_name = conn.first;
            const std::string& wire_name = conn.second;

            auto pin = std::make_shared<Pin>(port_name);
            pin->parent_module = module;

            // Infer direction from port name (simplified)
            if (port_name == "Y" || port_name == "Q" || port_name == "OUT") {
                pin->direction = PinDirection::OUTPUT;
            } else {
                pin->direction = PinDirection::INPUT;
            }

            module->addPin(pin);
        }

        current_circuit_->addModule(module);
    }

    // Create nets from connections
    // This is simplified - would need proper net extraction
    std::map<std::string, NetPtr> wire_to_net;

    for (const auto& inst : vmodule.instances) {
        const auto& connections = std::get<2>(inst);

        for (const auto& conn : connections) {
            const std::string& wire_name = conn.second;

            if (wire_to_net.find(wire_name) == wire_to_net.end()) {
                auto net = std::make_shared<Net>(wire_name);
                wire_to_net[wire_name] = net;
                current_circuit_->addNet(net);
            }
        }
    }

    // Set up die area (placeholder - would come from constraints)
    current_circuit_->die_area.x_min = 0;
    current_circuit_->die_area.y_min = 0;
    current_circuit_->die_area.x_max = 100;
    current_circuit_->die_area.y_max = 100;

    return true;
}

} // namespace eda
