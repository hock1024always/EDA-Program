#include <gtest/gtest.h>
#include <QApplication>

int main(int argc, char* argv[]) {
    ::testing::InitGoogleTest(&argc, argv);
    
    // Create QApplication for GUI tests
    QApplication app(argc, argv);
    
    return RUN_ALL_TESTS();
}
