#include "im2col.h"

void im2col_cpu(const double* data_im, int dataOffset, const int channels,
    const int height, const int width, const int kernel_h, const int kernel_w,
    const int stride_h, const int stride_w,
    double* data_col) {

  data_im += dataOffset;
  const int channel_size = height * width;

  for (int fitting_height = 0; fitting_height <= (height - kernel_h); fitting_height += stride_h) {
    for (int fitting_width = 0; fitting_width <= (width - kernel_w); fitting_width += stride_w) {
      for (int channel = 0; channel < channels; channel++) {
        for (int kernel_row = 0; kernel_row < kernel_h; kernel_row++) {
          for (int kernel_col = 0; kernel_col < kernel_w; kernel_col++) {
            int input_row = fitting_height + kernel_row;
            int input_col = fitting_width + kernel_col;
            *(data_col++) = data_im[input_row * width + input_col + channel_size * channel];
          }
        }
      }
    }
  }
}

void col2im_cpu(const double* data_col, int dataOffset, const int channels,
    const int height, const int width, const int kernel_h, const int kernel_w,
    const int stride_h, const int stride_w,
    double* data_im) {

  memset(data_im, 0, height * width * channels * sizeof(double));
  data_col += dataOffset;

  const int channel_size = height * width;

  for (int fitting_height = 0; fitting_height <= (height - kernel_h); fitting_height += stride_h) {
    for (int fitting_width = 0; fitting_width <= (width - kernel_w); fitting_width += stride_w) {
      for (int channel = 0; channel < channels; channel++) {
        for (int kernel_row = 0; kernel_row < kernel_h; kernel_row++) {
          for (int kernel_col = 0; kernel_col < kernel_w; kernel_col++) {
            int input_row = fitting_height + kernel_row;
            int input_col = fitting_width + kernel_col;
            data_im[input_row * width + input_col + channel_size * channel] += *(data_col++);
          }
        }
      }
    }
  }
}

inline int max ( int a, int b ) { return a > b ? a : b; }

void pool_forwards_cpu(const double* data_im, int dataOffset, const int channels,
    const int height, const int width, const int kernel_h, const int kernel_w,
    const int stride_h, const int stride_w,
    double* data_pooled) {

  data_im += dataOffset;

  const int channel_size = height * width;

  for (int channel = 0; channel < channels; channel++) {
    for (int fitting_height = 0; fitting_height <= (height - kernel_h); fitting_height += stride_h) {
      for (int fitting_width = 0; fitting_width <= (width - kernel_w); fitting_width += stride_w) {
        // Start with the value in 0,0
        int max_value = data_im[fitting_height * width + fitting_width + channel_size * channel];
        // Initial row, skipping the corner we've done
        for (int kernel_col = 1; kernel_col < kernel_w; kernel_col++) {
          int input_row  = fitting_height;
          int input_col  = fitting_width + kernel_col;
          max_value = max ( max_value, data_im[input_row * width + input_col + channel_size * channel] );
        }
        // The remaining rows
        for (int kernel_row = 1; kernel_row < kernel_h; kernel_row++) {
          for (int kernel_col = 0; kernel_col < kernel_w; kernel_col++) {
            int input_row = fitting_height + kernel_row;
            int input_col = fitting_width + kernel_col;
            max_value = max ( max_value, data_im[input_row * width + input_col + channel_size * channel] );
          }
        }
        *(data_pooled++) = max_value;
      }
    }
  }
}

void pool_backwards_cpu(const double* data_im, int data_im_offset,
    const double* data_pooled, int data_pooled_offset,
    const int channels, const int height, const int width, const int kernel_h,
    const int kernel_w, const int stride_h, const int stride_w,
    double* data_backgrad ) {

  data_im += data_im_offset;
  data_pooled += data_pooled_offset;
  memset(data_backgrad, 0, height * width * channels * sizeof(double));

  const int channel_size = height * width;

  for (int channel = 0; channel < channels; channel++) {
    for (int fitting_height = 0; fitting_height <= (height - kernel_h); fitting_height += stride_h) {
      for (int fitting_width = 0; fitting_width <= (width - kernel_w); fitting_width += stride_w) {
        int max_index = fitting_height * width + fitting_width + channel_size * channel;
        int max_value = data_im[max_index];
        for (int kernel_col = 1; kernel_col < kernel_w; kernel_col++) {
          int input_row  = fitting_height;
          int input_col  = fitting_width + kernel_col;
          int data_index = input_row * width + input_col + channel_size * channel;
          int data_value = data_im[data_index];
          if ( data_value > max_value )  {
              max_value = data_value;
              max_index = data_index;
          }
        }
        for (int kernel_row = 1; kernel_row < kernel_h; kernel_row++) {
          for (int kernel_col = 0; kernel_col < kernel_w; kernel_col++) {
            int input_row = fitting_height + kernel_row;
            int input_col = fitting_width + kernel_col;
            int data_index = input_row * width + input_col + channel_size * channel;
            int data_value = data_im[data_index];
            if ( data_value > max_value )  {
              max_value = data_value;
              max_index = data_index;
            }
          }
        }
        data_backgrad[max_index] += *(data_pooled++);
      }
    }
  }
}
