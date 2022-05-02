#ifndef __MATHUTIL__
#define __MATHUTIL__

float fconstrain(float x, float min, float max) {
    if (x < min) {
        return min;
    }
    if (x > max) {
        return max;
    }
    return x;
}

#endif