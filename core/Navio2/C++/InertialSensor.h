#ifndef _INERTIAL_SENSOR_H
#define _INERTIAL_SENSOR_H

class InertialSensor {
public:
    virtual bool initialize() = 0;
    virtual bool probe() = 0;
    virtual void update() = 0;

    double read_temperature() {return temperature;};
    void read_accelerometer(double *ax, double *ay, double *az) {*ax = _ax; *ay = _ay; *az = _az;};
    void read_gyroscope(double *gx, double *gy, double *gz) {*gx = _gx; *gy = _gy; *gz = _gz;};
    void read_magnetometer(double *mx, double *my, double *mz) {*mx = _mx; *my = _my; *mz = _mz;};

protected:
    double temperature;
    double _ax, _ay, _az;
    double _gx, _gy, _gz;
    double _mx, _my, _mz;
};

#endif //_INERTIAL_SENSOR_H
