#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <immintrin.h>

void sqrtSerial(int N,
                float initialGuess,
                float values[],
                float output[])
{

    static const float kThreshold = 0.00001f;

    for (int i=0; i<N; i++) {

        float x = values[i];
        float guess = initialGuess;

        float error = fabs(guess * guess * x - 1.f);

        while (error > kThreshold) {
            guess = (3.f * guess - x * guess * guess * guess) * 0.5f;
            error = fabs(guess * guess * x - 1.f);
        }

        output[i] = x * guess;
    }
}

// void simd_sqrt(int N, float startGuess, float* values, float* output)
// {
//     __mmask8 mask;
//     __m256 one = _mm256_set1_ps(1.0);
//     __m256 three = _mm256_set1_ps(3.0);
//     __m256 half = _mm256_set1_ps(0.5);
//     __m256 kThreshold = _mm256_set1_ps(0.00001f);
//     __m256 x, guess, error, tmp, tmp2;
//     __m256 abs = _mm256_castsi256_ps(_mm256_set1_epi32(0x80000000));
//     for(int i=0; i<N; i+=8) {
//         mask = 0xff;
//         x = _mm256_load_ps(values+i);
//         guess = _mm256_set1_ps(startGuess);
//         tmp = _mm256_mask_mul_ps(guess, mask, guess, guess);
//         tmp = _mm256_mask_mul_ps(tmp, mask, tmp, x);
//         error = _mm256_mask_sub_ps(tmp, mask, tmp, one);
//         error = _mm256_mask_andnot_ps(error, mask, error, abs);
//         mask = mask & (_mm256_cmp_ps_mask(error, kThreshold, 0x1e));
//         while(mask) {
//             tmp = _mm256_mask_mul_ps(guess, mask, guess, three);
//             tmp2 = _mm256_mask_mul_ps(guess, mask, guess, guess);
//             tmp2 = _mm256_mask_mul_ps(tmp2, mask, guess, tmp2);
//             tmp2 = _mm256_mask_mul_ps(tmp2, mask, tmp2, x);
//             tmp = _mm256_mask_sub_ps(tmp, mask, tmp, tmp2);
//             guess = _mm256_mask_mul_ps(guess, mask, tmp, half);

//             tmp = _mm256_mask_mul_ps(guess, mask, guess, guess);
//             tmp = _mm256_mask_mul_ps(tmp, mask, tmp, x);
//             error = _mm256_mask_sub_ps(tmp, mask, tmp, one);
//             error = _mm256_mask_andnot_ps(error, mask, error, abs);
//             mask = mask & (_mm256_cmp_ps_mask(error, kThreshold, 0x1e));
//         }
//         guess = _mm256_mul_ps(guess, x);
//         _mm256_store_ps(output+i, guess);
//     }
// }