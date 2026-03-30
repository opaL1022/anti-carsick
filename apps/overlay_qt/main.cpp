#include <QGuiApplication>
#include <QApplication>
#include <QTimer>
#include <QWindow>
#include <QDebug>
#include "overlaywidget.h"
#include "udpreceiver.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    qDebug() << QGuiApplication::platformName();

    OverlayWidget overlay;
    UdpReceiver receiver;

    QObject::connect(&receiver, &UdpReceiver::motionUpdated,
                     &overlay, &OverlayWidget::setMotion);

    receiver.start(9999);
    overlay.show();
    return app.exec();
}
