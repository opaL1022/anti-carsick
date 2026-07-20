import 'dart:convert';

class MotionPacket {
  const MotionPacket({
    required this.timestamp,
    required this.ax,
    required this.ay,
    required this.az,
  });

  final double timestamp;
  final double ax;
  final double ay;
  final double az;

  Map<String, double> toJson() => {
    't': timestamp,
    'ax': ax,
    'ay': ay,
    'az': az,
  };

  List<int> encode() => utf8.encode(jsonEncode(toJson()));
}
