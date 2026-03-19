#include <gtest/gtest.h>
#include "circuit_model.h"
#include "initial_placer.h"
#include "global_placer.h"

using namespace eda;

class PlacementTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create a simple test circuit
        circuit = std::make_unique<CircuitModel>("test_placement");

        // Set die area
        circuit->die_area.x_min = 0;
        circuit->die_area.y_min = 0;
        circuit->die_area.x_max = 100;
        circuit->die_area.y_max = 100;

        // Create fixed terminals
        auto term1 = std::make_shared<Module>("term1");
        term1->type = ModuleType::TERMINAL;
        term1->is_fixed = true;
        term1->width = 1;
        term1->height = 1;
        term1->position = Point2D(0, 50);

        auto term2 = std::make_shared<Module>("term2");
        term2->type = ModuleType::TERMINAL;
        term2->is_fixed = true;
        term2->width = 1;
        term2->height = 1;
        term2->position = Point2D(100, 50);

        // Create movable cells
        auto cell1 = std::make_shared<Module>("cell1");
        cell1->type = ModuleType::STANDARD_CELL;
        cell1->is_fixed = false;
        cell1->width = 2;
        cell1->height = 2;

        auto cell2 = std::make_shared<Module>("cell2");
        cell2->type = ModuleType::STANDARD_CELL;
        cell2->is_fixed = false;
        cell2->width = 2;
        cell2->height = 2;

        // Create pins
        auto pin_t1 = std::make_shared<Pin>("p1");
        pin_t1->parent_module = term1;
        pin_t1->position = Point2D(0.5, 50.5);
        term1->addPin(pin_t1);

        auto pin_t2 = std::make_shared<Pin>("p2");
        pin_t2->parent_module = term2;
        pin_t2->position = Point2D(100.5, 50.5);
        term2->addPin(pin_t2);

        auto pin_c1 = std::make_shared<Pin>("p3");
        pin_c1->parent_module = cell1;
        pin_c1->position = Point2D(1, 1);
        cell1->addPin(pin_c1);

        auto pin_c2 = std::make_shared<Pin>("p4");
        pin_c2->parent_module = cell2;
        pin_c2->position = Point2D(1, 1);
        cell2->addPin(pin_c2);

        // Create net
        auto net = std::make_shared<Net>("net1");
        net->addPin(pin_t1);
        net->addPin(pin_c1);
        net->addPin(pin_c2);
        net->addPin(pin_t2);

        // Add to circuit
        circuit->addModule(term1);
        circuit->addModule(term2);
        circuit->addModule(cell1);
        circuit->addModule(cell2);
        circuit->addNet(net);
    }

    std::unique_ptr<CircuitModel> circuit;
};

TEST_F(PlacementTest, InitialPlacerRuns) {
    InitialPlacer placer(*circuit);

    double hpwl_before = circuit->calcTotalHPWL();

    EXPECT_TRUE(placer.run());

    double hpwl_after = circuit->calcTotalHPWL();

    // HPWL should change after placement
    EXPECT_NE(hpwl_before, hpwl_after);

    // Check that cells are within die area
    for (const auto& module : circuit->modules) {
        if (!module->is_fixed) {
            EXPECT_GE(module->position.x, circuit->die_area.x_min);
            EXPECT_GE(module->position.y, circuit->die_area.y_min);
            EXPECT_LE(module->position.x + module->width, circuit->die_area.x_max);
            EXPECT_LE(module->position.y + module->height, circuit->die_area.y_max);
        }
    }
}

TEST_F(PlacementTest, GlobalPlacerRuns) {
    GlobalPlacer placer(*circuit);
    placer.setMaxIterations(50); // Limit iterations for test

    double hpwl_before = circuit->calcTotalHPWL();

    EXPECT_TRUE(placer.run());

    double hpwl_after = circuit->calcTotalHPWL();

    // HPWL should change
    EXPECT_NE(hpwl_before, hpwl_after);

    // Check stats
    const auto& stats = placer.getStats();
    EXPECT_GT(stats.iterations, 0);
    EXPECT_GE(stats.runtime_seconds, 0.0);
}

TEST_F(PlacementTest, PlacementProgressCallback) {
    GlobalPlacer placer(*circuit);
    placer.setMaxIterations(20);

    int callback_count = 0;
    placer.setProgressCallback([&callback_count](int iter, double hpwl, double overflow) {
        callback_count++;
        EXPECT_GT(iter, 0);
        EXPECT_GE(hpwl, 0);
        EXPECT_GE(overflow, 0);
    });

    placer.run();

    EXPECT_GT(callback_count, 0);
}
