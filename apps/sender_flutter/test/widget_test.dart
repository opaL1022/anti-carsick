import 'dart:convert';

import 'package:flutter_test/flutter_test.dart';
import 'package:motion_sender/main.dart';
import 'package:motion_sender/motion_packet.dart';

void main() {
  testWidgets('shows connection controls and starts idle', (tester) async {
    await tester.pumpWidget(const MyApp());

    expect(find.text('ANTI CARSICK'), findsOneWidget);
    expect(find.text('Windows IP'), findsOneWidget);
    expect(find.text('Port'), findsOneWidget);
    expect(find.text('開始傳送'), findsOneWidget);
    expect(find.text('待機'), findsOneWidget);
  });

  test('motion packet uses the receiver protocol', () {
    const packet = MotionPacket(timestamp: 123.5, ax: 1, ay: -2, az: 0.25);
    final json = jsonDecode(utf8.decode(packet.encode())) as Map<String, dynamic>;

    expect(json, {'t': 123.5, 'ax': 1.0, 'ay': -2.0, 'az': 0.25});
  });
}
