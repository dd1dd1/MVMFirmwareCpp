// 
// 
// 

#include "PressureLoop.h"

void PressureLoopClass::init(int32_t LoopRatio)
{
    _LoopRatio = LoopRatio;
    LoopCounter = 0;
    _PID_P = 70;
    _PID_I = 12;
    _PID_D = 0;
    _PID_P2 = 1.2;
    _PID_I2 = 0.4;
    _PID_D2 = 0;
    _pid_limit = 0.55;
    _filter_fast = 0.9;
    _filter_slow = 0.7;
    _ValvePWM = 0;
}


void PressureLoopClass::PID_SLOW_LOOP()
{

    static float pid_error = 0;
    static float pid_integral = 0;
    static float pid_prec = 0;
    static float pid_out = 0;

    float PID_P = _PID_P2;
    float PID_I = _PID_I2;
    float PID_D = _PID_D2;
    static float Pset2 = 0;

    static float pid_outb = 0;

    float Pmeas = 0;

    Pmeas = _pressure_patient;

    if (_Pset == 0) {
        Pset2 = Pmeas;
        pid_integral = 0;
        pid_prec = 0;
        pid_outb = 0;
    }
    else {
        Pset2 = (Pset2 * _filter_slow) + ((1- _filter_slow) * _Pset);

        pid_error = Pset2 - Pmeas;
        pid_integral += pid_error;
       
        if (pid_integral < 0)
            pid_integral = 0;

        pid_out = PID_P * pid_error + PID_I * pid_integral + PID_D * (pid_error - pid_prec);

        pid_outb = pid_out;

        if (pid_outb < Pset2 * _pid_limit)
            pid_outb = Pset2 * _pid_limit;
        if (pid_outb > 50)
            pid_outb = 50;

        pid_prec = pid_error;

        fast_pid_set = pid_outb;
    }

}

void PressureLoopClass::PID_FAST_LOOP()
{

    static float pid_error = 0;
    static float pid_integral = 0;
    static float pid_prec = 0;
    static float pid_out = 0;

   
    float PID_P = _PID_P;
    float PID_I = _PID_I; 
    float PID_D = _PID_D;
    static float Pset2 = 0;

    static float pid_outb = 0;

    float Pmeas = 0;

    Pmeas = _pressure_valve;

    if (_Pset == 0) {
        Pset2 = Pmeas;
        pid_integral = 0;
        pid_prec = 0;
        ledcWrite(0, 0);
    }
    else {
        Pset2 = (Pset2 * _filter_fast) + ((1- _filter_fast) * fast_pid_set);


        pid_error = Pset2 - Pmeas;
        pid_integral += pid_error;
        if ((pid_integral * PID_I) > 4095)
            pid_integral = (4095 / PID_I);
        if ((pid_integral * PID_I) < -4095)
            pid_integral = -(4095 / PID_I);

        pid_out = PID_P * pid_error + PID_I * pid_integral + PID_D * (pid_error - pid_prec);

        //pid_outb = pid_outb * 0.8 + pid_out*0.2;
        pid_outb = pid_out;
        if (pid_outb < 0)
            pid_outb = 0;
        pid_outb = pid_outb + 500;
        if (pid_outb > 4090)
            pid_outb = 4090;

        pid_prec = pid_error;

        if (_Pset == 0)
            ledcWrite(0, 0);
        else
            ledcWrite(0, pid_outb);
    }

   
}

void PressureLoopClass::SetTargetPressure(float pressure)
{
    _Pset = pressure;
}

void PressureLoopClass::Tick()
{
    if (_LoopRatio == 0)
    {
        PID_FAST_LOOP();
        PID_SLOW_LOOP();
    }
    else
    {
        if (LoopCounter >= _LoopRatio)
        {
            PID_FAST_LOOP();
            LoopCounter = 0;
        }
        else
        {
            PID_SLOW_LOOP();
            LoopCounter++;
        }
    }
}

void PressureLoopClass::SetPressure(t_pressure_selector ps, float pressure)
{
    if (ps == PRESSURE_VALVE)
    {
        _pressure_valve = pressure;
    }
    else
    {
        if (ps == PRESSURE_PATIENT)
        {
            _pressure_patient = pressure;
        }
    }
}

void PressureLoopClass::ConfigurePidSlow(float P, float I, float D)
{
    _PID_P2 = P;
    _PID_I2 = I;
    _PID_D2 = D;

}
void PressureLoopClass::ConfigurePidFast(float P, float I, float D)
{
    _PID_P = P;
    _PID_I = I;
    _PID_D = D;
}
void PressureLoopClass::GetPidSlow(float* P, float* I, float* D)
{
    *P = _PID_P2;
    *I = _PID_I2;
    *I = _PID_D2;
}
void PressureLoopClass::GetPidFast(float* P, float* I, float* D)
{
    *P = _PID_P;
    *I = _PID_I;
    *I = _PID_D;
}
void PressureLoopClass::SetPidFilter(float fast, float slow)
{
    _filter_fast = fast;
    _filter_slow = slow;
}
void PressureLoopClass::GetPidFilter(float* fast, float* slow)
{
    *fast = _filter_fast;
    *slow = _filter_slow;
}