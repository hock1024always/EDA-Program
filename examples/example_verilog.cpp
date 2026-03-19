#include <iostream>
#include "circuit_model.h"
#include "verilog_parser.h"

using namespace eda;

int main(int argc, char* argv[]) {
    std::cout << "EDA Integrated - Verilog Parser Example\n";
    std::cout << "=======================================\n\n";

    if (argc < 2) {
        std::cout << "Usage: " << argv[0] << " <verilog_file>\n";
        std::cout << "\nExample:\n";
        std::cout << "  " << argv[0] << " /path/to/design.v\n";
        return 1;
    }

    std::string v_file = argv[1];

    // Create circuit model
    CircuitModel circuit;

    // Parse Verilog file
    VerilogParser parser;
    if (!parser.parse(v_file, circuit)) {
        std::cerr << "Failed to parse: " << parser.getLastError() << "\n";
        return 1;
    }

    // Print circuit information
    std::cout << "Module Name: " << circuit.name << "\n\n";

    std::cout << "Circuit Elements:\n";
    std::cout << "  Modules (cells): " << circuit.getNumModules() << "\n";
    std::cout << "  Nets: " << circuit.getNumNets() << "\n";
    std::cout << "  Pins: " << circuit.getNumPins() << "\n\n";

    // List modules by type
    std::cout << "Module List:\n";
    for (const auto& module : circuit.modules) {
        std::cout << "  " << module->name;
        if (!module->cell_type.empty()) {
            std::cout << " [" << module->cell_type << "]";
        }
        std::cout << " - ";
        switch (module->type) {
            case ModuleType::STANDARD_CELL:
                std::cout << "Standard Cell";
                break;
            case ModuleType::TERMINAL:
                std::cout << "Terminal";
                break;
            case ModuleType::MACRO:
                std::cout << "Macro";
                break;
            default:
                std::cout << "Unknown";
        }
        if (module->is_fixed) {
            std::cout << " (Fixed)";
        }
        std::cout << "\n";
    }

    std::cout << "\nNet List:\n";
    int net_count = 0;
    for (const auto& net : circuit.nets) {
        std::cout << "  " << net->name << " (degree: " << net->pins.size() << ")\n";
        if (++net_count >= 10) {
            std::cout << "  ... and " << (circuit.getNumNets() - 10) << " more nets\n";
            break;
        }
    }

    return 0;
}
