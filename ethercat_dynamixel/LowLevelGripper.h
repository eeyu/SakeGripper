#ifndef __LLGRIPPER__
#define __LLGRIPPER__

#include <DynamixelShield.h>
#include "Timer.h"
#include "Parameters.h"
#include "MathUtil.h"

// Manages dynamixel interaction for single gripper
// TODO how much noise is in present load?

// <max torque> in % of physical max on 0 to 1023
// <torque limit> affects speed during position control. also limits max torque. in % of
//  physical max on 0 to 1023
// <goal torque> affects torque in torque control mode. in mA on 0 to 2047
// <present load> when stopped, jumps to at most 24, usually around 16. in % of physical max

class LowLevelGripper {
private:
    DynamixelShield *dxl;
    int dxl_id;
    int zero_position;

    bool is_busy;
    Timer check_motion_timer;
    Timer calibration_timer;
    Timer safeTorqueExceededTimer;
    Timer nonzeroTorqueTimer;
    int last_position = 10000;
    int maxTemperature;

public:
    LowLevelGripper() {

    }

    LowLevelGripper(int nid, DynamixelShield *ndxl) {
        dxl_id = nid;
        dxl = ndxl;
        
        zero_position = 0;
        is_busy = false;
        check_motion_timer.set(0.05);

        setSingleResolution();
        maxTemperature = dxl->readControlTableItem(ControlTableItem::TEMPERATURE_LIMIT, dxl_id);

        // Safety check. Shuts down on torque, temperature errors
        dxl->writeControlTableItem(ControlTableItem::SHUTDOWN, dxl_id, 0x3D);
    }

    void operate() {
        // DEBUG_SERIAL.println(calibration_timer.checkTimeLeft());
        if (calibration_timer.isRinging()) 
        {
            finishCalibration();
        }
        else 
        {
            if (check_motion_timer.isRinging()) 
            {
                last_position = getRawAbsolutePosition();
                check_motion_timer.restart();
            }
        }
        if (!calibration_timer.isTickingDown()) {
            performSafetyChecks();
        }
    }

private:
    void performSafetyChecks() {
        // if no motion is detected, reduce to safe torque
        if (!isMoving()) 
        {
            is_busy = false;
            reduceToSafeTorque();
            // debugPrintln("No Motion Detected");
        }

        // if safe torque is exceeded for n seconds, reduce to safe torque
        if (!safeTorqueExceededTimer.isTickingDown() && torqueIsExceeded(RAW_SAFE_TORQUE)) 
        {
            // debugPrint("Starting unsafe torque timer for n seconds: "); debugPrintln(String(SAFE_TORQUE_EXCEEDED_MAX_TIME));
            safeTorqueExceededTimer.set(SAFE_TORQUE_EXCEEDED_MAX_TIME);
        }
        if (safeTorqueExceededTimer.isRinging())
        {
            reduceToSafeTorque();
            safeTorqueExceededTimer.stopRinging();
            // debugPrintln("Torque is above safe level for n seconds");
        }
        
        // if torque > 0 for m seconds, completely remove torque
        if (!nonzeroTorqueTimer.isTickingDown() && torqueIsExceeded(RAW_TORQUE_NOISE_MAGNITUDE)) 
        {
            nonzeroTorqueTimer.set(NONZERO_TORQUE_MAX_TIME);
            // debugPrint("Starting nonzero torque timer for n seconds: "); debugPrintln(String(NONZERO_TORQUE_MAX_TIME));
        }
        if (nonzeroTorqueTimer.isRinging())
        {
            // debugPrintln("Torque is nonzero for n seconds");
            removeTorque();
            nonzeroTorqueTimer.stopRinging();
        }

        // if temperature > maximum, completely remove torque
        if (getTemperature() > maxTemperature) 
        {
            // debugPrintln("Temperature Exceeded");
            removeTorque();
        }
    }

    bool torqueIsExceeded(int rawTorque) {
        return (abs(getRawMeasuredTorque()) > rawTorque ||
            getRawTorqueLimit() > rawTorque ||
            abs(getRawDesiredTorque()) > rawTorque);
    }

public:
    // Close the gripper to set position
    void calibrate() {
        debugPrintln(String("Calibrating: ") + dxl_id);
        beginCalibration();
        calibration_timer.set(4.0); // will probably take this much time to fully close
        is_busy = true;
    }

private:
    void beginCalibration() {
        dxl->setOperatingMode(dxl_id, OP_EXTENDED_POSITION);
        dxl->writeControlTableItem(ControlTableItem::TORQUE_LIMIT, dxl_id, 500); // torque limit 500
        dxl->writeControlTableItem(ControlTableItem::TORQUE_ENABLE, dxl_id, 0); // torque enable off
        dxl->writeControlTableItem(ControlTableItem::TORQUE_CTRL_MODE_ENABLE, dxl_id, 1); // goal torque mode on
        dxl->writeControlTableItem(ControlTableItem::GOAL_TORQUE, dxl_id, 1024 + 100); // goal torque CW and value 100
    }
    
    void finishCalibration() {
        dxl->writeControlTableItem(ControlTableItem::GOAL_TORQUE, dxl_id, 1024 + 10); // reduce the load or something
        dxl->writeControlTableItem(ControlTableItem::MULTI_TURN_OFFSET, dxl_id, 0); //  multi turn offset is 0
        zero_position = getRawAbsolutePosition();
        debugPrintln("Calibrated");
    }

public:
    void setZero(int zero) {
        zero_position = zero;
    }

    void gotoPositionWithTorque(float positionRatio, float torqueRatio) { 
        int position = convertRatioToRawDynamixel(positionRatio, RAW_MAX_OPEN_POSITION);
        int torque = convertRatioToRawDynamixel(torqueRatio, RAW_MAX_TORQUE);
        
        setRawTorque(torque);
        if (position == 0) {
            closeWithTorque();
        } else {
            gotoRawPosition(position);
        }
    }

    void open() {
        gotoPositionWithTorque(1.0, 1.0);
    }

    void setTorque(float torqueRatio) {
        int torque = convertRatioToRawDynamixel(torqueRatio, RAW_MAX_TORQUE);
        setRawTorque(torque);
    }

    void reduceToSafeTorque() {
        setRawTorque(RAW_SAFE_TORQUE);
        // debugPrintln("Reducing to safe torque");
    }

    void removeTorque() {
        setTorque(0);
        setTorqueMode(false);
        // debugPrintln("Removing torque");
    }

public:
    int getTemperature() {
        return dxl->readControlTableItem(ControlTableItem::PRESENT_TEMPERATURE, dxl_id);
    }

    float getPositionRatio() {
        int rawServoPosition = getRawAbsolutePosition() - zero_position;
        return convertRawDynamixelToRatio(rawServoPosition, RAW_MAX_OPEN_POSITION);
    }

    float getTorqueRatioMagnitude() {
        int rawTorque = abs(getRawMeasuredTorque());
        return convertRawDynamixelToRatio(rawTorque, RAW_MAX_TORQUE);
    }

    bool isBusy() {
        return is_busy;
    }

    int getError() {
        return 0; // placeholder. for now it does nothing
    }

private:
    void closeWithTorque() {
        setTorqueMode(true);
        is_busy = true;
    }

    void gotoRawPosition(int position) {
        setTorqueMode(false);
        dxl->writeControlTableItem(ControlTableItem::GOAL_POSITION, dxl_id, (zero_position + position));
        is_busy = true;
    }

    void setRawTorque(int torque) {
        dxl->writeControlTableItem(ControlTableItem::TORQUE_LIMIT, dxl_id, torque);
        dxl->writeControlTableItem(ControlTableItem::GOAL_TORQUE, dxl_id, (unsigned short)(1024+torque));
    }

    bool isMoving() {
        return (last_position != getRawAbsolutePosition());
    }

    void setTorqueMode(bool mode_on) {
        if (mode_on) {
            dxl->writeControlTableItem(ControlTableItem::TORQUE_CTRL_MODE_ENABLE, dxl_id, 1);
        } else {
            dxl->writeControlTableItem(ControlTableItem::TORQUE_CTRL_MODE_ENABLE, dxl_id, 0);
        }
    }

    int getRawAbsolutePosition() {
        return dxl->readControlTableItem(ControlTableItem::PRESENT_POSITION, dxl_id);
    }

    int getRawMeasuredTorque() {
        int rawTorque = dxl->readControlTableItem(ControlTableItem::PRESENT_LOAD, dxl_id);
        if (rawTorque > 1024) {
            rawTorque -= 1024;
            return -rawTorque;
        }
        else {
            return rawTorque;
        }   
    }

    int getRawTorqueLimit() {
        return dxl->readControlTableItem(ControlTableItem::TORQUE_LIMIT, dxl_id);
    }

    int getRawDesiredTorque() {
        int rawTorque = dxl->readControlTableItem(ControlTableItem::GOAL_TORQUE, dxl_id);
        if (rawTorque > 1024) {
            rawTorque -= 1024;
            return -rawTorque;
        }
        else {
            return rawTorque;
        }   
    }

    void setSingleResolution() {
        dxl->writeControlTableItem(ControlTableItem::RESOLUTION_DIVIDER, dxl_id, 1);
    }


private:
    int convertRatioToRawDynamixel(float ratio, int maximum) {
        int rawDynamixel = (int) (ratio * maximum);
        return constrain(rawDynamixel, 0, maximum);
    }

    float convertRawDynamixelToRatio(int rawDynamixel, int maximum) {
        float ratio = (1.0 * rawDynamixel) / maximum;
        return fconstrain(ratio, 0.0, 1.0);
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
