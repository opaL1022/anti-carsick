#pragma once

#include <QWidget>

class OverlayWidget : public QWidget {
    Q_OBJECT

public:
    explicit OverlayWidget(QWidget *parent = nullptr);

public slots:
    void setMotion(double ax, double ay, double az);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    double ax_ = 0.0;
    double ay_ = 0.0;
    double az_ = 9.8;

    double filteredAx_ = 0.0;
};
