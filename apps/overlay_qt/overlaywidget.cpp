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
                   Qt::Tool |
                   Qt::WindowTransparentForInput |
                   Qt::WindowDoesNotAcceptFocus |
                   Qt::NoDropShadowWindowHint |
                   Qt::X11BypassWindowManagerHint);

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

    const int w = width();
    const int h = height();

    const double gain = 36.0;
    const double maxShift = 48.0;
    const double shiftX = qBound(-maxShift, filteredAx_ * gain, maxShift);
    const double shiftY = qBound(-maxShift, -filteredAy_ * gain, maxShift);

    const double margin = 28.0;
    const double anchors[] = { 0.18, 0.39, 0.61, 0.82 };
    const double offsets[] = { 0.0, 30.0, 54.0 };
    const double radii[] = { 16.0, 10.0, 6.0 };

    const auto drawContrastDot = [&p](const QPointF &center, double radius) {
        // The white ring remains visible on dark content while the black middle
        // remains visible on light content, without needing screen-capture access.
        p.setBrush(QColor(255, 255, 255, 235));
        p.drawEllipse(center, radius, radius);
        p.setBrush(QColor(0, 0, 0, 225));
        p.drawEllipse(center, radius * 0.62, radius * 0.62);
        p.setBrush(QColor(255, 255, 255, 245));
        p.drawEllipse(center, radius * 0.27, radius * 0.27);
    };

    for (double anchor : anchors) {
        for (int i = 0; i < 3; ++i) {
            const double offset = offsets[i];
            const double radius = radii[i];
            drawContrastDot(QPointF(margin + offset + shiftX, h * anchor + shiftY), radius);
            drawContrastDot(QPointF(w - margin - offset + shiftX, h * anchor + shiftY), radius);
            drawContrastDot(QPointF(w * anchor + shiftX, margin + offset + shiftY), radius);
            drawContrastDot(QPointF(w * anchor + shiftX, h - margin - offset + shiftY), radius);
        }
    }

    p.setPen(receiving ? QColor(250, 178, 142, 220) : QColor(255, 255, 255, 155));
    const QString status = receiving
        ? QStringLiteral("LIVE  ax %1  ay %2").arg(ax_, 0, 'f', 2).arg(ay_, 0, 'f', 2)
        : listening_
            ? QStringLiteral("WAITING FOR PHONE")
            : statusMessage_.toUpper();
    p.drawText(30, 40, status);
}
