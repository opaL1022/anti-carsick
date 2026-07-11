#pragma once

#include <QObject>
#include <QElapsedTimer>
#include <QString>

class QUdpSocket;

class UdpReceiver : public QObject {
    Q_OBJECT

public:
    explicit UdpReceiver(QObject *parent = nullptr);
    bool start(quint16 port);
    quint16 port() const;

signals:
    void motionUpdated(double ax, double ay, double az);
    void listeningChanged(bool listening, const QString &message);

private slots:
    void onReadyRead();

private:
    QUdpSocket *socket_;
    QElapsedTimer invalidPacketLogTimer_;
};
