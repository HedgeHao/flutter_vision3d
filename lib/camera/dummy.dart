import 'camera.dart';

class DummyCamera extends FvCamera {
  static String generateSerial() => 'dummy-${DateTime.now().millisecondsSinceEpoch}';

  DummyCamera(Map<String, dynamic> m) : super(m);
}
