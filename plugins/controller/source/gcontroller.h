#ifndef GCONTROLLER_H
#define GCONTROLLER_H

#include <string>

class GHID;

class GControllerDPadAxis
{
public:
    GControllerDPadAxis(GHID *ghid, unsigned int dID, bool x);

    void handle(float value, float lastValue);

private:
    GHID *ghid_;
    bool xAxis;
    unsigned int deviceID;
};

class GControllerTrigger
{
public:
    GControllerTrigger(GHID *ghid, unsigned int dID, bool dispatch);
    void handleBoth(float value, float lastValue);
    void handleLeft(float value, float lastValue);
    void handleRight(float value, float lastValue);

private:
    GHID *ghid_;
    unsigned int deviceID;
    bool leftDown;
    bool rightDown;
    bool shouldDispatch;
};

class GControllerJoystick
{
public:
    GControllerJoystick(GHID *ghid, unsigned int dID, bool left);
    void handleXAxis(float value, float lastValue);
    void handleYAxis(float value, float lastValue);
    void handle();

private:
    GHID *ghid_;
    float xAxis;
    float yAxis;
    float lastX;
    float lastY;
    bool isLeft;
    unsigned int deviceID;
};

class GController
{
public:
    GController(GHID *ghid, unsigned int dID, const char* pname, int btnCount, int vID, int pID);

    ~GController();

    unsigned int getId();

    const char* getName();

    void handleButtonDown(unsigned int buttonID);

    void handleButtonUp(unsigned int buttonID);

    void handleAxisMove(unsigned int axisID, float value, float lastValue);

    void copyArray(const int data[]);

private:
    GHID *ghid_;
    unsigned int deviceID;
    std::string name;
    int vendorID;
    int productID;
    GControllerDPadAxis *xDPad;
    GControllerDPadAxis *yDPad;
    GControllerTrigger *trigger;
    GControllerJoystick *rightStick;
    GControllerJoystick *leftStick;
    int *buttons;
    int *axis;
    int buttonCount;
    int buttonOffset;
    bool dispatchTrigger;
};

#endif // GCONTROLLER_H
