#include "udpreceiver.h"

#include <QUdpSocket>
#include <QNetworkDatagram>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDebug>

UdpReceiver::UdpReceiver(QObject *parent)
    : QObject(parent), socket_(new QUdpSocket(this)) {
    connect(socket_, &QUdpSocket::readyRead,
            this, &UdpReceiver::onReadyRead);
}

void UdpReceiver::start(quint16 port) {
    const bool ok = socket_->bind(QHostAddress::AnyIPv4, port);
    qDebug() << "UDP bind" << port << (ok ? "ok" : "failed");
}

void UdpReceiver::onReadyRead() {
    while (socket_->hasPendingDatagrams()) {
        QNetworkDatagram datagram = socket_->receiveDatagram();
        const QByteArray data = datagram.data();

        QJsonParseError err;
        QJsonDocument doc = QJsonDocument::fromJson(data, &err);
        if (err.error != QJsonParseError::NoError || !doc.isObject()) {
            continue;
        }

        QJsonObject obj = doc.object();
        double ax = obj.value("ax").toDouble(0.0);
        double ay = obj.value("ay").toDouble(0.0);
        double az = obj.value("az").toDouble(9.8);

        emit motionUpdated(ax, ay, az);
    }
}
