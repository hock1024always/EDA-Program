#include <iostream>
#include "circuit_model.h"
#include "bookshelf_parser.h"

using namespace eda;

int main(int argc, char* argv[]) {
    std::cout << "EDA Integrated - BookShelf Parser Example\n";
    std::cout << "=========================================\n\n";

    if (argc < 2) {
        std::cout << "Usage: " << argv[0] << " <aux_file>\n";
        std::cout << "\nExample:\n";
        std::cout << "  " << argv[0] << " /path/to/design.aux\n";
        return 1;
    }

    std::string aux_file = argv[1];

    // Create circuit model
    CircuitModel circuit;

    // Parse BookShelf files
    BookShelfParser parser;
    if (!parser.parse(aux_file, circuit)) {
        std::cerr << "Failed to parse: " << parser.getLastError() << "\n";
        return 1;
    }

    // Print detailed statistics
    std::cout << "Design Name: " << circuit.name << "\n\n";

    std::cout << "Core Region:\n";
    std::cout << "  Lower left: (" << circuit.die_area.x_min << ", "
              << circuit.die_area.y_min << ")\n";
    std::cout << "  Upper right: (" << circuit.die_area.x_max << ", "
              << circuit.die_area.y_max << ")\n";
    std::cout << "  Dimensions: " << circuit.die_area.width() << " x "
              << circuit.die_area.height() << "\n\n";

    std::cout << "Row Information:\n";
    std::cout << "  Number of rows: " << circuit.rows.size() << "\n";
    if (!circuit.rows.empty()) {
        std::cout << "  Row height: " << circuit.row_height << "\n";
        std::cout << "  Site width: " << circuit.site_width << "\n";
    }
    std::cout << "\n";

    std::cout << "Module Statistics:\n";
    std::cout << "  Total modules: " << circuit.getNumModules() << "\n";
    std::cout << "  Movable: " << circuit.getNumMovableModules() << "\n";
    std::cout << "  Fixed: " << circuit.getNumFixedModules() << "\n";
    std::cout << "\n";

    std::cout << "Net Statistics:\n";
    std::cout << "  Total nets: " << circuit.getNumNets() << "\n";
    std::cout << "  Total pins: " << circuit.getNumPins() << "\n";

    // Calculate initial HPWL
    double hpwl = circuit.calcTotalHPWL();
    std::cout << "  Total HPWL: " << hpwl << "\n";

    // Calculate utilization
    double utilization = circuit.calcUtilization();
    std::cout << "  Utilization: " << (utilization * 100.0) << "%\n";

    return 0;
}
