#include "progress_dialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QProgressBar>
#include <QPushButton>
#include <QTimer>
#include <QDateTime>
#include <QElapsedTimer>

namespace eda {

ProgressDialog::ProgressDialog(QWidget* parent)
    : QDialog(parent) {
    setWindowTitle("Progress");
    setModal(true);
    resize(400, 150);

    auto* layout = new QVBoxLayout(this);

    operation_label_ = new QLabel("Operation in progress...", this);
    operation_label_->setStyleSheet("font-weight: bold;");
    layout->addWidget(operation_label_);

    message_label_ = new QLabel("Please wait...", this);
    layout->addWidget(message_label_);

    progress_bar_ = new QProgressBar(this);
    layout->addWidget(progress_bar_);

    time_label_ = new QLabel("", this);
    layout->addWidget(time_label_);

    auto* button_layout = new QHBoxLayout;
    button_layout->addStretch();
    cancel_button_ = new QPushButton("Cancel", this);
    connect(cancel_button_, &QPushButton::clicked, this, &ProgressDialog::cancel);
    button_layout->addWidget(cancel_button_);
    layout->addLayout(button_layout);

    timer_ = new QTimer(this);
    connect(timer_, &QTimer::timeout, this, &ProgressDialog::updateTime);
    timer_->start(1000); // Update every second

    start_time_ = QDateTime::currentMSecsSinceEpoch();
}

void ProgressDialog::setOperation(const QString& operation) {
    operation_label_->setText(operation);
    setWindowTitle(operation);
}

void ProgressDialog::setProgress(int value, int maximum) {
    progress_bar_->setMaximum(maximum);
    progress_bar_->setValue(value);

    if (value >= maximum) {
        accept(); // Close dialog when complete
    }
}

void ProgressDialog::setMessage(const QString& message) {
    message_label_->setText(message);
}

void ProgressDialog::showTimeRemaining(bool show) {
    show_time_ = show;
    time_label_->setVisible(show);
}

void ProgressDialog::cancel() {
    emit canceled();
    reject();
}

void ProgressDialog::updateTime() {
    if (!show_time_) return;

    qint64 elapsed = QDateTime::currentMSecsSinceEpoch() - start_time_;
    qint64 seconds = elapsed / 1000;
    qint64 minutes = seconds / 60;
    seconds %= 60;

    QString time_str = QString("Elapsed: %1:%2")
                          .arg(minutes, 2, 10, QLatin1Char('0'))
                          .arg(seconds, 2, 10, QLatin1Char('0'));

    // Estimate remaining time if progress is known
    int value = progress_bar_->value();
    int maximum = progress_bar_->maximum();
    if (maximum > 0 && value > 0 && value < maximum) {
        qint64 total_est = elapsed * maximum / value;
        qint64 remaining = total_est - elapsed;
        qint64 rem_seconds = remaining / 1000;
        qint64 rem_minutes = rem_seconds / 60;
        rem_seconds %= 60;

        time_str += QString("  Remaining: %1:%2")
                       .arg(rem_minutes, 2, 10, QLatin1Char('0'))
                       .arg(rem_seconds, 2, 10, QLatin1Char('0'));
    }

    time_label_->setText(time_str);
}

} // namespace eda
