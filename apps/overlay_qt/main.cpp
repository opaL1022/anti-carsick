#include <QGuiApplication>
#include <QApplication>
#include <QByteArray>
#include <QDebug>
#include "overlaywidget.h"
#include "udpreceiver.h"

int main(int argc, char *argv[]) {
#if defined(Q_OS_LINUX)
    // Regular Wayland windows cannot request a compositor overlay layer, so a
    // native Wayland client may be placed below the app it should cover. When
    // XWayland is available, use Qt's XCB backend to retain the always-on-top
    // and input-transparent window behavior.
    const QByteArray platform = qgetenv("QT_QPA_PLATFORM");
    if ((platform.isEmpty() || platform.startsWith("wayland")) &&
        !qEnvironmentVariableIsEmpty("WAYLAND_DISPLAY") &&
        !qEnvironmentVariableIsEmpty("DISPLAY")) {
        qputenv("QT_QPA_PLATFORM", "xcb");
    }
#endif

    QApplication app(argc, argv);
    qDebug() << QGuiApplication::platformName();

    OverlayWidget overlay;
    UdpReceiver receiver;

    QObject::connect(&receiver, &UdpReceiver::motionUpdated,
                     &overlay, &OverlayWidget::setMotion);

    receiver.start(9999);
    overlay.showFullScreen();
    return app.exec();
}
