import 'dart:async';
import 'dart:convert';
import 'dart:io';
import 'dart:math';

import 'package:flutter/material.dart';

class MotionSenderPage extends StatefulWidget {
  const MotionSenderPage({super.key});

  @override
  State<MotionSenderPage> createState() => _MotionSenderPageState();
}

class _MotionSenderPageState extends State<MotionSenderPage> {
  final _ipController = TextEditingController(text: '127.0.0.1');
  final _portController = TextEditingController(text: '9999');

  RawDatagramSocket? _socket;
  Timer? _timer;

  bool _running = false;
  double _ax = 0;
  double _ay = 0;
  double _az = 0;
  double _t = 0;

  Future<void> _startFakeSending() async {
    final host = _ipController.text.trim();
    final port = int.tryParse(_portController.text.trim());

    if (port == null) return;

    _socket ??= await RawDatagramSocket.bind(InternetAddress.anyIPv4, 0);

    _timer?.cancel();
    _timer = Timer.periodic(const Duration(milliseconds: 33), (_) {
      _t += 0.1;

      // 假資料：先模擬左右晃動與前後變化
      _ax = sin(_t) * 1.5;
      _ay = cos(_t * 0.7) * 0.8;
      _az = 9.8;

      final payload = jsonEncode({
        't': DateTime.now().millisecondsSinceEpoch / 1000.0,
        'ax': _ax,
        'ay': _ay,
        'az': _az,
      });

      _socket!.send(utf8.encode(payload), InternetAddress(host), port);

      setState(() {});
    });

    setState(() {
      _running = true;
    });
  }

  void _stopSending() {
    _timer?.cancel();
    _timer = null;
    setState(() {
      _running = false;
    });
  }

  @override
  void dispose() {
    _timer?.cancel();
    _socket?.close();
    _ipController.dispose();
    _portController.dispose();
    super.dispose();
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(title: const Text('Motion Sender')),
      body: Padding(
        padding: const EdgeInsets.all(16),
        child: Column(
          children: [
            TextField(
              controller: _ipController,
              decoration: const InputDecoration(
                labelText: 'Target IP',
                border: OutlineInputBorder(),
              ),
            ),
            const SizedBox(height: 12),
            TextField(
              controller: _portController,
              keyboardType: TextInputType.number,
              decoration: const InputDecoration(
                labelText: 'Target Port',
                border: OutlineInputBorder(),
              ),
            ),
            const SizedBox(height: 20),
            Row(
              children: [
                Expanded(
                  child: ElevatedButton(
                    onPressed: _running ? null : _startFakeSending,
                    child: const Text('Start Fake Sender'),
                  ),
                ),
                const SizedBox(width: 12),
                Expanded(
                  child: ElevatedButton(
                    onPressed: _running ? _stopSending : null,
                    child: const Text('Stop'),
                  ),
                ),
              ],
            ),
            const SizedBox(height: 24),
            Text('ax: ${_ax.toStringAsFixed(3)}'),
            Text('ay: ${_ay.toStringAsFixed(3)}'),
            Text('az: ${_az.toStringAsFixed(3)}'),
          ],
        ),
      ),
    );
  }
}
