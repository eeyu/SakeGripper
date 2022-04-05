#ifndef __GRIPPER__
#define __GRIPPER__

// ECat Gripper interfaces between ecat and the LL gripper

#include <DynamixelShield.h>
#include "Timer.h"
#include "LowLevelGripper.h"
 

#define GRIPPER_SCALE_MIN 0
#define GRIPPER_SCALE_MAX 100

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
    int last_position = 10000;

public:
    Gripper() {

    }

    Gripper(int nid, DynamixelShield *ndxl) {
        dxl_id = nid;
        dxl = ndxl;
        setSingleResolution();
        zero_position = 0;

        is_busy = false;
        check_motion_timer.reset(0.05);
    }

    // Find the difference between write address and write word in python
    void calibrate() {
        debugPrintln(String("Calibrating: ") + dxl_id);
        dxl->setOperatingMode(dxl_id, OP_EXTENDED_POSITION);
        dxl->writeControlTableItem(ControlTableItem::TORQUE_LIMIT, dxl_id, 500); // torque limit 500
        dxl->writeControlTableItem(ControlTableItem::TORQUE_ENABLE, dxl_id, 0); // torque enable off
        dxl->writeControlTableItem(ControlTableItem::TORQUE_CTRL_MODE_ENABLE, dxl_id, 1); // goal torque mode on
        dxl->writeControlTableItem(ControlTableItem::GOAL_TORQUE, dxl_id, 1024 + 100); // goal torque CW and value 100

        delay(4 * 1000); // time to fully close

        dxl->writeControlTableItem(ControlTableItem::GOAL_TORQUE, dxl_id, 1024 + 10); // reduce the load or something
        dxl->writeControlTableItem(ControlTableItem::MULTI_TURN_OFFSET, dxl_id, 0); //  multi turn offset is 0
        zero_position = getAbsolutePosition();

        debugPrintln("Calibrated");
    }

    // 0 to 100 for each
    void gotoPositionWithTorque(int position, int closing_torque) { 
        if (!is_busy) {
            position = clamp(position, GRIPPER_SCALE_MIN, GRIPPER_SCALE_MAX);
            closing_torque = clamp(closing_torque, GRIPPER_SCALE_MIN, GRIPPER_SCALE_MAX);
            int servo_position = scale(position, GRIP_MAX);
            setMaxEffort(closing_torque);

            if (position == 0) {
                closeWithTorque();
            } else {
                gotoServoPosition(servo_position);
            }

            int holding_torque = min(TORQUE_HOLD, closing_torque);
            setMaxEffort(holding_torque);
        } else {
            debugPrintln("Busy");
        }
    }

    void release() {
        if (!is_busy) {
            setTorqueMode(false);
        } else {
            debugPrintln("Busy");
        }
    }

    void open() {
        if (!is_busy) {
            gotoPositionWithTorque(100, 100);
        } else {
            debugPrintln("Busy");
        }
    }

    int getTemperature() {
        return dxl->readControlTableItem(ControlTableItem::PRESENT_TEMPERATURE, dxl_id);
    }

    bool isBusy() {
        return is_busy;
    }

    void operate() {
        if (check_motion_timer.timeOut()) {
            if (!isMoving()) {
                is_busy = false;
            }
            last_position = getAbsolutePosition();
            check_motion_timer.reset();
        }
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
        int result = map(n, GRIPPER_SCALE_MIN, GRIPPER_SCALE_MAX, 0, max);
        return clamp(result, 0, max);
    }

    int downScale(int n, int max) {
        int result = map(n, 0, max, GRIPPER_SCALE_MIN, GRIPPER_SCALE_MAX);
        return clamp(result, 0, GRIPPER_SCALE_MAX);
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
