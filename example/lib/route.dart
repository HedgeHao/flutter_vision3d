abstract class AppRoutes {
  static const home = '/home';
  static const realsense = '/realsense';
  static const uvc = '/uvc';
  static const fr = '/fr';
  static const efficientNet = '/efficient_net';
  static const handDetection = '/hand_detection';
  static const opencv = '/opencv';

  static String join(List<String> args) => args.join('');
}
