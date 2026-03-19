#include <gtest/gtest.h>
#include "circuit_model.h"

using namespace eda;

class CircuitModelTest : public ::testing::Test {
protected:
    void SetUp() override {
        circuit = std::make_unique<CircuitModel>("test_design");
    }

    std::unique_ptr<CircuitModel> circuit;
};

TEST_F(CircuitModelTest, CreateModule) {
    auto module = std::make_shared<Module>("M1");
    module->width = 10.0;
    module->height = 20.0;
    module->type = ModuleType::STANDARD_CELL;

    circuit->addModule(module);

    EXPECT_EQ(circuit->getNumModules(), 1);
    EXPECT_NE(circuit->getModule("M1"), nullptr);
    EXPECT_EQ(circuit->getModule("M1")->width, 10.0);
}

TEST_F(CircuitModelTest, CreateNet) {
    auto net = std::make_shared<Net>("N1");
    circuit->addNet(net);

    EXPECT_EQ(circuit->getNumNets(), 1);
    EXPECT_NE(circuit->getNet("N1"), nullptr);
}

TEST_F(CircuitModelTest, PinConnection) {
    auto module1 = std::make_shared<Module>("M1");
    auto module2 = std::make_shared<Module>("M2");

    auto pin1 = std::make_shared<Pin>("P1");
    pin1->direction = PinDirection::OUTPUT;
    pin1->parent_module = module1;
    module1->addPin(pin1);

    auto pin2 = std::make_shared<Pin>("P2");
    pin2->direction = PinDirection::INPUT;
    pin2->parent_module = module2;
    module2->addPin(pin2);

    auto net = std::make_shared<Net>("N1");
    net->addPin(pin1);
    net->addPin(pin2);

    circuit->addModule(module1);
    circuit->addModule(module2);
    circuit->addNet(net);

    EXPECT_EQ(net->pins.size(), 2);
    EXPECT_EQ(net->calcHPWL(), 0.0); // Pins at same position initially
}

TEST_F(CircuitModelTest, HPWLCalculation) {
    auto module1 = std::make_shared<Module>("M1");
    module1->position = Point2D(0, 0);
    module1->width = 1;
    module1->height = 1;

    auto module2 = std::make_shared<Module>("M2");
    module2->position = Point2D(10, 20);
    module2->width = 1;
    module2->height = 1;

    auto pin1 = std::make_shared<Pin>("P1");
    pin1->parent_module = module1;
    pin1->position = Point2D(0.5, 0.5);
    module1->addPin(pin1);

    auto pin2 = std::make_shared<Pin>("P2");
    pin2->parent_module = module2;
    pin2->position = Point2D(10.5, 20.5);
    module2->addPin(pin2);

    auto net = std::make_shared<Net>("N1");
    net->addPin(pin1);
    net->addPin(pin2);

    circuit->addModule(module1);
    circuit->addModule(module2);
    circuit->addNet(net);

    double hpwl = net->calcHPWL();
    EXPECT_DOUBLE_EQ(hpwl, 30.0); // (10.5-0.5) + (20.5-0.5) = 10 + 20 = 30
}

TEST_F(CircuitModelTest, DieArea) {
    circuit->die_area.x_min = 0;
    circuit->die_area.y_min = 0;
    circuit->die_area.x_max = 100;
    circuit->die_area.y_max = 100;

    EXPECT_DOUBLE_EQ(circuit->die_area.width(), 100.0);
    EXPECT_DOUBLE_EQ(circuit->die_area.height(), 100.0);
    EXPECT_DOUBLE_EQ(circuit->die_area.area(), 10000.0);

    Point2D inside(50, 50);
    Point2D outside(150, 150);

    EXPECT_TRUE(circuit->die_area.contains(inside));
    EXPECT_FALSE(circuit->die_area.contains(outside));
}

TEST_F(CircuitModelTest, ModuleOverlap) {
    auto module1 = std::make_shared<Module>("M1");
    module1->position = Point2D(0, 0);
    module1->width = 10;
    module1->height = 10;

    auto module2 = std::make_shared<Module>("M2");
    module2->position = Point2D(5, 5);
    module2->width = 10;
    module2->height = 10;

    auto module3 = std::make_shared<Module>("M3");
    module3->position = Point2D(20, 20);
    module3->width = 5;
    module3->height = 5;

    EXPECT_TRUE(module1->overlaps(*module2));
    EXPECT_FALSE(module1->overlaps(*module3));
}

TEST_F(CircuitModelTest, ClearCircuit) {
    auto module = std::make_shared<Module>("M1");
    auto net = std::make_shared<Net>("N1");

    circuit->addModule(module);
    circuit->addNet(net);

    EXPECT_EQ(circuit->getNumModules(), 1);
    EXPECT_EQ(circuit->getNumNets(), 1);

    circuit->clear();

    EXPECT_EQ(circuit->getNumModules(), 0);
    EXPECT_EQ(circuit->getNumNets(), 0);
}
