import 'package:flutter/material.dart';

import 'theme.dart';
import 'motion_sender_page.dart';

void main() {
  runApp(const MyApp());
}

class MyApp extends StatelessWidget {
  const MyApp({super.key});

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      title: 'Motion Sender',
      theme: DaybreakTheme.theme,
      home: const MotionSenderPage(),
      debugShowCheckedModeBanner: false,
    );
  }
}
