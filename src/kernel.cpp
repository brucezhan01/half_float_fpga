#include "kernel.h"

void kernel_gemm(unsigned int M, unsigned int N, unsigned int K, data_t *A, data_t *B, data_t *C, data_t *Y)
{
    for (unsigned int m = 0; m < M; ++m) {
        for (unsigned int n = 0; n < N; ++n) {
            for (unsigned int k = 0; k < K; ++k) {
                unsigned int a_idx = m*K+k;
                unsigned int b_idx = k*N+n;
                unsigned int y_idx = m*N+n;
                Y[y_idx] += A[a_idx]*B[b_idx];
            }
        }
    }
}

void kernel_vmultadd(data_t *A, data_t *B, data_t *C, data_t *Y, unsigned int len, unsigned loop) {
}

void kernel_op(param_t const *gmem_rd, param_t *gmem_wr)
{
#pragma HLS INLINE self off
    instw_t instw_buf[MAX_INST_WORD_NUM];
    unsigned int instw_num = gmem_rd->instw_num;
    if (instw_num > MAX_INST_WORD_NUM) {
        gmem_wr->status = status_err_instw_num;
        return;
    }

    memcpy(instw_buf, gmem_rd->instw_buf, instw_num*sizeof(instw_t));
    for (unsigned int pc = 0; pc < instw_num; ) {
        unsigned int op;
        instw_t instw = instw_buf[pc++];
        op = instw_get_op(instw);
        switch (op) {
            case op_gemm:
                {
                data_t *in_buf = gmem_rd->in_buf;
                data_t *out_buf = gmem_wr->out_buf;
                unsigned int M = instw_get_gemm_m(instw);
                unsigned int N = instw_get_gemm_n(instw);
                unsigned int K = instw_get_gemm_k(instw);
                unsigned int a = instw_get_gemm_a(instw_buf[pc]);
                unsigned int b = instw_get_gemm_b(instw_buf[pc++]);
                unsigned int c = instw_get_gemm_c(instw_buf[pc]);
                unsigned int y = instw_get_gemm_y(instw_buf[pc++]);
                data_t *A = (data_t *)(in_buf + a);
                data_t *B = (data_t *)(in_buf + b);
                data_t *C = (data_t *)(in_buf + c);
                data_t *Y = (data_t *)(out_buf + y);
                printf("in_buf(0x%08x),out_buf(0x%08x),a(%d),b(%d),c(%d),y(%d)\n", in_buf,out_buf,a,b,c,y);
                printf("M(%d),N(%d),K(%d),A(0x%08x),B(0x%08x),C(0x%08x),Y(0x%08x)\n", M,N,K,A,B,C,Y);
                kernel_gemm(M,N,K,A,B,C,Y);
                gmem_wr->status = status_ok;
                }
                break;
            case op_vmultadd:
                {
                }
                break;
            default:
                gmem_wr->status = status_err_op;
        }
    }
}

void kernel_top(param_t const *gmem_rd, param_t *gmem_wr)
{
#pragma HLS INTERFACE m_axi port=gmem_rd offset=slave bundle=gmem_m num_write_outstanding=16 num_read_outstanding=16 max_write_burst_length=16 max_read_burst_length=16 depth=16 latency=125
#pragma HLS INTERFACE m_axi port=gmem_wr offset=slave bundle=gmem_m num_write_outstanding=16 num_read_outstanding=16 max_write_burst_length=16 max_read_burst_length=16 depth=16 latency=125
#pragma HLS INTERFACE s_axilite port=gmem_rd bundle=control
#pragma HLS INTERFACE s_axilite port=gmem_wr bundle=control
#pragma HLS INTERFACE s_axilite port=return bundle=control
#pragma HLS DATA_PACK variable=gmem_rd
#pragma HLS DATA_PACK variable=gmem_wr

#pragma HLS DATAFLOW

    kernel_op(gmem_rd, gmem_wr);
}
