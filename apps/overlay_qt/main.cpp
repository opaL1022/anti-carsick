#include <QAction>
#include <QApplication>
#include <QByteArray>
#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QMenu>
#include <QMessageBox>
#include <QStyle>
#include <QSystemTrayIcon>
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
    QApplication::setApplicationName(QStringLiteral("Anti Carsick Overlay"));
    QApplication::setApplicationVersion(QStringLiteral("1.0.0"));
    QApplication::setQuitOnLastWindowClosed(false);

    QCommandLineParser parser;
    parser.setApplicationDescription(
        QStringLiteral("Receives phone motion over UDP and renders edge cues."));
    parser.addHelpOption();
    parser.addVersionOption();
    QCommandLineOption portOption(
        {QStringLiteral("p"), QStringLiteral("port")},
        QStringLiteral("UDP port to listen on (1-65535)."),
        QStringLiteral("port"),
        QStringLiteral("9999"));
    parser.addOption(portOption);
    parser.process(app);

    bool portOk = false;
    const int requestedPort = parser.value(portOption).toInt(&portOk);
    if (!portOk || requestedPort < 1 || requestedPort > 65535) {
        QMessageBox::critical(nullptr, QStringLiteral("Anti Carsick"),
                              QStringLiteral("Port must be between 1 and 65535."));
        return 2;
    }

    OverlayWidget overlay;
    UdpReceiver receiver;

    QObject::connect(&receiver, &UdpReceiver::motionUpdated,
                     &overlay, &OverlayWidget::setMotion);
    QObject::connect(&receiver, &UdpReceiver::listeningChanged,
                     &overlay, &OverlayWidget::setListeningState);

    QMenu trayMenu;
    QAction toggleAction(QStringLiteral("Hide overlay"), &trayMenu);
    QAction exitAction(QStringLiteral("Exit"), &trayMenu);
    trayMenu.addAction(&toggleAction);
    trayMenu.addSeparator();
    trayMenu.addAction(&exitAction);

    QSystemTrayIcon tray(app.style()->standardIcon(QStyle::SP_ComputerIcon));
    tray.setToolTip(QStringLiteral("Anti Carsick Overlay — UDP %1").arg(requestedPort));
    tray.setContextMenu(&trayMenu);

    const auto toggleOverlay = [&overlay, &toggleAction]() {
        if (overlay.isVisible()) {
            overlay.hide();
            toggleAction.setText(QStringLiteral("Show overlay"));
        } else {
            overlay.showFullScreen();
            toggleAction.setText(QStringLiteral("Hide overlay"));
        }
    };
    QObject::connect(&toggleAction, &QAction::triggered, &app, toggleOverlay);
    QObject::connect(&exitAction, &QAction::triggered, &app, &QApplication::quit);
    QObject::connect(&tray, &QSystemTrayIcon::activated, &app,
                     [&toggleOverlay](QSystemTrayIcon::ActivationReason reason) {
        if (reason == QSystemTrayIcon::DoubleClick) {
            toggleOverlay();
        }
    });

    const bool listening = receiver.start(static_cast<quint16>(requestedPort));
    if (!listening) {
        QMessageBox::critical(
            nullptr,
            QStringLiteral("Anti Carsick"),
            QStringLiteral("UDP port %1 could not be opened. Another app may be using it.")
                .arg(requestedPort));
        return 1;
    }

    overlay.showFullScreen();
    if (QSystemTrayIcon::isSystemTrayAvailable()) {
        tray.show();
        tray.showMessage(
            QStringLiteral("Anti Carsick is running"),
            QStringLiteral("Listening on UDP %1. Double-click the tray icon to hide or show the overlay.")
                .arg(requestedPort),
            QSystemTrayIcon::Information,
            4000);
    }

    return app.exec();
}
