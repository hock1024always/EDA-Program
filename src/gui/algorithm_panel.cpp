#include "algorithm_panel.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QPushButton>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QCheckBox>
#include <QLabel>
#include <QProgressBar>

namespace eda {

AlgorithmPanel::AlgorithmPanel(QWidget* parent)
    : QWidget(parent) {
    createLayout();
}

void AlgorithmPanel::createLayout() {
    main_layout_ = new QVBoxLayout(this);
    main_layout_->setSpacing(10);

    createInitialGroup();
    createGlobalGroup();
    createControlGroup();

    main_layout_->addStretch(1);
}

void AlgorithmPanel::createInitialGroup() {
    initial_group_ = new QGroupBox("Initial Placement (Kraftwerk2A)", this);
    auto* layout = new QFormLayout(initial_group_);

    max_iter_spin_ = new QSpinBox(this);
    max_iter_spin_->setRange(1, 1000);
    max_iter_spin_->setValue(100);
    layout->addRow("Max Iterations:", max_iter_spin_);

    error_spin_ = new QDoubleSpinBox(this);
    error_spin_->setRange(1e-6, 1e-1);
    error_spin_->setValue(1e-4);
    error_spin_->setDecimals(6);
    layout->addRow("Target Error:", error_spin_);

    initial_place_btn_ = new QPushButton("Run Initial Placement", this);
    connect(initial_place_btn_, &QPushButton::clicked,
            this, &AlgorithmPanel::onInitialPlaceClicked);

    auto* btn_layout = new QHBoxLayout;
    btn_layout->addStretch();
    btn_layout->addWidget(initial_place_btn_);
    layout->addRow(btn_layout);

    main_layout_->addWidget(initial_group_);
}

void AlgorithmPanel::createGlobalGroup() {
    global_group_ = new QGroupBox("Global Placement (ePlace-MS)", this);
    auto* layout = new QFormLayout(global_group_);

    density_spin_ = new QDoubleSpinBox(this);
    density_spin_->setRange(0.1, 1.0);
    density_spin_->setValue(1.0);
    density_spin_->setSingleStep(0.05);
    layout->addRow("Target Density:", density_spin_);

    overflow_spin_ = new QDoubleSpinBox(this);
    overflow_spin_->setRange(0.0, 1.0);
    overflow_spin_->setValue(0.1);
    overflow_spin_->setSingleStep(0.01);
    layout->addRow("Target Overflow:", overflow_spin_);

    global_iter_spin_ = new QSpinBox(this);
    global_iter_spin_->setRange(1, 1000);
    global_iter_spin_->setValue(200);
    layout->addRow("Max Iterations:", global_iter_spin_);

    global_place_btn_ = new QPushButton("Run Global Placement", this);
    connect(global_place_btn_, &QPushButton::clicked,
            this, &AlgorithmPanel::onGlobalPlaceClicked);

    auto* btn_layout = new QHBoxLayout;
    btn_layout->addStretch();
    btn_layout->addWidget(global_place_btn_);
    layout->addRow(btn_layout);

    main_layout_->addWidget(global_group_);
}

void AlgorithmPanel::createControlGroup() {
    control_group_ = new QGroupBox("Control", this);
    auto* layout = new QVBoxLayout(control_group_);

    full_flow_btn_ = new QPushButton("Run Full Flow", this);
    full_flow_btn_->setStyleSheet("font-weight: bold;");
    connect(full_flow_btn_, &QPushButton::clicked,
            this, &AlgorithmPanel::onFullFlowClicked);

    auto* btn_layout = new QHBoxLayout;
    btn_layout->addStretch();
    btn_layout->addWidget(full_flow_btn_);
    layout->addLayout(btn_layout);

    progress_bar_ = new QProgressBar(this);
    progress_bar_->setVisible(false);
    layout->addWidget(progress_bar_);

    status_label_ = new QLabel("Ready", this);
    status_label_->setAlignment(Qt::AlignCenter);
    layout->addWidget(status_label_);

    main_layout_->addWidget(control_group_);
}

void AlgorithmPanel::onInitialPlaceClicked() {
    status_label_->setText("Running initial placement...");
    progress_bar_->setVisible(true);
    progress_bar_->setRange(0, 0); // Indeterminate

    emit initialPlacementRequested();
}

void AlgorithmPanel::onGlobalPlaceClicked() {
    status_label_->setText("Running global placement...");
    progress_bar_->setVisible(true);
    progress_bar_->setRange(0, 0); // Indeterminate

    emit globalPlacementRequested();
}

void AlgorithmPanel::onFullFlowClicked() {
    status_label_->setText("Running full placement flow...");
    progress_bar_->setVisible(true);
    progress_bar_->setRange(0, 0); // Indeterminate

    emit fullFlowRequested();
}

} // namespace eda
