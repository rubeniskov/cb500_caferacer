#include <math.h>

typedef struct __KalmanTypeDef
{          
  float ErrorEstimate;
  float ErrorMeasure;
  float ProcessNoise;

  float Gain; 

  float CurrentEstimate; 
  float LastEstimate;  
} KalmanTypeDef;

void KalmanInit(KalmanTypeDef *kalman, float errorMeasure, float errorEstimate, float processNoise) {
    kalman->ErrorMeasure = errorMeasure;
    kalman->ErrorEstimate = errorEstimate;
    kalman->ProcessNoise = processNoise;
}

float KalmanUpdateEstimate(KalmanTypeDef *kalman, float value) {
    kalman->Gain = kalman->ErrorEstimate / (kalman->ErrorEstimate + kalman->ErrorMeasure);
    kalman->CurrentEstimate = kalman->LastEstimate + kalman->Gain * (value - kalman->LastEstimate);
    kalman->ErrorEstimate = (1.0 - kalman->Gain) * kalman->ErrorEstimate 
                            + fabs(kalman->LastEstimate - kalman->CurrentEstimate) 
                            * kalman->ProcessNoise;
    kalman->LastEstimate = kalman->CurrentEstimate;

    return kalman->CurrentEstimate;
}