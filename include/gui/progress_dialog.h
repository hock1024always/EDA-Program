#ifndef EDA_GUI_PROGRESS_DIALOG_H
#define EDA_GUI_PROGRESS_DIALOG_H

#include <QDialog>
#include <memory>

QT_BEGIN_NAMESPACE
class QProgressBar;
class QLabel;
class QPushButton;
class QTimer;
QT_END_NAMESPACE

namespace eda {

/**
 * @brief 进度对话框
 *
 * 显示长时间运行操作的进度
 */
class ProgressDialog : public QDialog {
    Q_OBJECT

public:
    explicit ProgressDialog(QWidget* parent = nullptr);
    ~ProgressDialog() override = default;

    void setOperation(const QString& operation);
    void setProgress(int value, int maximum = 100);
    void setMessage(const QString& message);
    void showTimeRemaining(bool show = true);

public slots:
    void cancel();

signals:
    void canceled();

private:
    void updateTime();

    QLabel* operation_label_ = nullptr;
    QLabel* message_label_ = nullptr;
    QProgressBar* progress_bar_ = nullptr;
    QLabel* time_label_ = nullptr;
    QPushButton* cancel_button_ = nullptr;
    QTimer* timer_ = nullptr;

    qint64 start_time_ = 0;
    bool show_time_ = false;
};

} // namespace eda

#endif // EDA_GUI_PROGRESS_DIALOG_H
