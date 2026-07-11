# Anti Carsick

Anti Carsick uses motion cues to reduce the visual/vestibular mismatch that can
contribute to motion discomfort. A phone streams acceleration over the local
network and a transparent Windows overlay renders matching dots at both screen
edges.

> This is an experimental comfort aid, not a medical device or treatment. Stop
> using it if the visual cues make symptoms worse.

## How it works

```text
Android / iPhone                       Windows PC
┌──────────────────────┐   UDP JSON   ┌────────────────────────┐
│ Flutter sender       │ ───────────► │ Qt 6 overlay           │
│ user acceleration    │   port 9999  │ filtered edge cues     │
└──────────────────────┘              └────────────────────────┘
```

Packets are UTF-8 JSON objects containing `t`, `ax`, `ay`, and `az`. The sender
targets approximately 30 updates per second. The receiver validates every
packet and applies a low-pass filter before moving the visual cues.

## Features

- Real user-acceleration data on Android and iOS (gravity removed)
- Simulation mode for development and desktop sender testing
- Configurable destination address and UDP port
- Transparent, always-on-top, mouse-through Windows overlay
- Horizontal and vertical motion cues with smoothing
- Waiting/live state and stale-connection detection
- Windows system tray controls for showing, hiding, and exiting
- Input and UDP packet validation

## Run on Windows

### 1. Build the overlay

Install:

- Visual Studio 2022 with **Desktop development with C++**
- CMake
- Qt 6.5 or newer, using the **MSVC 2022 64-bit** component

From PowerShell in the repository root:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File .\scripts\build-windows.ps1 -QtRoot C:\Qt\6.8.3\msvc2022_64
```

Adjust the Qt path to the version installed on your machine. The portable output
is created at `dist\AntiCarsickOverlay`. Run:

```powershell
.\dist\AntiCarsickOverlay\MotionOverlay.exe
```

To use another UDP port:

```powershell
.\dist\AntiCarsickOverlay\MotionOverlay.exe --port 10000
```

The first time it runs, allow the app through Windows Firewall on **private
networks**. Right-click the system tray icon to hide the overlay or exit.

### 2. Run the sender

Install a Flutter SDK that includes Dart 3.11 or newer and an Android/iOS
toolchain, then:

```powershell
cd apps\sender_flutter
flutter pub get
flutter run
```

In the sender:

1. Enter the Windows PC's local IPv4 address. Find it with `ipconfig` under the
   active Wi-Fi adapter, for example `192.168.1.10`.
2. Keep port `9999`, unless the overlay was started with a different port.
3. Ensure phone and PC are on the same Wi-Fi network.
4. Select **手機感測器** and press **開始傳送**.

For a same-PC smoke test, run the Flutter Windows app in simulation mode with
the target `127.0.0.1:9999`.

## Development

Flutter checks:

```powershell
cd apps\sender_flutter
dart format lib test
flutter analyze
flutter test
```

Qt debug build:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File .\scripts\build-windows.ps1 -Configuration Debug -QtRoot C:\Qt\6.8.3\msvc2022_64
```

## Project layout

```text
apps/
  sender_flutter/   Flutter phone sender and simulation client
  overlay_qt/       Qt/C++ Windows overlay and UDP receiver
scripts/
  build-windows.ps1 Windows build and portable deployment
```

## Troubleshooting

- **Overlay says WAITING FOR PHONE:** verify the PC IP and matching port; avoid
  guest Wi-Fi networks that isolate devices.
- **Port could not be opened:** close another overlay instance or choose another
  port on both apps.
- **No packets across Wi-Fi:** allow `MotionOverlay.exe` through Windows Firewall
  on private networks.
- **Sensor unavailable:** use a physical Android/iPhone; desktop platforms use
  simulation mode.
