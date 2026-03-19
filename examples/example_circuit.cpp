#include <iostream>
#include "circuit_model.h"

using namespace eda;

int main() {
    std::cout << "EDA Integrated - Circuit Model Example\n";
    std::cout << "======================================\n\n";

    // Create a simple circuit
    CircuitModel circuit("simple_adder");

    // Set die area
    circuit.die_area.x_min = 0;
    circuit.die_area.y_min = 0;
    circuit.die_area.x_max = 100;
    circuit.die_area.y_max = 100;

    // Create input terminals
    auto in_a = std::make_shared<Module>("A");
    in_a->type = ModuleType::TERMINAL;
    in_a->is_fixed = true;
    in_a->width = 0.1;
    in_a->height = 0.1;
    in_a->position = Point2D(0, 50);

    auto in_b = std::make_shared<Module>("B");
    in_b->type = ModuleType::TERMINAL;
    in_b->is_fixed = true;
    in_b->width = 0.1;
    in_b->height = 0.1;
    in_b->position = Point2D(0, 40);

    auto out_y = std::make_shared<Module>("Y");
    out_y->type = ModuleType::TERMINAL;
    out_y->is_fixed = true;
    out_y->width = 0.1;
    out_y->height = 0.1;
    out_y->position = Point2D(100, 45);

    // Create gates
    auto xor1 = std::make_shared<Module>("xor1");
    xor1->type = ModuleType::STANDARD_CELL;
    xor1->cell_type = "XOR2X1";
    xor1->width = 2.0;
    xor1->height = 2.0;
    xor1->position = Point2D(40, 45);

    // Create pins
    auto pin_a = std::make_shared<Pin>("A");
    pin_a->direction = PinDirection::OUTPUT;
    pin_a->parent_module = in_a;
    pin_a->position = Point2D(0, 50);
    in_a->addPin(pin_a);

    auto pin_b = std::make_shared<Pin>("B");
    pin_b->direction = PinDirection::OUTPUT;
    pin_b->parent_module = in_b;
    pin_b->position = Point2D(0, 40);
    in_b->addPin(pin_b);

    auto pin_xor_a = std::make_shared<Pin>("A");
    pin_xor_a->direction = PinDirection::INPUT;
    pin_xor_a->parent_module = xor1;
    pin_xor_a->position = Point2D(40, 46);
    xor1->addPin(pin_xor_a);

    auto pin_xor_b = std::make_shared<Pin>("B");
    pin_xor_b->direction = PinDirection::INPUT;
    pin_xor_b->parent_module = xor1;
    pin_xor_b->position = Point2D(40, 44);
    xor1->addPin(pin_xor_b);

    auto pin_xor_y = std::make_shared<Pin>("Y");
    pin_xor_y->direction = PinDirection::OUTPUT;
    pin_xor_y->parent_module = xor1;
    pin_xor_y->position = Point2D(42, 45);
    xor1->addPin(pin_xor_y);

    auto pin_y = std::make_shared<Pin>("Y");
    pin_y->direction = PinDirection::INPUT;
    pin_y->parent_module = out_y;
    pin_y->position = Point2D(100, 45);
    out_y->addPin(pin_y);

    // Create nets
    auto net_a = std::make_shared<Net>("net_a");
    net_a->addPin(pin_a);
    net_a->addPin(pin_xor_a);

    auto net_b = std::make_shared<Net>("net_b");
    net_b->addPin(pin_b);
    net_b->addPin(pin_xor_b);

    auto net_y = std::make_shared<Net>("net_y");
    net_y->addPin(pin_xor_y);
    net_y->addPin(pin_y);

    // Add to circuit
    circuit.addModule(in_a);
    circuit.addModule(in_b);
    circuit.addModule(out_y);
    circuit.addModule(xor1);

    circuit.addNet(net_a);
    circuit.addNet(net_b);
    circuit.addNet(net_y);

    // Print circuit information
    std::cout << "Circuit: " << circuit.name << "\n";
    std::cout << "Die Area: " << circuit.die_area.width() << " x "
              << circuit.die_area.height() << "\n\n";

    std::cout << "Modules:\n";
    for (const auto& mod : circuit.modules) {
        std::cout << "  " << mod->name;
        if (!mod->cell_type.empty()) {
            std::cout << " [" << mod->cell_type << "]";
        }
        std::cout << " at (" << mod->position.x << ", " << mod->position.y << ")";
        if (mod->is_fixed) {
            std::cout << " [FIXED]";
        }
        std::cout << "\n";
    }

    std::cout << "\nNets:\n";
    for (const auto& net : circuit.nets) {
        std::cout << "  " << net->name << ": ";
        for (size_t i = 0; i < net->pins.size(); ++i) {
            if (i > 0) std::cout << " - ";
            std::cout << net->pins[i]->parent_module->name;
        }
        std::cout << " (HPWL: " << net->calcHPWL() << ")\n";
    }

    std::cout << "\nTotal HPWL: " << circuit.calcTotalHPWL() << "\n";

    // Test validation
    std::cout << "\nValidation: " << (circuit.validate() ? "PASSED" : "FAILED") << "\n";

    return 0;
}
