#include "median_filter.h"

#include <stdio.h>

#define WINDOW_SIZE 5

int median_filter(int *input, int input_size) {
  int output = 0;
  int half_window = WINDOW_SIZE / 2;
  int window[WINDOW_SIZE];
  for (int i = half_window; i < input_size - half_window; i++) {
    // Copy input data into current window:
    for (int j = 0; j < WINDOW_SIZE; j++) {
      window[j] = input[i + j - half_window];
    }
    // Sort window:
    for (int j = 0; j < WINDOW_SIZE - 1; j++) {
      for (int k = j + 1; k < WINDOW_SIZE; k++) {
        int ss = 0;
        if (window[j] > window[k]) {
          int temp = window[j];
          window[j] = window[k];
          window[k] = temp;
          ss = 1;
        }
        if (!ss) {
          break;
        }
      }
    }
    // Output median value:
    output = window[half_window];
  }
  return output;
}

void bubbleSort(int *data, int len) {
  if (len < 1) {
    return;
  }
  int arrBoundary = len - 1;
  int lastSwapIndex = 0;
  for (int i = 0; i < len - 1; i++) {
    int ok = 1;
    for (int j = 0; j < arrBoundary; j++) {
      if (data[j] > data[j + 1]) {
        int d = data[j];
        data[j] = data[j + 1];
        data[j + 1] = d;
        ok = 0;
        lastSwapIndex = j;
      }
    }

    arrBoundary = lastSwapIndex;
    if (ok) {
      return;
    }
  }
}

int Deduplication(int *data, int len) {
  if (len < 0) {
    return -1;
  }

  int slow = 0, fast = 1;
  while (fast < len) {
    if (data[fast] != data[slow]) {
      slow = slow + 1;
      data[slow] = data[fast];
    }
    fast = fast + 1;
  }
  return slow + 1;
}

int avg_filter(int *data, int len) {
  if (len < 2) {
    return -1;
  }
  int sum = 0;
  for (int i = 1; i < len - 1; i++) {
    sum += data[i];
  }
  return sum / (len - 2);
}

int KalmanFilter(int inData) {
  static float prevData = 0;                             // 先前数值
  static float p = 10, q = 0.001, r = 0.001, kGain = 0;  // q控制误差  r控制响应速度

  p = p + q;
  kGain = p / (p + r);                                // 计算卡尔曼增益
  inData = prevData + (kGain * (inData - prevData));  // 计算本次滤波估计值
  p = (1 - kGain) * p;                                // 更新测量方差
  prevData = inData;
  return inData;                                      // 返回滤波值
}