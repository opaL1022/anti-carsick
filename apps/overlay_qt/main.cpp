#include <QApplication>
#include "overlaywidget.h"
#include "udpreceiver.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    OverlayWidget overlay;
    UdpReceiver receiver;

    QObject::connect(&receiver, &UdpReceiver::motionUpdated,
                     &overlay, &OverlayWidget::setMotion);

    receiver.start(9999);
    overlay.showFullScreen();

    return app.exec();
}
