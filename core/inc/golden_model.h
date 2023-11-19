#include <inttypes.h>

#ifndef MIN
#define MIN(x, y) (((x) < (y)) ? (x) : (y))
#endif

#ifndef MAX
#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#endif

#define ROCC_INSTRUCTION_R_R_R(x, rd, rs1, rs2, func7)                         \
  {                                                                            \
    asm volatile(".insn r " STR(CAT(CUSTOM_, x)) ", " STR(0x7) ", " STR(       \
                     func7) ", %0, %1, %2"                                     \
                 : "=r"(rd)                                                    \
                 : "r"(rs1), "r"(rs2));                                        \
  }

#define ROCC_INSTRUCTION_0_R_R(x, rs1, rs2, func7)                             \
  {                                                                            \
    asm volatile(".insn r " STR(CAT(CUSTOM_, x)) ", " STR(0x3) ", " STR(       \
                     func7) ", x0, %0, %1"                                     \
                 :                                                             \
                 : "r"(rs1), "r"(rs2));                                        \
  }

void rand_dense_matrix(int8_t *dense_matrix, uint8_t dense_matrix_stride,
                       uint8_t dense_matrix_size, uint8_t num_dense_rows,
                       uint8_t range) {
  //printf("\nDense Matrix\n");
  for (uint8_t i = 0; i < num_dense_rows; i++) {
    // printf("\n");
    for (uint8_t j = 0; j < dense_matrix_size * dense_matrix_stride; j++) {
      dense_matrix[(i * dense_matrix_stride * dense_matrix_size) + j] =
          (rand() % range) - (range / 2);
      printf("%d, ",
             dense_matrix[(i * dense_matrix_stride * dense_matrix_size) + j]);
    }
  }
}

void rand_sparse_row(int8_t *sparse_row, uint8_t sparse_row_size,
                     uint8_t num_dense_rows, uint8_t range) {
  //printf("\nSparse Row\n");
  for (uint8_t i = 0; i < 2 * sparse_row_size; i += 2) {
    sparse_row[i] = (rand() % range) - (range / 2);
    sparse_row[i + 1] = rand() % num_dense_rows;
    // printf("%d, ", sparse_row[i]);
  }
}

void accel_model(int8_t *sparse_row, uint8_t sparse_row_size,
                 int8_t *dense_matrix, uint8_t dense_matrix_stride,
                 uint8_t dense_matrix_size, int8_t *output_buf) {
  int32_t *temp_buf = malloc(32 * dense_matrix_stride * dense_matrix_size);
  // printf("\nBeginning Model\n");
  for (uint8_t i = 0; i < dense_matrix_size * dense_matrix_stride; i++) {
    temp_buf[i] =
        MAX(-49280,
            MIN(49151,
                (int32_t)sparse_row[0] *
                    (int32_t)dense_matrix[(sparse_row[1] * dense_matrix_stride *
                                           dense_matrix_size) +
                                          i]));
  }
  for (uint8_t i = 2; i < 2 * sparse_row_size; i += 2) {
    for (uint8_t j = 0; j < dense_matrix_size * dense_matrix_stride; j++) {
      temp_buf[j] =
          MAX(-49280,
              MIN(49151,
                  temp_buf[j] + (int32_t)sparse_row[i] *
                                (int32_t)dense_matrix[(sparse_row[i + 1] *
                                                    dense_matrix_stride *
                                                    dense_matrix_size) +
                                                    j]));
    }
  }
  for (uint8_t i = 0; i < dense_matrix_size * dense_matrix_stride; i++) {
    output_buf[i] = MAX(-128, MIN(127, temp_buf[i]));
  }
  free(temp_buf);
}
