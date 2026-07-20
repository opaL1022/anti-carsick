#include "overlaywidget.h"

#include <QPainter>
#include <QGuiApplication>
#include <QRandomGenerator>
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
    // Drive the spring animation independently from arriving network packets.
    refreshTimer->setInterval(16);
    connect(refreshTimer, &QTimer::timeout, this, qOverload<>(&OverlayWidget::update));
    refreshTimer->start();
    animationClock_.start();
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

void OverlayWidget::updateDotPhysics(
    const std::array<QPointF, kDotCount> &targets,
    const std::array<QPointF, kDotCount> &restTargets) {
    constexpr double anchorStiffness = 42.0;
    constexpr double springStiffness = 30.0;
    constexpr double damping = 10.5;
    constexpr double noiseStrength = 18.0;

    const double elapsedSeconds = animationClock_.restart() / 1000.0;
    // A debugger stop or a temporarily stalled compositor must not cause a jump.
    const double dt = qBound(0.001, elapsedSeconds, 0.04);

    if (physicsWidth_ != width() || physicsHeight_ != height()) {
        physicsWidth_ = width();
        physicsHeight_ = height();
        for (auto &dot : dots_) {
            dot.initialized = false;
        }
    }

    std::array<QPointF, 48> forces {};
    for (int index = 0; index < static_cast<int>(dots_.size()); ++index) {
        DotState &dot = dots_[index];
        if (!dot.initialized) {
            dot.position = targets[index];
            dot.velocity = QPointF();
            dot.noise = QPointF();
            dot.initialized = true;
        }

        const auto randomUnit = []() {
            return QRandomGenerator::global()->generateDouble() * 2.0 - 1.0;
        };
        // Correlated noise gives every dot a subtle independent drift, rather
        // than frame-to-frame flicker.
        dot.noise = dot.noise * 0.94 + QPointF(randomUnit(), randomUnit()) * 0.06;
        forces[index] += (targets[index] - dot.position) * anchorStiffness;
        forces[index] += dot.noise * noiseStrength;
        forces[index] -= dot.velocity * damping;
    }

    // Every edge is a 4 -> 3 -> 2 triangular mesh: a smaller dot belongs in
    // the gap between the two larger dots before it. Its springs use the rest
    // layout as their natural length. The cubic term makes excessive stretching
    // and compression progressively harder.
    const auto applySpring = [&](int first, int second) {
        const QPointF delta = dots_[second].position - dots_[first].position;
        const double distance = qSqrt(delta.x() * delta.x() + delta.y() * delta.y());
        const QPointF restDelta = restTargets[second] - restTargets[first];
        const double naturalLength = qSqrt(restDelta.x() * restDelta.x() +
                                            restDelta.y() * restDelta.y());
        if (distance < 0.001 || naturalLength < 0.001) {
            return;
        }

        const double extension = distance - naturalLength;
        const double normalizedExtension = extension / naturalLength;
        const double springForce = extension * springStiffness *
            (1.0 + 4.0 * normalizedExtension * normalizedExtension);
        const QPointF restoringForce = delta / distance * springForce;
        forces[first] += restoringForce;
        forces[second] -= restoringForce;
    };
    for (int side = 0; side < 4; ++side) {
        const int base = side * kDotsPerSide;
        // Middle dots: 4, 5, 6; inner dots: 7, 8.
        for (int i = 0; i < 3; ++i) {
            applySpring(base + i, base + 4 + i);
            applySpring(base + i + 1, base + 4 + i);
        }
        for (int i = 0; i < 2; ++i) {
            applySpring(base + 4 + i, base + 7 + i);
            applySpring(base + 4 + i + 1, base + 7 + i);
        }
    }

    for (int index = 0; index < static_cast<int>(dots_.size()); ++index) {
        DotState &dot = dots_[index];
        dot.velocity += forces[index] * dt;
        dot.position += dot.velocity * dt;
    }
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
    // More room makes the three-dot cue readable even in peripheral vision.
    const double offsets[] = { 0.0, 44.0, 86.0 };
    const double radii[] = { 16.0, 10.0, 6.0 };
    const double motionInfluence[] = { 0.68, 1.0, 1.38 };

    std::array<QPointF, kDotCount> targets;
    std::array<QPointF, kDotCount> restTargets;
    const auto edgePoint = [&](int side, double anchor, double offset,
                               double xShift, double yShift) {
        switch (side) {
        case 0: return QPointF(margin + offset + xShift, h * anchor + yShift);
        case 1: return QPointF(w - margin - offset + xShift, h * anchor + yShift);
        case 2: return QPointF(w * anchor + xShift, margin + offset + yShift);
        default: return QPointF(w * anchor + xShift, h - margin - offset + yShift);
        }
    };
    for (int side = 0; side < 4; ++side) {
        const int base = side * kDotsPerSide;
        for (int layer = 0; layer < 3; ++layer) {
            const int count = 4 - layer;
            std::array<double, 4> layerAnchors = { anchors[0], anchors[1], anchors[2], anchors[3] };
            int layerAnchorCount = 4;
            for (int midpoint = 0; midpoint < layer; ++midpoint) {
                for (int i = 0; i < layerAnchorCount - 1; ++i) {
                    layerAnchors[i] = (layerAnchors[i] + layerAnchors[i + 1]) / 2.0;
                }
                --layerAnchorCount;
            }
            for (int i = 0; i < count; ++i) {
                // Each layer's anchors are the midpoints of the layer before it.
                const double anchor = layerAnchors[i];
                const int index = base + (layer == 0 ? i : layer == 1 ? 4 + i : 7 + i);
                restTargets[index] = edgePoint(side, anchor, offsets[layer], 0.0, 0.0);
                targets[index] = edgePoint(side, anchor, offsets[layer],
                                           shiftX * motionInfluence[layer],
                                           shiftY * motionInfluence[layer]);
            }
        }
    }
    updateDotPhysics(targets, restTargets);

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

    for (int side = 0; side < 4; ++side) {
        const int base = side * kDotsPerSide;
        for (int i = 0; i < kDotsPerSide; ++i) {
            const int layer = i < 4 ? 0 : i < 7 ? 1 : 2;
            drawContrastDot(dots_[base + i].position, radii[layer]);
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
