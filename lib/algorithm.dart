import 'dart:math';

double sigmoid(double x) {
  return 1 / (1 + exp(-x));
}

List<double> softmax(List<double> input) {
  int i;
  double m, sum, constant;
  int size = input.length;

  m = -double.infinity;

  for (i = 0; i < size; i++) {
    if (m < input[i]) m = input[i];
  }

  sum = 0.0;
  for (i = 0; i < size; ++i) {
    sum += exp(input[i] - m);
  }

  constant = m + log(sum);
  for (i = 0; i < size; ++i) {
    input[i] = exp(input[i] - constant);
  }

  return input;
}
