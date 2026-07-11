import 'package:flutter/material.dart';

class DaybreakTheme {
  // ===== Daybreak base colors =====
  static const Color bg = Color(0xFF1C1E26);
  static const Color fg = Color(0xFFD5D8DA);
  static const Color cursor = Color(0xFFFAC29A);
  static const Color selBg = Color(0xFF333543);

  static const Color black = Color(0xFF1A1C23);
  static const Color red = Color(0xFFE95678);
  static const Color green = Color(0xFF29D398);
  static const Color yellow = Color(0xFFFAB795);
  static const Color blue = Color(0xFF26BBD9);
  static const Color purple = Color(0xFFB877DB);
  static const Color cyan = Color(0xFF59E1E3);
  static const Color white = Color(0xFFD5D8DA);

  static const Color brightBlack = Color(0xFF333543);
  static const Color brightWhite = Color(0xFFFDF0ED);
  static const Color deepOrange = Color(0xFFFAB28E);
  static const Color muted = Color(0xFF9DA2AA);

  static final ColorScheme colorScheme =
      const ColorScheme.dark(
        brightness: Brightness.dark,
        primary: deepOrange,
        onPrimary: bg,

        secondary: purple,
        onSecondary: brightWhite,

        tertiary: blue,
        onTertiary: bg,

        error: red,
        onError: brightWhite,

        surface: bg,
        onSurface: fg,
      ).copyWith(
        surfaceContainerHighest: selBg,
        outline: brightBlack,
        shadow: Colors.black54,
      );

  static ThemeData get theme {
    return ThemeData(
      useMaterial3: true,
      brightness: Brightness.dark,
      colorScheme: colorScheme,
      scaffoldBackgroundColor: bg,
      canvasColor: bg,

      appBarTheme: const AppBarTheme(
        backgroundColor: black,
        foregroundColor: fg,
        elevation: 0,
        centerTitle: true,
        surfaceTintColor: Colors.transparent,
        titleTextStyle: TextStyle(
          color: fg,
          fontSize: 20,
          fontWeight: FontWeight.w700,
        ),
      ),

      cardTheme: CardThemeData(
        color: const Color(0xFF232530),
        surfaceTintColor: Colors.transparent,
        elevation: 0,
        margin: const EdgeInsets.all(10),
        shape: RoundedRectangleBorder(
          borderRadius: BorderRadius.circular(16),
          side: const BorderSide(color: brightBlack, width: 1),
        ),
      ),

      dividerColor: brightBlack,

      textTheme: const TextTheme(
        headlineLarge: TextStyle(color: fg, fontWeight: FontWeight.w700),
        headlineMedium: TextStyle(color: fg, fontWeight: FontWeight.w700),
        titleLarge: TextStyle(
          color: fg,
          fontSize: 22,
          fontWeight: FontWeight.w700,
        ),
        titleMedium: TextStyle(color: fg, fontWeight: FontWeight.w600),
        bodyLarge: TextStyle(color: fg, fontSize: 16),
        bodyMedium: TextStyle(color: fg, fontSize: 14),
        bodySmall: TextStyle(color: Color(0xFFBBBBBB), fontSize: 12),
        labelLarge: TextStyle(color: bg, fontWeight: FontWeight.w700),
      ),

      iconTheme: const IconThemeData(color: deepOrange),

      elevatedButtonTheme: ElevatedButtonThemeData(
        style: ElevatedButton.styleFrom(
          backgroundColor: deepOrange,
          foregroundColor: bg,
          elevation: 0,
          padding: const EdgeInsets.symmetric(horizontal: 18, vertical: 14),
          shape: RoundedRectangleBorder(
            borderRadius: BorderRadius.circular(14),
          ),
          textStyle: const TextStyle(fontWeight: FontWeight.w700),
        ),
      ),

      outlinedButtonTheme: OutlinedButtonThemeData(
        style: OutlinedButton.styleFrom(
          foregroundColor: deepOrange,
          side: const BorderSide(color: deepOrange),
          padding: const EdgeInsets.symmetric(horizontal: 18, vertical: 14),
          shape: RoundedRectangleBorder(
            borderRadius: BorderRadius.circular(14),
          ),
        ),
      ),

      textButtonTheme: TextButtonThemeData(
        style: TextButton.styleFrom(
          foregroundColor: purple,
          textStyle: const TextStyle(fontWeight: FontWeight.w600),
        ),
      ),

      inputDecorationTheme: InputDecorationTheme(
        filled: true,
        fillColor: black,
        labelStyle: const TextStyle(color: Color(0xFFBBBBBB)),
        hintStyle: const TextStyle(color: Color(0xFFBBBBBB)),
        prefixIconColor: deepOrange,
        suffixIconColor: deepOrange,
        border: OutlineInputBorder(
          borderRadius: BorderRadius.circular(14),
          borderSide: const BorderSide(color: brightBlack),
        ),
        enabledBorder: OutlineInputBorder(
          borderRadius: BorderRadius.circular(14),
          borderSide: const BorderSide(color: brightBlack),
        ),
        focusedBorder: OutlineInputBorder(
          borderRadius: BorderRadius.circular(14),
          borderSide: const BorderSide(color: deepOrange, width: 1.4),
        ),
        errorBorder: OutlineInputBorder(
          borderRadius: BorderRadius.circular(14),
          borderSide: const BorderSide(color: red),
        ),
        focusedErrorBorder: OutlineInputBorder(
          borderRadius: BorderRadius.circular(14),
          borderSide: const BorderSide(color: red, width: 1.4),
        ),
      ),

      chipTheme: ChipThemeData(
        backgroundColor: selBg,
        selectedColor: purple.withValues(alpha: 0.22),
        disabledColor: brightBlack,
        labelStyle: const TextStyle(color: fg),
        secondaryLabelStyle: const TextStyle(color: fg),
        padding: const EdgeInsets.symmetric(horizontal: 10, vertical: 6),
        shape: RoundedRectangleBorder(
          borderRadius: BorderRadius.circular(12),
          side: const BorderSide(color: brightBlack),
        ),
      ),

      checkboxTheme: CheckboxThemeData(
        fillColor: WidgetStateProperty.resolveWith((states) {
          if (states.contains(WidgetState.selected)) return deepOrange;
          return Colors.transparent;
        }),
        checkColor: WidgetStateProperty.all(bg),
        side: const BorderSide(color: brightBlack),
        shape: RoundedRectangleBorder(borderRadius: BorderRadius.circular(4)),
      ),

      radioTheme: RadioThemeData(
        fillColor: WidgetStateProperty.resolveWith((states) {
          if (states.contains(WidgetState.selected)) return deepOrange;
          return const Color(0xFFBBBBBB);
        }),
      ),

      switchTheme: SwitchThemeData(
        thumbColor: WidgetStateProperty.resolveWith((states) {
          if (states.contains(WidgetState.selected)) return deepOrange;
          return fg;
        }),
        trackColor: WidgetStateProperty.resolveWith((states) {
          if (states.contains(WidgetState.selected)) {
            return deepOrange.withValues(alpha: 0.35);
          }
          return brightBlack;
        }),
      ),

      floatingActionButtonTheme: const FloatingActionButtonThemeData(
        backgroundColor: deepOrange,
        foregroundColor: bg,
        elevation: 2,
      ),

      snackBarTheme: SnackBarThemeData(
        backgroundColor: black,
        contentTextStyle: const TextStyle(color: fg),
        actionTextColor: deepOrange,
        shape: RoundedRectangleBorder(borderRadius: BorderRadius.circular(14)),
        behavior: SnackBarBehavior.floating,
      ),

      dialogTheme: DialogThemeData(
        backgroundColor: const Color(0xFF232530),
        surfaceTintColor: Colors.transparent,
        shape: RoundedRectangleBorder(borderRadius: BorderRadius.circular(18)),
        titleTextStyle: const TextStyle(
          color: fg,
          fontSize: 20,
          fontWeight: FontWeight.w700,
        ),
        contentTextStyle: const TextStyle(color: fg, fontSize: 14),
      ),

      bottomSheetTheme: const BottomSheetThemeData(
        backgroundColor: bg,
        surfaceTintColor: Colors.transparent,
      ),

      listTileTheme: const ListTileThemeData(
        iconColor: deepOrange,
        textColor: fg,
        tileColor: Colors.transparent,
      ),

      progressIndicatorTheme: const ProgressIndicatorThemeData(
        color: deepOrange,
        linearTrackColor: brightBlack,
        circularTrackColor: brightBlack,
      ),
    );
  }
}
