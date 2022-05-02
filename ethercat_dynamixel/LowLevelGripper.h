#ifndef __LLGRIPPER__
#define __LLGRIPPER__

#include <DynamixelShield.h>
#include "Timer.h"

class LowLevelGripper {
private:
    int GRIP_MAX = 2500;
    int TORQUE_MAX = 800;
    int TORQUE_HOLD = 13;

    DynamixelShield *dxl;
    int dxl_id;
    int zero_position;

    bool is_busy;
    Timer check_motion_timer;
    Timer calibration_timer;
    bool is_in_calibration = false;
    int last_position = 10000;

public:
    LowLevelGripper() {

    }

    LowLevelGripper(int nid, DynamixelShield *ndxl) {
        dxl_id = nid;
        dxl = ndxl;
        setSingleResolution();
        zero_position = 0;

        is_busy = false;
        check_motion_timer.reset(0.05);
        calibration_timer.reset(4.0);

        // Safety checks. Shuts down
        dxl->writeControlTableItem(ControlTableItem::SHUTDOWN, dxl_id, 0x3D);
    }

    void operate() {
        if (is_in_calibration) 
        {
            if (calibrationHasFinished()) 
            {
                finishCalibration();
            }
        } 
        else 
        {
            if (check_motion_timer.timeOut()) 
            {
                if (!isMoving()) 
                {
                    is_busy = false;
                }
                last_position = getAbsolutePosition();
                check_motion_timer.reset();
            }
        }
    }

    // Close the gripper to set position
    void calibrate() {
        debugPrintln(String("Calibrating: ") + dxl_id);
        beginCalibration();
        calibration_timer.reset(4.0); // will probably take this much time to fully close
        is_busy = true;
    }

    int getPosition() {
        // Get position in dynamixel units
        int rawServoPosition = getAbsolutePosition() - zero_position;
        // Scale to (0,100)
        return downScale(rawServoPosition, GRIP_MAX);
    }

    void setZero(int zero) {
        zero_position = zero;
    }

    bool exceededOperationalSafetyChecks() {
        // if (torque > max torque for n seconds) {
        //     return true;
        // }
        
        int currentTemperature = dxl->readControlTableItem(ControlTableItem::PRESENT_TEMPERATURE, dxl_id);
        int maxTemperature = dxl->readControlTableItem(ControlTableItem::TEMPERATURE_LIMIT, dxl_id);
        if (currentTemperature > maxTemperature) {
            return true;
        }
        return false;
    }

private:
    void beginCalibration() {
        is_in_calibration = true;
        dxl->setOperatingMode(dxl_id, OP_EXTENDED_POSITION);
        dxl->writeControlTableItem(ControlTableItem::TORQUE_LIMIT, dxl_id, 500); // torque limit 500
        dxl->writeControlTableItem(ControlTableItem::TORQUE_ENABLE, dxl_id, 0); // torque enable off
        dxl->writeControlTableItem(ControlTableItem::TORQUE_CTRL_MODE_ENABLE, dxl_id, 1); // goal torque mode on
        dxl->writeControlTableItem(ControlTableItem::GOAL_TORQUE, dxl_id, 1024 + 100); // goal torque CW and value 100
    }
    
    void finishCalibration() {
        dxl->writeControlTableItem(ControlTableItem::GOAL_TORQUE, dxl_id, 1024 + 10); // reduce the load or something
        dxl->writeControlTableItem(ControlTableItem::MULTI_TURN_OFFSET, dxl_id, 0); //  multi turn offset is 0
        zero_position = getAbsolutePosition();
        is_in_calibration = false;
        debugPrintln("Calibrated");
    }

    bool calibrationHasFinished() {
        return calibration_timer.timeOut() && is_in_calibration;
    }



public:
    // 0 to 100 for each
    void gotoPositionWithTorque(int position, int closing_torque) { 
        // if (!is_busy) {
        position = clamp(position, GRIPPER_RESOLUTION_MIN, GRIPPER_RESOLUTION_MAX);
        closing_torque = clamp(closing_torque, GRIPPER_RESOLUTION_MIN, GRIPPER_RESOLUTION_MAX);
        int servo_position = scale(position, GRIP_MAX);
        setMaxEffort(closing_torque);

        if (position == 0) {
            closeWithTorque();
        } else {
            gotoServoPosition(servo_position);
        }

        // int holding_torque = min(TORQUE_HOLD, closing_torque);
        // setMaxEffort(holding_torque);
        setMaxEffort(closing_torque);
        // } else {
        //     debugPrintln("Busy");
        // }
    }

    void release() {
        // if (!is_busy) {
        setTorqueMode(false);
        // } else {
        //     debugPrintln("Busy");
        // }
    }

    void open() {
        // if (!is_busy) {
        gotoPositionWithTorque(GRIPPER_RESOLUTION_MAX, GRIPPER_RESOLUTION_MAX);
        // } else {
        //     debugPrintln("Busy");
        // }
    }

    void setTorque(int torque) {
        torque = clamp(torque, GRIPPER_RESOLUTION_MIN, GRIPPER_RESOLUTION_MAX);
        setMaxEffort(torque);
    }

    int getTemperature() {
        return dxl->readControlTableItem(ControlTableItem::PRESENT_TEMPERATURE, dxl_id);
    }

    bool isBusy() {
        return is_busy;
    }

    void setBusy() {
        is_busy = true;
    }

private:
    void closeWithTorque() {
        setTorqueMode(true);
        setBusy();
    }

    void gotoServoPosition(int position) {
        setTorqueMode(false);
        dxl->writeControlTableItem(ControlTableItem::GOAL_POSITION, dxl_id, (zero_position + position));
        setBusy();
    }

    bool isMoving() {
        return (last_position != getAbsolutePosition());
    }

    void setTorqueMode(bool mode_on) {
        if (mode_on) {
            dxl->writeControlTableItem(ControlTableItem::TORQUE_CTRL_MODE_ENABLE, dxl_id, 1);
        } else {
            dxl->writeControlTableItem(ControlTableItem::TORQUE_CTRL_MODE_ENABLE, dxl_id, 0);
        }
    }

    int getAbsolutePosition() {
        return dxl->readControlTableItem(ControlTableItem::PRESENT_POSITION, dxl_id);
    }

    void setSingleResolution() {
        dxl->writeControlTableItem(ControlTableItem::RESOLUTION_DIVIDER, dxl_id, 1);
    }

    // input on scale of 0 to 100
    void setMaxEffort(int max_effort) {
        int scaled_torque = scale(max_effort, TORQUE_MAX);
        dxl->writeControlTableItem(ControlTableItem::TORQUE_LIMIT, dxl_id, scaled_torque);
        dxl->writeControlTableItem(ControlTableItem::GOAL_TORQUE, dxl_id, (unsigned short)(1024+scaled_torque));
    }

    int scale(int n, int max) {
        int result = map(n, GRIPPER_RESOLUTION_MIN, GRIPPER_RESOLUTION_MAX, 0, max);
        return clamp(result, 0, max);
    }

    int downScale(int n, int max) {
        int result = map(n, 0, max, GRIPPER_RESOLUTION_MIN, GRIPPER_RESOLUTION_MAX);
        return clamp(result, 0, GRIPPER_RESOLUTION_MAX);
    }

    int clamp(int x, int min, int max) {
        if (x < min) {
            return min;
        } else if (x > max) {
            return max;
        } else {
            return x;
        }
    }



private:
    void debugPrintln(String s) {
        #ifdef DEBUG_SERIAL
        DEBUG_SERIAL.println(s);
        #endif
    }

    void debugPrint(String s) {
        #ifdef DEBUG_SERIAL
        DEBUG_SERIAL.print(s);
        #endif
    }
};

#endif
