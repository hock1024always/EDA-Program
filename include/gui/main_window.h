#ifndef EDA_GUI_MAIN_WINDOW_H
#define EDA_GUI_MAIN_WINDOW_H

#include <QMainWindow>
#include <memory>
#include "circuit_model.h"

QT_BEGIN_NAMESPACE
class QAction;
class QMenu;
class QToolBar;
class QStatusBar;
class QDockWidget;
QT_END_NAMESPACE

namespace eda {

// Forward declarations
class CircuitCanvas;
class PlacementView;
class AlgorithmPanel;
class ProgressDialog;

/**
 * @brief 主窗口类
 *
 * 包含菜单栏、工具栏、状态栏以及主要的视图组件
 */
class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

    // Public interface
    void loadCircuit(const std::string& filename);
    void setCircuit(std::unique_ptr<CircuitModel> circuit);

signals:
    void circuitLoaded();
    void placementStarted();
    void placementFinished();

protected:
    void closeEvent(QCloseEvent* event) override;

private slots:
    // Menu actions
    void newCircuit();
    void openFile();
    void saveFile();
    void saveAsFile();
    void exportPlacement();
    void exportImage();
    
    void runInitialPlacement();
    void runGlobalPlacement();
    void runFullFlow();
    
    void showPreferences();
    void showAbout();

    // View actions
    void zoomIn();
    void zoomOut();
    void fitToView();
    void toggleGrid();

    // Update UI
    void updateWindowTitle();
    void updateMenus();
    void updateStatusMessage(const QString& message);

private:
    // UI setup
    void createActions();
    void createMenus();
    void createToolBars();
    void createStatusBar();
    void createDockWindows();
    void connectSignals();

    // State
    std::unique_ptr<CircuitModel> circuit_;
    QString current_file_;
    bool modified_ = false;

    // Child widgets
    CircuitCanvas* circuit_canvas_ = nullptr;
    PlacementView* placement_view_ = nullptr;
    AlgorithmPanel* algorithm_panel_ = nullptr;
    ProgressDialog* progress_dialog_ = nullptr;

    // Actions
    QAction* new_act_ = nullptr;
    QAction* open_act_ = nullptr;
    QAction* save_act_ = nullptr;
    QAction* save_as_act_ = nullptr;
    QAction* export_placement_act_ = nullptr;
    QAction* export_image_act_ = nullptr;
    QAction* exit_act_ = nullptr;

    QAction* initial_place_act_ = nullptr;
    QAction* global_place_act_ = nullptr;
    QAction* full_flow_act_ = nullptr;

    QAction* zoom_in_act_ = nullptr;
    QAction* zoom_out_act_ = nullptr;
    QAction* fit_view_act_ = nullptr;
    QAction* toggle_grid_act_ = nullptr;

    QAction* preferences_act_ = nullptr;
    QAction* about_act_ = nullptr;

    // Menus
    QMenu* file_menu_ = nullptr;
    QMenu* edit_menu_ = nullptr;
    QMenu* view_menu_ = nullptr;
    QMenu* placement_menu_ = nullptr;
    QMenu* tools_menu_ = nullptr;
    QMenu* help_menu_ = nullptr;

    // Toolbars
    QToolBar* file_toolbar_ = nullptr;
    QToolBar* view_toolbar_ = nullptr;
    QToolBar* placement_toolbar_ = nullptr;
};

} // namespace eda

#endif // EDA_GUI_MAIN_WINDOW_H
