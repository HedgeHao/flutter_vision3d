import 'dart:math';
import 'dart:typed_data';

import 'package:flutter_vision/algorithm.dart';

int numClasses = 1;
int downsampleScale = 8;
double ratio_h = 1.0 / 224;
double ratio_w = 1.0 / 224;
int map_h = 224 ~/ downsampleScale;
int map_w = 224 ~/ downsampleScale;
double conf_thresh = 0.55;

class FaceInfo {
  double x1, y1, x2, y2, confProb;

  FaceInfo(this.x1, this.y1, this.x2, this.y2, this.confProb);
}

List<FaceInfo> processFaceDetectorOutputs(Float32List featureMap, int image_width, int image_height) {
  List<FaceInfo> faceInfos = [];
  for (int i = 0; i < map_h * map_w; i++) // each grid
  {
    int row = i ~/ map_w;
    int col = i % map_w;
    int grid_index = i * (4 + numClasses);

    // get best conf
    int conf_index = grid_index + 4;
    double best_conf = -10.0;
    int best_idx = 0;
    for (int c = 0; c < numClasses; c++) {
      if (featureMap[conf_index + c] > best_conf) {
        best_conf = featureMap[conf_index + c];
        best_idx = c;
      }
    }
    best_conf = sigmoid(best_conf);
    if (best_conf > conf_thresh) {
      // get bbox info
      int box_index = grid_index + 0;
      double x = (col + sigmoid(featureMap[box_index + 0])) / map_w;
      double y = (row + sigmoid(featureMap[box_index + 1])) / map_h;
      double w = exp(featureMap[box_index + 2]) * downsampleScale * ratio_w;
      double h = exp(featureMap[box_index + 3]) * downsampleScale * ratio_h;

      double xmin = x - w / 2;
      double ymin = y - h / 2;
      double xmax = xmin + w;
      double ymax = ymin + h;

      faceInfos.add(FaceInfo(xmin * image_width, ymin * image_height, xmax * image_width, ymax * image_height, best_conf));
    }
  }

  return faceInfos;
}

double getIOU(FaceInfo a, FaceInfo b) {
  double i_xmin = a.x1 > b.x1 ? a.x1 : b.x1;
  double i_ymin = a.y1 > b.y1 ? a.y1 : b.y1;
  double i_xmax = a.x2 < b.x2 ? a.x2 : b.x2;
  double i_ymax = a.y2 < b.y2 ? a.y2 : b.y2;

  // get area of intersection
  double intersection_w = (i_xmax - i_xmin) > 0 ? (i_xmax - i_xmin) : 0;
  double intersection_h = (i_ymax - i_ymin) > 0 ? (i_ymax - i_ymin) : 0;
  double intersection_area = intersection_w * intersection_h;

  // Area of Union
  double union_area = (a.x2 - a.x1) * (a.y2 - a.y1) + (b.x2 - b.x1) * (b.y2 - b.y1) - intersection_area;

  return intersection_area / union_area;
}

List<FaceInfo> nms(List<FaceInfo> faceInfos, double iouThreshold) {
  int removeNum = 0;
  faceInfos.sort((a, b) => a.confProb > b.confProb ? 0 : 1);
  for (int i = 0; i < faceInfos.length; i++) {
    if (faceInfos[i].confProb < 0) {
      continue;
    }

    for (int j = i + 1; j < faceInfos.length; j++) {
      if (faceInfos[j].confProb < 0) {
        continue;
      }

      if (getIOU(faceInfos[i], faceInfos[j]) > iouThreshold) {
        faceInfos[j].confProb = -1.0;
        removeNum++;
      }
    }
  }

  faceInfos.sort((a, b) => a.confProb > b.confProb ? 0 : 1);
  faceInfos.removeRange(faceInfos.length - removeNum, faceInfos.length);
  return faceInfos;
}
