#include <gtest/gtest.h>
#include <QApplication>
#include "main_window.h"

using namespace eda;

class GUITest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create QApplication if not exists
        if (!qApp) {
            int argc = 1;
            char arg0[] = "test";
            char* argv[] = {arg0};
            app_ = std::make_unique<QApplication>(argc, argv);
        }
    }

    std::unique_ptr<QApplication> app_;
};

TEST_F(GUITest, MainWindowCreation) {
    MainWindow window;
    EXPECT_NO_THROW(window.show());
}

TEST_F(GUITest, CanvasCreation) {
    CircuitCanvas canvas;
    EXPECT_NE(canvas.scene(), nullptr);
}

TEST_F(GUITest, PlacementViewCreation) {
    PlacementView view;
    EXPECT_NE(view.scene(), nullptr);
}

TEST_F(GUITest, AlgorithmPanelCreation) {
    AlgorithmPanel panel;
    EXPECT_TRUE(panel.isVisible());
}
