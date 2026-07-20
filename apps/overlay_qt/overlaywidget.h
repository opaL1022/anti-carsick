#pragma once

#include <array>
#include <QWidget>
#include <QElapsedTimer>
#include <QPointF>
#include <QString>

class OverlayWidget : public QWidget {
    Q_OBJECT

public:
    explicit OverlayWidget(QWidget *parent = nullptr);

public slots:
    void setMotion(double ax, double ay, double az);
    void setListeningState(bool listening, const QString &message);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    static constexpr int kDotsPerSide = 9;
    static constexpr int kDotCount = kDotsPerSide * 4;

    struct DotState {
        QPointF position;
        QPointF velocity;
        QPointF noise;
        bool initialized = false;
    };

    void updateDotPhysics(const std::array<QPointF, kDotCount> &targets,
                          const std::array<QPointF, kDotCount> &restTargets);

    double ax_ = 0.0;
    double ay_ = 0.0;
    double az_ = 9.8;

    double filteredAx_ = 0.0;
    double filteredAy_ = 0.0;
    bool listening_ = false;
    QString statusMessage_ = QStringLiteral("Starting…");
    QElapsedTimer lastPacket_;
    QElapsedTimer animationClock_;
    std::array<DotState, kDotCount> dots_;
    int physicsWidth_ = 0;
    int physicsHeight_ = 0;
};
