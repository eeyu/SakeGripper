#ifndef __GRIPPERERROR__
#define __GRIPPERERROR__

enum GripperError {
    NONE = 0,
    SAFE_TORQUE_LIMIT = 1,
    ZERO_TORQUE_LIMIT = 2,
    TEMPERATURE = 3
};


#endif