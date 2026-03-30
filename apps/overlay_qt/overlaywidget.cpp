#include "overlaywidget.h"

#include <QPainter>
#include <QGuiApplication>
#include <QScreen>
#include <QtMath>

OverlayWidget::OverlayWidget(QWidget *parent)
    : QWidget(parent) {
    setWindowFlags(Qt::FramelessWindowHint |
                   Qt::WindowStaysOnTopHint |
                   Qt::Tool |
                   Qt::WindowTransparentForInput |
                   Qt::WindowDoesNotAcceptFocus |
                   Qt::NoDropShadowWindowHint |
                   Qt::X11BypassWindowManagerHint);

    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_TransparentForMouseEvents);

    if (auto *screen = QGuiApplication::primaryScreen()) {
        setGeometry(screen->geometry());
    }
}

void OverlayWidget::setMotion(double ax, double ay, double az) {
    ax_ = ax;
    ay_ = ay;
    az_ = az;

    constexpr double alpha = 0.12;
    filteredAx_ = alpha * ax_ + (1.0 - alpha) * filteredAx_;

    update();
}

void OverlayWidget::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event);

    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);
    p.setPen(Qt::NoPen);
    p.setBrush(QColor(255, 255, 255, 180));

    const int w = width();
    const int h = height();

    const double gain = 30.0;
    const double maxShift = 40.0;
    const double shift = qBound(-maxShift, filteredAx_ * gain, maxShift);

    const double r = 7.0;
    const double d = r * 2.0;
    const double margin = 24.0;
    const double ys[] = { h * 0.2, h * 0.4, h * 0.6, h * 0.8 };

    for (double y : ys) {
        p.drawEllipse(QRectF(margin + shift, y, d, d));
        p.drawEllipse(QRectF(w - margin - d + shift, y, d, d));
    }

    p.setPen(QColor(255, 255, 255, 200));
    p.drawText(30, 40, QString("ax=%1 ay=%2").arg(ax_, 0, 'f', 3).arg(ay_, 0, 'f', 3));
}
