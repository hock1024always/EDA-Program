#ifndef EDA_GUI_ALGORITHM_PANEL_H
#define EDA_GUI_ALGORITHM_PANEL_H

#include <QWidget>
#include <memory>

QT_BEGIN_NAMESPACE
class QPushButton;
class QGroupBox;
class QVBoxLayout;
class QHBoxLayout;
class QSpinBox;
class QDoubleSpinBox;
class QCheckBox;
class QLabel;
class QProgressBar;
QT_END_NAMESPACE

namespace eda {

/**
 * @brief 算法控制面板
 *
 * 提供算法参数设置和执行控制
 */
class AlgorithmPanel : public QWidget {
    Q_OBJECT

public:
    explicit AlgorithmPanel(QWidget* parent = nullptr);
    ~AlgorithmPanel() override = default;

signals:
    void initialPlacementRequested();
    void globalPlacementRequested();
    void fullFlowRequested();

private slots:
    void onInitialPlaceClicked();
    void onGlobalPlaceClicked();
    void onFullFlowClicked();

private:
    void createLayout();
    void createInitialGroup();
    void createGlobalGroup();
    void createControlGroup();

    // UI Components
    QVBoxLayout* main_layout_ = nullptr;

    // Initial placement group
    QGroupBox* initial_group_ = nullptr;
    QSpinBox* max_iter_spin_ = nullptr;
    QDoubleSpinBox* error_spin_ = nullptr;
    QPushButton* initial_place_btn_ = nullptr;

    // Global placement group
    QGroupBox* global_group_ = nullptr;
    QDoubleSpinBox* density_spin_ = nullptr;
    QDoubleSpinBox* overflow_spin_ = nullptr;
    QSpinBox* global_iter_spin_ = nullptr;
    QPushButton* global_place_btn_ = nullptr;

    // Control group
    QGroupBox* control_group_ = nullptr;
    QPushButton* full_flow_btn_ = nullptr;
    QProgressBar* progress_bar_ = nullptr;
    QLabel* status_label_ = nullptr;
};

} // namespace eda

#endif // EDA_GUI_ALGORITHM_PANEL_H
