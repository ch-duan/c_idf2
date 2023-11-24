#pragma once

#ifdef __cplusplus
extern "C" {
#endif
int median_filter(int *input, int input_size);
int avg_filter(int *data, int len);
void bubbleSort(int *data, int len);
int Deduplication(int *data, int len);
int KalmanFilter(int inData);
#ifdef __cplusplus
}
#endif
