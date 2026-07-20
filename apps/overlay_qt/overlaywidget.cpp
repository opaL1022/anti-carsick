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

void OverlayWidget::updateDotPhysics(const std::array<QPointF, 48> &targets) {
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

    // Each consecutive trio has two springs.  Their natural lengths are the
    // visual gaps between the three resting dots. The cubic term means that
    // stretching or compression becomes progressively harder, keeping dots
    // from bunching together or spreading into separate clusters.
    constexpr double naturalGaps[] = { 44.0, 42.0 };
    for (int group = 0; group < 16; ++group) {
        const int first = group * 3;
        for (int pair = 0; pair < 2; ++pair) {
            const int left = first + pair;
            const int right = left + 1;
            const QPointF delta = dots_[right].position - dots_[left].position;
            const double distance = qSqrt(delta.x() * delta.x() + delta.y() * delta.y());
            if (distance < 0.001) {
                continue;
            }

            const double extension = distance - naturalGaps[pair];
            const double normalizedExtension = extension / naturalGaps[pair];
            const double springForce = extension * springStiffness *
                (1.0 + 4.0 * normalizedExtension * normalizedExtension);
            const QPointF restoringForce = delta / distance * springForce;
            forces[left] += restoringForce;
            forces[right] -= restoringForce;
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

    std::array<QPointF, 48> targets;
    for (int anchorIndex = 0; anchorIndex < 4; ++anchorIndex) {
        const double anchor = anchors[anchorIndex];
        for (int side = 0; side < 4; ++side) {
            const int group = anchorIndex * 4 + side;
            for (int i = 0; i < 3; ++i) {
                const double offset = offsets[i];
                const double influence = motionInfluence[i];
                const double xShift = shiftX * influence;
                const double yShift = shiftY * influence;
                switch (side) {
                case 0: // Left edge, outer to inner.
                    targets[group * 3 + i] = QPointF(margin + offset + xShift, h * anchor + yShift);
                    break;
                case 1: // Right edge, outer to inner.
                    targets[group * 3 + i] = QPointF(w - margin - offset + xShift, h * anchor + yShift);
                    break;
                case 2: // Top edge, outer to inner.
                    targets[group * 3 + i] = QPointF(w * anchor + xShift, margin + offset + yShift);
                    break;
                default: // Bottom edge, outer to inner.
                    targets[group * 3 + i] = QPointF(w * anchor + xShift, h - margin - offset + yShift);
                    break;
                }
            }
        }
    }
    updateDotPhysics(targets);

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

    for (int group = 0; group < 16; ++group) {
        for (int i = 0; i < 3; ++i) {
            drawContrastDot(dots_[group * 3 + i].position, radii[i]);
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
