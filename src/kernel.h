#pragma once

#include <ap_int.h>
#include "cpfp.hpp"

#define LANE_NUM        16
#define MAX_INST_WORD_NUM    32

typedef cpfp data_t;

struct lane_t {
    data_t val[LANE_NUM];
};

typedef enum {op_vmultadd, op_gemm, op_gemv, op_conv, op_relu, op_leaky, op_maxpool} op_type_t;
typedef enum {status_ok, status_err_op, status_err_instw_num} status_type_t;

typedef ap_uint<64> instw_t;

struct param_t {
    unsigned int instw_num;
    instw_t *instw_buf;
    data_t *in_buf;
    data_t *out_buf;
    unsigned int status;
};


inline void instw_set_op(instw_t *instw, unsigned int op) {
    (*instw).range(7,0) = op;
}
inline unsigned int instw_get_op(instw_t instw) {
    return instw.range(7,0).to_uint();
}

inline void instw_set_gemm_m(instw_t *instw, unsigned int m) {
    (*instw).range(31,16) = m;
}
inline void instw_set_gemm_n(instw_t *instw, unsigned int n) {
    (*instw).range(47,32) = n;
}
inline void instw_set_gemm_k(instw_t *instw, unsigned int k) {
    (*instw).range(63,48) = k;
}
inline void instw_set_gemm_a(instw_t *instw, unsigned int a) {
    (*instw).range(31,0) = a;
}
inline void instw_set_gemm_b(instw_t *instw, unsigned int b) {
    (*instw).range(63,32) = b;
}
inline void instw_set_gemm_c(instw_t *instw, unsigned int c) {
    (*instw).range(31,0) = c;
}
inline void instw_set_gemm_y(instw_t *instw, unsigned int y) {
    (*instw).range(63,32) = y;
}
inline unsigned int instw_get_gemm_m(instw_t instw) {
    return instw.range(31,16).to_uint();
}
inline unsigned int instw_get_gemm_n(instw_t instw) {
    return instw.range(47,32).to_uint();
}
inline unsigned int instw_get_gemm_k(instw_t instw) {
    return instw.range(63,48).to_uint();
}
inline unsigned int instw_get_gemm_a(instw_t instw) {
    return instw.range(31,0).to_uint();
}
inline unsigned int instw_get_gemm_b(instw_t instw) {
    return instw.range(63,32).to_uint();
}
inline unsigned int instw_get_gemm_c(instw_t instw) {
    return instw.range(31,0).to_uint();
}
inline unsigned int instw_get_gemm_y(instw_t instw) {
    return instw.range(63,32).to_uint();
}

void kernel_top(param_t const *gmem_rd, param_t *gmem_wr);

