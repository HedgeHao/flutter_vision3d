class OpenCVBarcodeResult {
/*
{
  "px1": 123.234,
  "py1": 123.234,
  "px2": 123.234,
  "py2": 123.234,
  "px3": 123.234,
  "py3": 123.234,
  "px4": 123.234,
  "py4": 123.234,
  "data": "123.234"
} 
*/

  double? px1;
  double? py1;
  double? px2;
  double? py2;
  double? px3;
  double? py3;
  double? px4;
  double? py4;
  String? data;

  OpenCVBarcodeResult({
    this.px1,
    this.py1,
    this.px2,
    this.py2,
    this.px3,
    this.py3,
    this.px4,
    this.py4,
    this.data,
  });
  OpenCVBarcodeResult.fromJson(Map<dynamic, dynamic> json) {
    px1 = json['px1']?.toDouble();
    py1 = json['py1']?.toDouble();
    px2 = json['px2']?.toDouble();
    py2 = json['py2']?.toDouble();
    px3 = json['px3']?.toDouble();
    py3 = json['py3']?.toDouble();
    px4 = json['px4']?.toDouble();
    py4 = json['py4']?.toDouble();
    data = json['data']?.toString();
  }
  Map<String, dynamic> toJson() {
    final data = <String, dynamic>{};
    data['px1'] = px1;
    data['py1'] = py1;
    data['px2'] = px2;
    data['py2'] = py2;
    data['px3'] = px3;
    data['py3'] = py3;
    data['px4'] = px4;
    data['py4'] = py4;
    data['data'] = this.data;
    return data;
  }
}
