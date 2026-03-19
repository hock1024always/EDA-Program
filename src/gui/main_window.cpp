#include "main_window.h"
#include "circuit_canvas.h"
#include "placement_view.h"
#include "algorithm_panel.h"
#include "progress_dialog.h"
#include "bookshelf_parser.h"
#include "verilog_parser.h"
#include <QApplication>
#include <QMenuBar>
#include <QToolBar>
#include <QStatusBar>
#include <QDockWidget>
#include <QFileDialog>
#include <QMessageBox>
#include <QCloseEvent>
#include <QSplitter>
#include <QVBoxLayout>
#include <QLabel>

namespace eda {

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent) {
    setWindowTitle("EDA Integrated");
    resize(1200, 800);

    // Create central widget with splitter
    auto* splitter = new QSplitter(Qt::Horizontal, this);
    setCentralWidget(splitter);

    // Create views
    circuit_canvas_ = new CircuitCanvas(this);
    placement_view_ = new PlacementView(this);

    splitter->addWidget(circuit_canvas_);
    splitter->addWidget(placement_view_);
    splitter->setSizes({600, 600});

    // Create dock windows
    createDockWindows();

    // Create UI components
    createActions();
    createMenus();
    createToolBars();
    createStatusBar();
    connectSignals();

    // Initial state
    updateMenus();
}

MainWindow::~MainWindow() = default;

void MainWindow::createActions() {
    // File actions
    new_act_ = new QAction(QIcon::fromTheme("document-new"), "&New", this);
    new_act_->setShortcut(QKeySequence::New);
    new_act_->setStatusTip("Create a new circuit");
    connect(new_act_, &QAction::triggered, this, &MainWindow::newCircuit);

    open_act_ = new QAction(QIcon::fromTheme("document-open"), "&Open...", this);
    open_act_->setShortcut(QKeySequence::Open);
    open_act_->setStatusTip("Open an existing file");
    connect(open_act_, &QAction::triggered, this, &MainWindow::openFile);

    save_act_ = new QAction(QIcon::fromTheme("document-save"), "&Save", this);
    save_act_->setShortcut(QKeySequence::Save);
    save_act_->setStatusTip("Save the current file");
    connect(save_act_, &QAction::triggered, this, &MainWindow::saveFile);

    save_as_act_ = new QAction(QIcon::fromTheme("document-save-as"), "Save &As...", this);
    save_as_act_->setShortcut(QKeySequence::SaveAs);
    save_as_act_->setStatusTip("Save the file with a new name");
    connect(save_as_act_, &QAction::triggered, this, &MainWindow::saveAsFile);

    export_placement_act_ = new QAction("Export &Placement...", this);
    export_placement_act_->setStatusTip("Export placement to .pl file");
    connect(export_placement_act_, &QAction::triggered, this, &MainWindow::exportPlacement);

    export_image_act_ = new QAction("Export &Image...", this);
    export_image_act_->setStatusTip("Export layout as image");
    connect(export_image_act_, &QAction::triggered, this, &MainWindow::exportImage);

    exit_act_ = new QAction("E&xit", this);
    exit_act_->setShortcut(QKeySequence::Quit);
    exit_act_->setStatusTip("Exit the application");
    connect(exit_act_, &QAction::triggered, this, &QWidget::close);

    // Placement actions
    initial_place_act_ = new QAction("Initial &Placement", this);
    initial_place_act_->setStatusTip("Run initial placement (Kraftwerk2A)");
    connect(initial_place_act_, &QAction::triggered, this, &MainWindow::runInitialPlacement);

    global_place_act_ = new QAction("&Global Placement", this);
    global_place_act_->setStatusTip("Run global placement (ePlace-MS)");
    connect(global_place_act_, &QAction::triggered, this, &MainWindow::runGlobalPlacement);

    full_flow_act_ = new QAction("&Full Flow", this);
    full_flow_act_->setStatusTip("Run both initial and global placement");
    connect(full_flow_act_, &QAction::triggered, this, &MainWindow::runFullFlow);

    // View actions
    zoom_in_act_ = new QAction(QIcon::fromTheme("zoom-in"), "Zoom &In", this);
    zoom_in_act_->setShortcut(QKeySequence::ZoomIn);
    connect(zoom_in_act_, &QAction::triggered, this, &MainWindow::zoomIn);

    zoom_out_act_ = new QAction(QIcon::fromTheme("zoom-out"), "Zoom &Out", this);
    zoom_out_act_->setShortcut(QKeySequence::ZoomOut);
    connect(zoom_out_act_, &QAction::triggered, this, &MainWindow::zoomOut);

    fit_view_act_ = new QAction(QIcon::fromTheme("zoom-fit-best"), "&Fit to View", this);
    fit_view_act_->setShortcut(Qt::CTRL | Qt::Key_0);
    connect(fit_view_act_, &QAction::triggered, this, &MainWindow::fitToView);

    toggle_grid_act_ = new QAction("&Grid", this);
    toggle_grid_act_->setCheckable(true);
    toggle_grid_act_->setChecked(true);
    connect(toggle_grid_act_, &QAction::triggered, this, &MainWindow::toggleGrid);

    // Help actions
    about_act_ = new QAction("&About", this);
    connect(about_act_, &QAction::triggered, this, &MainWindow::showAbout);
}

void MainWindow::createMenus() {
    // File menu
    file_menu_ = menuBar()->addMenu("&File");
    file_menu_->addAction(new_act_);
    file_menu_->addAction(open_act_);
    file_menu_->addSeparator();
    file_menu_->addAction(save_act_);
    file_menu_->addAction(save_as_act_);
    file_menu_->addSeparator();
    file_menu_->addAction(export_placement_act_);
    file_menu_->addAction(export_image_act_);
    file_menu_->addSeparator();
    file_menu_->addAction(exit_act_);

    // Edit menu
    edit_menu_ = menuBar()->addMenu("&Edit");
    // Will add cut/copy/paste actions later

    // View menu
    view_menu_ = menuBar()->addMenu("&View");
    view_menu_->addAction(zoom_in_act_);
    view_menu_->addAction(zoom_out_act_);
    view_menu_->addAction(fit_view_act_);
    view_menu_->addSeparator();
    view_menu_->addAction(toggle_grid_act_);

    // Placement menu
    placement_menu_ = menuBar()->addMenu("&Placement");
    placement_menu_->addAction(initial_place_act_);
    placement_menu_->addAction(global_place_act_);
    placement_menu_->addAction(full_flow_act_);

    // Tools menu
    tools_menu_ = menuBar()->addMenu("&Tools");
    // Will add preference/tools later

    // Help menu
    help_menu_ = menuBar()->addMenu("&Help");
    help_menu_->addAction(about_act_);
}

void MainWindow::createToolBars() {
    // File toolbar
    file_toolbar_ = addToolBar("File");
    file_toolbar_->addAction(new_act_);
    file_toolbar_->addAction(open_act_);
    file_toolbar_->addAction(save_act_);

    // View toolbar
    view_toolbar_ = addToolBar("View");
    view_toolbar_->addAction(zoom_in_act_);
    view_toolbar_->addAction(zoom_out_act_);
    view_toolbar_->addAction(fit_view_act_);

    // Placement toolbar
    placement_toolbar_ = addToolBar("Placement");
    placement_toolbar_->addAction(initial_place_act_);
    placement_toolbar_->addAction(global_place_act_);
    placement_toolbar_->addAction(full_flow_act_);
}

void MainWindow::createStatusBar() {
    statusBar()->showMessage("Ready");
}

void MainWindow::createDockWindows() {
    // Algorithm control panel
    auto* algorithm_dock = new QDockWidget("Algorithms", this);
    algorithm_dock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    algorithm_panel_ = new AlgorithmPanel(this);
    algorithm_dock->setWidget(algorithm_panel_);
    addDockWidget(Qt::RightDockWidgetArea, algorithm_dock);
}

void MainWindow::connectSignals() {
    // Connect algorithm panel signals
    connect(algorithm_panel_, &AlgorithmPanel::initialPlacementRequested,
            this, &MainWindow::runInitialPlacement);
    connect(algorithm_panel_, &AlgorithmPanel::globalPlacementRequested,
            this, &MainWindow::runGlobalPlacement);
    connect(algorithm_panel_, &AlgorithmPanel::fullFlowRequested,
            this, &MainWindow::runFullFlow);

    // Connect view signals
    connect(circuit_canvas_, &CircuitCanvas::mouseMoved,
            [this](const QPoint& pos) {
                statusBar()->showMessage(QString("Mouse: (%1, %2)")
                                         .arg(pos.x()).arg(pos.y()));
            });
}

void MainWindow::newCircuit() {
    if (modified_) {
        QMessageBox::StandardButton ret = QMessageBox::warning(
            this, tr("EDA Integrated"),
            tr("The document has been modified.\nDo you want to save your changes?"),
            QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);

        if (ret == QMessageBox::Save) {
            saveFile();
        } else if (ret == QMessageBox::Cancel) {
            return;
        }
    }

    circuit_ = std::make_unique<CircuitModel>("Untitled");
    current_file_.clear();
    modified_ = false;
    updateWindowTitle();
    updateMenus();
    emit circuitLoaded();
}

void MainWindow::openFile() {
    QString fileName = QFileDialog::getOpenFileName(
        this, tr("Open Circuit File"), "",
        tr("BookShelf Files (*.aux);;Verilog Files (*.v *.sv);;All Files (*)"));

    if (!fileName.isEmpty()) {
        loadCircuit(fileName.toStdString());
    }
}

void MainWindow::loadCircuit(const std::string& filename) {
    // Determine file type
    std::string ext;
    size_t dot_pos = filename.find_last_of('.');
    if (dot_pos != std::string::npos) {
        ext = filename.substr(dot_pos + 1);
    }

    circuit_ = std::make_unique<CircuitModel>();

    bool success = false;
    if (ext == "aux" || ext == "nodes") {
        BookShelfParser parser;
        success = parser.parse(filename, *circuit_);
    } else if (ext == "v" || ext == "sv") {
        VerilogParser parser;
        success = parser.parse(filename, *circuit_);
    }

    if (success) {
        current_file_ = QString::fromStdString(filename);
        modified_ = false;
        updateWindowTitle();
        updateMenus();

        // Update views
        circuit_canvas_->setCircuit(circuit_.get());
        placement_view_->setCircuit(circuit_.get());

        statusBar()->showMessage(QString("Loaded %1").arg(current_file_));
        emit circuitLoaded();
    } else {
        QMessageBox::critical(this, tr("Error"),
                             tr("Failed to load file: %1").arg(QString::fromStdString(filename)));
    }
}

void MainWindow::setCircuit(std::unique_ptr<CircuitModel> circuit) {
    circuit_ = std::move(circuit);
    modified_ = true;
    updateWindowTitle();
    updateMenus();

    // Update views
    circuit_canvas_->setCircuit(circuit_.get());
    placement_view_->setCircuit(circuit_.get());

    emit circuitLoaded();
}

void MainWindow::saveFile() {
    if (current_file_.isEmpty()) {
        saveAsFile();
    } else {
        // TODO: Implement save functionality
        modified_ = false;
        updateWindowTitle();
        statusBar()->showMessage("Saved");
    }
}

void MainWindow::saveAsFile() {
    QString fileName = QFileDialog::getSaveFileName(
        this, tr("Save Circuit"), "",
        tr("BookShelf Files (*.pl);;All Files (*)"));

    if (!fileName.isEmpty()) {
        current_file_ = fileName;
        saveFile();
    }
}

void MainWindow::exportPlacement() {
    if (!circuit_) {
        QMessageBox::warning(this, tr("Warning"), tr("No circuit loaded"));
        return;
    }

    QString fileName = QFileDialog::getSaveFileName(
        this, tr("Export Placement"), "",
        tr("Placement Files (*.pl);;All Files (*)"));

    if (!fileName.isEmpty()) {
        std::string filename = fileName.toStdString();
        if (circuit_->exportToPl(filename)) {
            statusBar()->showMessage(QString("Exported to %1").arg(fileName));
        } else {
            QMessageBox::critical(this, tr("Error"), tr("Failed to export placement"));
        }
    }
}

void MainWindow::exportImage() {
    // TODO: Implement image export
    QMessageBox::information(this, tr("Not Implemented"),
                            tr("Image export is not yet implemented"));
}

void MainWindow::runInitialPlacement() {
    if (!circuit_) {
        QMessageBox::warning(this, tr("Warning"), tr("No circuit loaded"));
        return;
    }

    emit placementStarted();
    // TODO: Run placement in separate thread
    emit placementFinished();
}

void MainWindow::runGlobalPlacement() {
    if (!circuit_) {
        QMessageBox::warning(this, tr("Warning"), tr("No circuit loaded"));
        return;
    }

    emit placementStarted();
    // TODO: Run placement in separate thread
    emit placementFinished();
}

void MainWindow::runFullFlow() {
    if (!circuit_) {
        QMessageBox::warning(this, tr("Warning"), tr("No circuit loaded"));
        return;
    }

    emit placementStarted();
    // TODO: Run placement in separate thread
    emit placementFinished();
}

void MainWindow::zoomIn() {
    circuit_canvas_->zoomIn();
    placement_view_->zoomIn();
}

void MainWindow::zoomOut() {
    circuit_canvas_->zoomOut();
    placement_view_->zoomOut();
}

void MainWindow::fitToView() {
    circuit_canvas_->fitToView();
    placement_view_->fitToView();
}

void MainWindow::toggleGrid() {
    circuit_canvas_->setShowGrid(toggle_grid_act_->isChecked());
    placement_view_->setShowGrid(toggle_grid_act_->isChecked());
}

void MainWindow::showAbout() {
    QMessageBox::about(this, tr("About EDA Integrated"),
                       tr("<h3>EDA Integrated</h3>"
                          "<p>An integrated Electronic Design Automation tool</p>"
                          "<p>Version 1.0.0</p>"));
}

void MainWindow::updateWindowTitle() {
    QString title = "EDA Integrated";
    if (!current_file_.isEmpty()) {
        title += " - " + QFileInfo(current_file_).fileName();
    }
    if (modified_) {
        title += "*";
    }
    setWindowTitle(title);
}

void MainWindow::updateMenus() {
    bool has_circuit = circuit_ != nullptr;

    save_act_->setEnabled(has_circuit);
    save_as_act_->setEnabled(has_circuit);
    export_placement_act_->setEnabled(has_circuit);
    export_image_act_->setEnabled(has_circuit);

    initial_place_act_->setEnabled(has_circuit);
    global_place_act_->setEnabled(has_circuit);
    full_flow_act_->setEnabled(has_circuit);
}

void MainWindow::updateStatusMessage(const QString& message) {
    statusBar()->showMessage(message);
}

void MainWindow::closeEvent(QCloseEvent* event) {
    if (modified_) {
        QMessageBox::StandardButton ret = QMessageBox::warning(
            this, tr("EDA Integrated"),
            tr("The document has been modified.\nDo you want to save your changes?"),
            QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);

        if (ret == QMessageBox::Save) {
            saveFile();
            if (modified_) { // Save was cancelled
                event->ignore();
                return;
            }
        } else if (ret == QMessageBox::Cancel) {
            event->ignore();
            return;
        }
    }

    event->accept();
}

} // namespace eda
