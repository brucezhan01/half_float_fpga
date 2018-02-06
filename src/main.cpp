#include <stdio.h>
#include "kernel.h"

#define M    8
#define N    8
#define K    8

void fgemm_ref(unsigned int m_len, unsigned int n_len, unsigned int k_len, float *A, float *B, float *C, float *Y)
{
    for (unsigned int m = 0; m < m_len; ++m) {
        for (unsigned int n = 0; n < n_len; ++n) {
            for (unsigned int k = 0; k < k_len; ++k) {
                unsigned int a_idx = m*k_len+k;
                unsigned int b_idx = k*n_len+n;
                unsigned int y_idx = m*n_len+n;
                Y[y_idx] += A[a_idx]*B[b_idx];
            }
        }
    }
}
void dgemm_ref(unsigned int m_len, unsigned int n_len, unsigned int k_len, data_t *A, data_t *B, data_t *C, data_t *Y)
{
    for (unsigned int m = 0; m < m_len; ++m) {
        for (unsigned int n = 0; n < n_len; ++n) {
            for (unsigned int k = 0; k < k_len; ++k) {
                unsigned int a_idx = m*k_len+k;
                unsigned int b_idx = k*n_len+n;
                unsigned int y_idx = m*n_len+n;
                Y[y_idx] += A[a_idx]*B[b_idx];
            }
        }
    }
}

float fA[M*K];
float fB[K*N];
float fC[M*N];
float fY[M*N];
data_t dSrcBuf[M*K+K*N+M*N];
data_t dDstBuf[M*N];
data_t *dA = &dSrcBuf[0];
data_t *dB = &dSrcBuf[0+M*K];
data_t *dC = &dSrcBuf[0+M*K+K*N];
data_t dY[M*N];
data_t *dY_hw = &dDstBuf[0];
int main()
{
#if 1
    float min_error = 1e-3;
    long bigerr_cnt = 0;
    long bigerr_cnt_hw = 0;
    float err = 0.0f;

    for (unsigned int m = 0; m < M; ++m) {
        for (unsigned int k = 0; k < K; ++k) {
            float r = static_cast<float>(rand())/static_cast<float>(RAND_MAX)*2.0f - 1.0f; // range: [-1, 1]
            unsigned int a_idx = m*K+k;
            fA[a_idx] = r;
            dA[a_idx] = data_t(r);
        }
    }
    for (unsigned int k = 0; k < K; ++k) {
        for (unsigned int n = 0; n < N; ++n) {
            float r = static_cast<float>(rand())/static_cast<float>(RAND_MAX)*2.0f - 1.0f; // range: [-1, 1]
            unsigned int b_idx = k*N+n;
            fB[b_idx] = r;
            dB[b_idx] = data_t(r);
        }
    }
    for (unsigned int m = 0; m < M; ++m) {
        for (unsigned int n = 0; n < N; ++n) {
            float r = 0.0f;
            unsigned int y_idx = m*N+n;
            fY[y_idx] = r;
            dY[y_idx] = data_t(r);
            dY_hw[y_idx] = data_t(r);
        }
    }
    printf("M(%d),N(%d),K(%d),A(0x%08x),B(0x%08x),C(0x%08x),Y(0x%08x),srcb(0x%08x),dstb(0x%08x)\n",M,N,K,dA,dB,dC,dY_hw,&dSrcBuf[0], &dDstBuf[0]);
    fgemm_ref(M, N, K, fA, fB, fC, fY);
    dgemm_ref(M, N, K, dA, dB, dC, dY);

#if 1
    instw_t instw_buf[16];
    instw_set_op(&instw_buf[0], op_gemm);
    instw_set_gemm_m(&instw_buf[0],M);
    instw_set_gemm_n(&instw_buf[0],N);
    instw_set_gemm_k(&instw_buf[0],K);
    instw_set_gemm_a(&instw_buf[1],0);
    instw_set_gemm_b(&instw_buf[1],M*K);
    instw_set_gemm_c(&instw_buf[2],M*K+K*N);
    instw_set_gemm_y(&instw_buf[2],0);
    param_t params_src;
    param_t params_dst;
    params_src.instw_num = 3;
    params_src.instw_buf = instw_buf;
    params_src.in_buf = &dSrcBuf[0];
    params_src.out_buf = &dDstBuf[0];
    params_dst.out_buf = &dDstBuf[0];
    params_dst.status = -1;
    kernel_top(&params_src, &params_dst);
#endif

    for (unsigned int m = 0; m < M; ++m) {
        for (unsigned int n = 0; n < N; ++n) {
            unsigned int y_idx = m*N+n;
            err = fabs(fY[y_idx]-(float)dY[y_idx]);
            if (err > min_error) {
                ++bigerr_cnt;
                #if 1
                printf("Error (%.8f) is greater than %.8f\n", err, min_error);
                #endif
            }
        }
    }
    printf("(cpfp impl) Total %ld count which error is bigger than %.8f\n", bigerr_cnt, min_error);
    for (unsigned int m = 0; m < M; ++m) {
        for (unsigned int n = 0; n < N; ++n) {
            unsigned int y_idx = m*N+n;
            err = fabs((float)dY[y_idx]-(float)dY_hw[y_idx]);
            if (err > 1e-5) {
                ++bigerr_cnt_hw;
                #if 1
                printf("Error (%.8f) is greater than %.8f\n", err, min_error);
                #endif
            }
        }
    }
    printf("(cpfp impl hw) Total %ld count which error is bigger than %.8f\n", bigerr_cnt_hw, 1e-5);

#else
    float a[LENGTH];
    float b[LENGTH];
    float c[LENGTH];
    
    half ha[LENGTH];
    half hb[LENGTH];
    half hc[LENGTH];   
    
    cpfp ha_1[LENGTH];
    cpfp hb_1[LENGTH];
    cpfp hc_1[LENGTH];
    
    float min_error = 1e-4;
    long bigerr_cnt = 0;
    long bigerr1_cnt = 0;
    float err = 0.0f;
    float err_1 = 0.0f;
    
    for (int i = 0; i < LENGTH; ++i) {
        float r = static_cast<float>(rand())/static_cast<float>(RAND_MAX)*2.0f - 1.0f; // range: [-1, 1]
        a[i] = r;
        r = static_cast<float>(rand())/static_cast<float>(RAND_MAX)*2.0f - 1.0f;
        b[i] = r;
        c[i] = a[i]*b[i];
        
        ha[i] = half_cast<half, std::round_to_nearest>(a[i]);
        hb[i] = half_cast<half, std::round_to_nearest>(b[i]);
        hc[i] = ha[i]*hb[i];
        
        ha_1[i] = cpfp(a[i]);
        hb_1[i] = cpfp(b[i]);
        hc_1[i] = ha_1[i]*hb_1[i];

        err = fabs(c[i]-(float)hc[i]);
        err_1 = fabs(c[i]-(float)hc_1[i]);
        
        #if 0
        printf("==================== %d\n", i);
        printf("a=%.8f, b=%.8f, c=%.8f\n", a[i],b[i],c[i]);
        printf("ha=%.8f, hb=%.8f, hc=%.8f, hc_bin=0x%08x, hc_bin_hfloat=%.8f\n", (float)ha[i],(float)hb[i],(float)hc[i],hc[i].get_binary(),(float)half::from_binary(hc[i].get_binary()));
        printf("ha_1=%.8f, hb_1=%.8f, hc_1=%.8f\n", (float)ha_1[i],(float)hb_1[i],(float)hc_1[i]);
        printf("err=%.8f\n", err);
        printf("err_1=%.8f\n", err_1);
        printf("\n");
        #endif
        
        if (err > min_error) {
            ++bigerr_cnt;
            #if 0
            printf("Error (%.8f) is greater than %.8f\n", err, min_error);
            #endif
        }
        if (err_1 > min_error) {
            ++bigerr1_cnt;
            #if 0
            printf("Error-1 (%.8f) is greater than %.8f\n", err_1, min_error);
            #endif
        }
    }
    
    printf("(half impl) Total %ld count which error is bigger than %.8f\n", bigerr_cnt, min_error);
    printf("(cpfp impl) Total %ld count which error is bigger than %.8f\n", bigerr1_cnt, min_error);
#endif
    
    return 0;
}
