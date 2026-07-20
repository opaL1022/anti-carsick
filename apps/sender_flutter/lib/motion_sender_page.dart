import 'dart:async';
import 'dart:io';
import 'dart:math';

import 'package:flutter/foundation.dart';
import 'package:flutter/material.dart';
import 'package:sensors_plus/sensors_plus.dart';

import 'motion_packet.dart';
import 'theme.dart';

enum MotionSource { device, simulation }

class MotionSenderPage extends StatefulWidget {
  const MotionSenderPage({super.key});

  @override
  State<MotionSenderPage> createState() => _MotionSenderPageState();
}

class _MotionSenderPageState extends State<MotionSenderPage>
    with WidgetsBindingObserver {
  final _ipController = TextEditingController(text: '127.0.0.1');
  final _portController = TextEditingController(text: '9999');

  RawDatagramSocket? _socket;
  StreamSubscription<UserAccelerometerEvent>? _sensorSubscription;
  Timer? _simulationTimer;

  bool _running = false;
  bool _starting = false;
  double _ax = 0;
  double _ay = 0;
  double _az = 0;
  double _simulationTime = 0;
  int _packetsSent = 0;
  String? _error;
  bool _pausedByBackground = false;
  late MotionSource _source;

  bool get _deviceSensorsSupported {
    if (kIsWeb) return true;
    return Platform.isAndroid || Platform.isIOS;
  }

  @override
  void initState() {
    super.initState();
    WidgetsBinding.instance.addObserver(this);
    _source = _deviceSensorsSupported
        ? MotionSource.device
        : MotionSource.simulation;
  }

  @override
  void didChangeAppLifecycleState(AppLifecycleState state) {
    if ((state == AppLifecycleState.paused ||
            state == AppLifecycleState.detached) &&
        (_running || _starting)) {
      _pausedByBackground = true;
      unawaited(_stopSending(message: 'iPhone 進入背景後會暫停動作傳送；回到 app 後請重新開始。'));
    }
  }

  Future<void> _startSending() async {
    final host = _ipController.text.trim();
    final port = int.tryParse(_portController.text.trim());

    if (host.isEmpty) {
      setState(() => _error = '請輸入接收端 IP 或主機名稱。');
      return;
    }
    if (port == null || port < 1 || port > 65535) {
      setState(() => _error = 'Port 必須介於 1 到 65535。');
      return;
    }

    setState(() {
      _starting = true;
      _error = null;
      _packetsSent = 0;
    });

    try {
      await _stopSending(updateState: false);
      final addresses = await InternetAddress.lookup(host);
      final target = addresses.firstWhere(
        (address) => address.type == InternetAddressType.IPv4,
        orElse: () => throw const SocketException('找不到 IPv4 位址'),
      );
      _socket = await RawDatagramSocket.bind(InternetAddress.anyIPv4, 0);

      if (_source == MotionSource.device) {
        _sensorSubscription =
            userAccelerometerEventStream(
              samplingPeriod: const Duration(milliseconds: 33),
            ).listen(
              (event) => _send(target, port, event.x, event.y, event.z),
              onError: (Object error) {
                if (!mounted) return;
                unawaited(_stopSending(message: '無法讀取動作感測器：$error'));
              },
              cancelOnError: true,
            );
      } else {
        _simulationTimer = Timer.periodic(const Duration(milliseconds: 33), (
          _,
        ) {
          _simulationTime += 0.1;
          _send(
            target,
            port,
            sin(_simulationTime) * 1.5,
            cos(_simulationTime * 0.7) * 0.8,
            0,
          );
        });
      }

      if (mounted) {
        setState(() {
          _running = true;
          _starting = false;
          _pausedByBackground = false;
        });
      }
    } catch (error) {
      if (!mounted) return;
      setState(() {
        _starting = false;
        _error = '無法啟動傳送：$error';
      });
    }
  }

  void _send(InternetAddress target, int port, double x, double y, double z) {
    final packet = MotionPacket(
      timestamp: DateTime.now().millisecondsSinceEpoch / 1000,
      ax: x,
      ay: y,
      az: z,
    );

    try {
      final sent = _socket?.send(packet.encode(), target, port) ?? 0;
      if (sent <= 0) {
        throw const SocketException('UDP socket 沒有送出資料');
      }
      if (!mounted) return;
      setState(() {
        _ax = x;
        _ay = y;
        _az = z;
        _packetsSent++;
      });
    } catch (error) {
      if (!mounted) return;
      unawaited(_stopSending(message: '傳送失敗：$error'));
    }
  }

  Future<void> _stopSending({bool updateState = true, String? message}) async {
    final sensorSubscription = _sensorSubscription;
    _sensorSubscription = null;
    await sensorSubscription?.cancel();
    _simulationTimer?.cancel();
    _simulationTimer = null;
    _socket?.close();
    _socket = null;
    if (mounted && updateState) {
      setState(() {
        _running = false;
        _starting = false;
        if (message != null) {
          _error = message;
        }
      });
    }
  }

  @override
  void dispose() {
    WidgetsBinding.instance.removeObserver(this);
    unawaited(_sensorSubscription?.cancel() ?? Future<void>.value());
    _simulationTimer?.cancel();
    _socket?.close();
    _ipController.dispose();
    _portController.dispose();
    super.dispose();
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        title: const Text('ANTI CARSICK'),
        actions: [
          Padding(
            padding: const EdgeInsets.only(right: 16),
            child: _StatusPill(running: _running),
          ),
        ],
      ),
      body: SafeArea(
        child: ListView(
          padding: const EdgeInsets.all(20),
          children: [
            Text('讓視覺跟上移動', style: Theme.of(context).textTheme.headlineMedium),
            const SizedBox(height: 8),
            Text(
              '將手機動作資料送到 Windows overlay，在螢幕邊緣顯示移動提示。手機與電腦需位於同一網路。',
              style: Theme.of(context).textTheme.bodyMedium?.copyWith(
                color: DaybreakTheme.muted,
                height: 1.5,
              ),
            ),
            const SizedBox(height: 24),
            Card(
              margin: EdgeInsets.zero,
              child: Padding(
                padding: const EdgeInsets.all(18),
                child: Column(
                  crossAxisAlignment: CrossAxisAlignment.start,
                  children: [
                    Text(
                      '連線設定',
                      style: Theme.of(context).textTheme.titleMedium,
                    ),
                    const SizedBox(height: 16),
                    Row(
                      children: [
                        Expanded(
                          flex: 2,
                          child: TextField(
                            controller: _ipController,
                            enabled: !_running && !_starting,
                            decoration: const InputDecoration(
                              labelText: 'Windows IP',
                              hintText: '192.168.1.10',
                              prefixIcon: Icon(Icons.computer_rounded),
                            ),
                          ),
                        ),
                        const SizedBox(width: 12),
                        Expanded(
                          child: TextField(
                            controller: _portController,
                            enabled: !_running && !_starting,
                            keyboardType: TextInputType.number,
                            decoration: const InputDecoration(
                              labelText: 'Port',
                            ),
                          ),
                        ),
                      ],
                    ),
                    const SizedBox(height: 18),
                    SegmentedButton<MotionSource>(
                      segments: [
                        ButtonSegment(
                          value: MotionSource.device,
                          enabled: _deviceSensorsSupported,
                          icon: const Icon(Icons.sensors_rounded),
                          label: const Text('手機感測器'),
                        ),
                        const ButtonSegment(
                          value: MotionSource.simulation,
                          icon: Icon(Icons.science_outlined),
                          label: Text('模擬資料'),
                        ),
                      ],
                      selected: {_source},
                      onSelectionChanged: _running || _starting
                          ? null
                          : (selection) =>
                                setState(() => _source = selection.first),
                    ),
                    if (!_deviceSensorsSupported) ...[
                      const SizedBox(height: 10),
                      const Text(
                        '此平台沒有動作感測器支援，請使用模擬資料；實際使用請在 Android 或 iPhone 執行。',
                        style: TextStyle(
                          color: DaybreakTheme.muted,
                          fontSize: 12,
                        ),
                      ),
                    ],
                  ],
                ),
              ),
            ),
            const SizedBox(height: 16),
            _MotionReadout(ax: _ax, ay: _ay, az: _az, packets: _packetsSent),
            if (_pausedByBackground) ...[
              const SizedBox(height: 14),
              const Text(
                '為了節省電力，iPhone 會暫停背景 app 的動作串流。回到這個畫面後請重新開始傳送。',
                style: TextStyle(color: DaybreakTheme.muted, fontSize: 12),
              ),
            ],
            if (_error != null) ...[
              const SizedBox(height: 14),
              Container(
                padding: const EdgeInsets.all(14),
                decoration: BoxDecoration(
                  color: DaybreakTheme.red.withValues(alpha: 0.12),
                  border: Border.all(
                    color: DaybreakTheme.red.withValues(alpha: 0.5),
                  ),
                  borderRadius: BorderRadius.circular(12),
                ),
                child: Row(
                  children: [
                    const Icon(
                      Icons.error_outline_rounded,
                      color: DaybreakTheme.red,
                    ),
                    const SizedBox(width: 10),
                    Expanded(child: Text(_error!)),
                  ],
                ),
              ),
            ],
            const SizedBox(height: 20),
            SizedBox(
              height: 54,
              child: ElevatedButton.icon(
                onPressed: _starting
                    ? null
                    : _running
                    ? () => unawaited(_stopSending())
                    : _startSending,
                icon: _starting
                    ? const SizedBox.square(
                        dimension: 18,
                        child: CircularProgressIndicator(strokeWidth: 2),
                      )
                    : Icon(
                        _running
                            ? Icons.stop_rounded
                            : Icons.play_arrow_rounded,
                      ),
                label: Text(
                  _starting
                      ? '正在連線…'
                      : _running
                      ? '停止傳送'
                      : '開始傳送',
                ),
                style: _running
                    ? ElevatedButton.styleFrom(
                        backgroundColor: DaybreakTheme.red,
                        foregroundColor: DaybreakTheme.brightWhite,
                      )
                    : null,
              ),
            ),
          ],
        ),
      ),
    );
  }
}

class _StatusPill extends StatelessWidget {
  const _StatusPill({required this.running});

  final bool running;

  @override
  Widget build(BuildContext context) {
    final color = running ? DaybreakTheme.green : DaybreakTheme.muted;
    return Center(
      child: Container(
        padding: const EdgeInsets.symmetric(horizontal: 10, vertical: 6),
        decoration: BoxDecoration(
          border: Border.all(color: color.withValues(alpha: 0.5)),
          borderRadius: BorderRadius.circular(20),
        ),
        child: Row(
          mainAxisSize: MainAxisSize.min,
          children: [
            Container(
              width: 7,
              height: 7,
              decoration: BoxDecoration(color: color, shape: BoxShape.circle),
            ),
            const SizedBox(width: 7),
            Text(
              running ? '傳送中' : '待機',
              style: TextStyle(color: color, fontSize: 12),
            ),
          ],
        ),
      ),
    );
  }
}

class _MotionReadout extends StatelessWidget {
  const _MotionReadout({
    required this.ax,
    required this.ay,
    required this.az,
    required this.packets,
  });

  final double ax;
  final double ay;
  final double az;
  final int packets;

  @override
  Widget build(BuildContext context) {
    return Card(
      margin: EdgeInsets.zero,
      child: Padding(
        padding: const EdgeInsets.all(18),
        child: Column(
          crossAxisAlignment: CrossAxisAlignment.start,
          children: [
            Row(
              mainAxisAlignment: MainAxisAlignment.spaceBetween,
              children: [
                Text('即時動作', style: Theme.of(context).textTheme.titleMedium),
                Text(
                  '$packets packets',
                  style: const TextStyle(
                    color: DaybreakTheme.muted,
                    fontSize: 12,
                  ),
                ),
              ],
            ),
            const SizedBox(height: 18),
            Row(
              children: [
                Expanded(
                  child: _AxisValue(axis: 'X', value: ax),
                ),
                Expanded(
                  child: _AxisValue(axis: 'Y', value: ay),
                ),
                Expanded(
                  child: _AxisValue(axis: 'Z', value: az),
                ),
              ],
            ),
          ],
        ),
      ),
    );
  }
}

class _AxisValue extends StatelessWidget {
  const _AxisValue({required this.axis, required this.value});

  final String axis;
  final double value;

  @override
  Widget build(BuildContext context) {
    return Column(
      children: [
        Text(
          axis,
          style: const TextStyle(
            color: DaybreakTheme.deepOrange,
            fontWeight: FontWeight.w700,
          ),
        ),
        const SizedBox(height: 4),
        Text(
          value.toStringAsFixed(2),
          style: Theme.of(context).textTheme.titleLarge,
        ),
        const Text(
          'm/s²',
          style: TextStyle(color: DaybreakTheme.muted, fontSize: 11),
        ),
      ],
    );
  }
}
