#ifndef __LLGRIPPER__
#define __LLGRIPPER__

#include <DynamixelShield.h>
#include "Timer.h"
#include "Parameters.h"
#include "MathUtil.h"
#include "GripperError.h"
#include "Hourglass.h"

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
    Hourglass nonzeroTorqueHourglass;
    Hourglass safeTorqueExceededHourglass;

    Timer printTimer;
    int last_position = 10000;
    int maxTemperature;

    int desiredTorqueLimit = RAW_MAX_TORQUE;
    int desiredTorque = 0;
    int desiredPosition = 0;

    GripperError error = NONE;

public:
    LowLevelGripper() {

    }

    LowLevelGripper(int nid, DynamixelShield *ndxl) {
        dxl_id = nid;
        dxl = ndxl;
        
        zero_position = 0;
        is_busy = false;
        check_motion_timer.set(0.05);

        safeTorqueExceededHourglass = Hourglass(SAFE_TORQUE_EXCEEDED_MAX_TIME);
        nonzeroTorqueHourglass = Hourglass(NONZERO_TORQUE_MAX_TIME);

        setSingleResolution();
        // maxTemperature = dxl->readControlTableItem(ControlTableItem::TEMPERATURE_LIMIT, dxl_id);
        maxTemperature = MAX_TEMPERATURE;
        // Safety check. Shuts down on torque, temperature errors
        dxl->writeControlTableItem(ControlTableItem::SHUTDOWN, dxl_id, 36);

        printTimer.set(1.0);
    }

    void operate() {
        if (printTimer.isRinging()) {
            printTimer.restart();
            // DEBUG_SERIAL.println(" ----");
            // DEBUG_SERIAL.print(String(" _DT ") +getRawDesiredTorque());
            // DEBUG_SERIAL.print(String(" _LT ") +getRawTorqueLimit());
            // DEBUG_SERIAL.print(String(" _MT ") +getRawMeasuredTorque());

            // DEBUG_SERIAL.print(String(" Des_T ") +desiredTorque);
            // DEBUG_SERIAL.print(String(" Lim_T ") +desiredTorqueLimit);
            // DEBUG_SERIAL.print(String(" HGST ") + safeTorqueExceededHourglass.getTimeLeftForwardSec());
            // DEBUG_SERIAL.print(String(" HG0T ") + nonzeroTorqueHourglass.getTimeLeftForwardSec());
            // DEBUG_SERIAL.print(String(" Err ") + error);
            // DEBUG_SERIAL.print(String(" Tmp ") +getTemperature());
        }
        if (calibration_timer.isTickingDown()) 
        {
            if (calibration_timer.isRinging()) 
            {
                finishCalibration();
                calibration_timer.stopRinging();
            }
        }
        else 
        {
            performSafetyChecks();
            sendDesiredsToDynamixel();
        }
        if (check_motion_timer.isRinging()) 
        {
            last_position = getRawAbsolutePosition();
            check_motion_timer.restart();
        }
    }

private:
    // Lower on chain is more severe error
    void performSafetyChecks() {
        desiredTorqueLimit = RAW_MAX_TORQUE;
        error = NONE;
        // when no motion is detected, reduce to safe torque. 
        if (!isMoving()) 
        {
            is_busy = false;
            setTorqueLimitWithError(RAW_SAFE_TORQUE, SAFE_TORQUE_LIMIT);
        }

        // if safe torque is exceeded for n cumulative seconds, send to timeout
        // limit resets after timeout period
        if (torqueIsExceeded(RAW_SAFE_TORQUE, true))
        {
            safeTorqueExceededHourglass.runForward();
        }
        else
        {
            safeTorqueExceededHourglass.runBackward();
        }
        if (safeTorqueExceededHourglass.lastEmptiedSideIsForward())
        {
            setTorqueLimitWithError(RAW_SAFE_TORQUE, SAFE_TORQUE_LIMIT);
        }

        // if torque is nonzero for n cumulative seconds, limit to 0 until operator resets
        if (torqueIsExceeded(RAW_TORQUE_NOISE_MAGNITUDE))
        {
            nonzeroTorqueHourglass.runForward();
        }
        else if (!nonzeroTorqueHourglass.lastEmptiedSideIsForward())
        {
            nonzeroTorqueHourglass.runBackward();
        }
        if (nonzeroTorqueHourglass.outOfTimeForward()) 
        {
            setTorqueLimitWithError(0, ZERO_TORQUE_LIMIT);
        }

        // if temperature > maximum, completely remove torque. Cannot operate until cooldown
        if (getTemperature() >= maxTemperature) 
        {
            debugPrintln("Temperature Exceeded");
            setTorqueLimitWithError(0, TEMPERATURE);
        }
    }

    bool torqueIsExceeded(int rawTorque, bool inclusive = false) {
        if (inclusive)
        {
            // return (abs(getRawMeasuredTorque()) >= rawTorque ||
            //     getRawTorqueLimit() >= rawTorque ||
            //     abs(getRawDesiredTorque()) >= rawTorque);
            return (abs(getRawMeasuredTorque()) >= rawTorque);
        }
        else
        {
            // return (abs(getRawMeasuredTorque()) > rawTorque ||
            //     getRawTorqueLimit() > rawTorque ||
            //     abs(getRawDesiredTorque()) > rawTorque);
            return (abs(getRawMeasuredTorque()) > rawTorque);
        }

    }

    void setTorqueLimitWithError(int limit, GripperError nerror) {
        desiredTorqueLimit = min(limit, desiredTorqueLimit);
        error = nerror;
    }

    void sendDesiredsToDynamixel() {
        int torque = constrain(desiredTorque, 0, desiredTorqueLimit);
        
        if (torque == 0)
        {
            dxl->writeControlTableItem(ControlTableItem::TORQUE_ENABLE, dxl_id, 0);
        }
        else
        {
            dxl->writeControlTableItem(ControlTableItem::TORQUE_LIMIT, dxl_id, torque);
            dxl->writeControlTableItem(ControlTableItem::GOAL_TORQUE, dxl_id, (unsigned short)(1024+torque));

            if (desiredPosition == 0) {
                closeWithTorque();
            } else {
                gotoRawPosition(desiredPosition);
            }
        }

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

        removeTorque();
        debugPrintln(String("Calibrated: ") + zero_position);
    }

public:
    void setZero(int zero) {
        zero_position = zero;
    }

    void setDesiredPositionAndTorque(float positionRatio, float torqueRatio) { 
        int position = convertRatioToRawDynamixel(positionRatio, RAW_MAX_OPEN_POSITION);
        int torque = convertRatioToRawDynamixel(torqueRatio, RAW_MAX_TORQUE);
        
        desiredTorque = torque;
        desiredPosition = position;
    }

    void open() {
        setDesiredPositionAndTorque(1.0, 1.0);
    }

    void setTorque(float torqueRatio) {
        int torque = convertRatioToRawDynamixel(torqueRatio, RAW_MAX_TORQUE);
        desiredTorque = torque;
    }

    void clearErrorAndResetLimit() {
        desiredTorqueLimit = RAW_MAX_TORQUE;
        error = NONE;
        safeTorqueExceededHourglass.reset();
        nonzeroTorqueHourglass.reset();
        removeTorque();
    }

    void removeTorque() {
        setTorque(0);
        setTorqueMode(false);
    }

// Accessors
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

    GripperError getError() {
        return error;
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
        if (rawTorque >= 1024) {
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
