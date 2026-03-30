#pragma once

#include <QObject>

class QUdpSocket;

class UdpReceiver : public QObject {
    Q_OBJECT

public:
    explicit UdpReceiver(QObject *parent = nullptr);
    void start(quint16 port);

signals:
    void motionUpdated(double ax, double ay, double az);

private slots:
    void onReadyRead();

private:
    QUdpSocket *socket_;
};
