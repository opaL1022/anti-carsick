#include "overlaywidget.h"

#include <QPainter>
#include <QGuiApplication>
#include <QScreen>
#include <QTimer>
#include <QtMath>

OverlayWidget::OverlayWidget(QWidget *parent)
    : QWidget(parent) {
    setWindowFlags(Qt::FramelessWindowHint |
                   Qt::WindowStaysOnTopHint |
                   Qt::Tool);

    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_TransparentForMouseEvents);
    setWindowTitle(QStringLiteral("Anti Carsick Overlay"));

    if (auto *screen = QGuiApplication::primaryScreen()) {
        setGeometry(screen->geometry());
    }

    auto *refreshTimer = new QTimer(this);
    refreshTimer->setInterval(250);
    connect(refreshTimer, &QTimer::timeout, this, qOverload<>(&OverlayWidget::update));
    refreshTimer->start();
}

void OverlayWidget::setMotion(double ax, double ay, double az) {
    ax_ = ax;
    ay_ = ay;
    az_ = az;

    constexpr double alpha = 0.12;
    filteredAx_ = alpha * ax_ + (1.0 - alpha) * filteredAx_;
    filteredAy_ = alpha * ay_ + (1.0 - alpha) * filteredAy_;
    lastPacket_.restart();

    update();
}

void OverlayWidget::setListeningState(bool listening, const QString &message) {
    listening_ = listening;
    statusMessage_ = message;
    update();
}

void OverlayWidget::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event);

    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);
    p.setPen(Qt::NoPen);
    const bool receiving = listening_ && lastPacket_.isValid() && lastPacket_.elapsed() < 1500;
    p.setBrush(receiving ? QColor(250, 178, 142, 205) : QColor(255, 255, 255, 85));

    const int w = width();
    const int h = height();

    const double gain = 30.0;
    const double maxShift = 40.0;
    const double shiftX = qBound(-maxShift, filteredAx_ * gain, maxShift);
    const double shiftY = qBound(-maxShift, -filteredAy_ * gain, maxShift);

    const double r = 7.0;
    const double d = r * 2.0;
    const double margin = 24.0;
    const double ys[] = { h * 0.2, h * 0.4, h * 0.6, h * 0.8 };

    for (double y : ys) {
        p.drawEllipse(QRectF(margin + shiftX, y + shiftY, d, d));
        p.drawEllipse(QRectF(w - margin - d + shiftX, y + shiftY, d, d));
    }

    p.setPen(receiving ? QColor(250, 178, 142, 220) : QColor(255, 255, 255, 155));
    const QString status = receiving
        ? QStringLiteral("LIVE  ax %1  ay %2").arg(ax_, 0, 'f', 2).arg(ay_, 0, 'f', 2)
        : listening_
            ? QStringLiteral("WAITING FOR PHONE")
            : statusMessage_.toUpper();
    p.drawText(30, 40, status);
}
