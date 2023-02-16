import 'camera.dart';

enum RealsenseConfiguration { THRESHOLD_FILTER, ALIGN_TO_COLOR, ALIGN_TO_DEPTH }

class RealsenseCamera extends FvCamera {
  RealsenseCamera(Map<String, dynamic> m) : super(m);
}
