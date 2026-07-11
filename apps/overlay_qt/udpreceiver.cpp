#include "udpreceiver.h"

#include <QUdpSocket>
#include <QNetworkDatagram>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDebug>
#include <cmath>

UdpReceiver::UdpReceiver(QObject *parent)
    : QObject(parent), socket_(new QUdpSocket(this)) {
    connect(socket_, &QUdpSocket::readyRead,
            this, &UdpReceiver::onReadyRead);
}

bool UdpReceiver::start(quint16 port) {
    if (socket_->state() == QAbstractSocket::BoundState) {
        socket_->close();
    }

    const bool ok = socket_->bind(QHostAddress::AnyIPv4, port);
    const QString message = ok
        ? QStringLiteral("Listening on UDP %1").arg(port)
        : QStringLiteral("Could not listen on UDP %1: %2")
              .arg(port)
              .arg(socket_->errorString());
    qInfo().noquote() << message;
    emit listeningChanged(ok, message);
    return ok;
}

quint16 UdpReceiver::port() const {
    return socket_->localPort();
}

void UdpReceiver::onReadyRead() {
    while (socket_->hasPendingDatagrams()) {
        QNetworkDatagram datagram = socket_->receiveDatagram();
        const QByteArray data = datagram.data();

        QJsonParseError err;
        QJsonDocument doc = QJsonDocument::fromJson(data, &err);
        if (err.error != QJsonParseError::NoError || !doc.isObject()) {
            if (!invalidPacketLogTimer_.isValid() || invalidPacketLogTimer_.elapsed() > 2000) {
                qWarning() << "Ignored invalid UDP packet from" << datagram.senderAddress();
                invalidPacketLogTimer_.restart();
            }
            continue;
        }

        QJsonObject obj = doc.object();
        if (!obj.value("ax").isDouble() || !obj.value("ay").isDouble() ||
            !obj.value("az").isDouble()) {
            continue;
        }
        double ax = obj.value("ax").toDouble(0.0);
        double ay = obj.value("ay").toDouble(0.0);
        double az = obj.value("az").toDouble(0.0);

        if (!std::isfinite(ax) || !std::isfinite(ay) || !std::isfinite(az)) {
            continue;
        }

        emit motionUpdated(ax, ay, az);
    }
}
