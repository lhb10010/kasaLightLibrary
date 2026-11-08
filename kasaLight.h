//kasaLight.h

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string>

struct bulbInfo{

    std::string alias;
    bool on;
    std::string mode;
    int brightness;
    int hue;
    int saturation;
    int temp;
    int errorCode;

};


class kasaLight{

    public:

        kasaLight(std::string address);
        bool setColor(unsigned char red, unsigned char green, unsigned char blue);
        bool ON();
        bool OFF();
        bool setColor(int hue, int saturation);
        bool setColor(int hue);
        bool customCommand(std::string command);
        bool setBrightness(int brightness);
        bool setWhite();
        bool setWhite(int temp);
        bool blink();
        bool blink(int times);
        bool blink(int times, int delay);
        void setTransition(int transition);
        int getTransition();
        int getSocket();
        bulbInfo getAllInfo();
        bulbInfo customCommandReturnInfo(std::string command);
        bool getPower();
        std::string getMode();
        int getBrightness();
        int getHue();
        int getSaturation();
        int getTemp();
        int getErrorCode();
        std::string getAlias();
        static std::string encryptCommand(std::string command);
        ~kasaLight();
        std::string getInfoString();

    private:
        unsigned char* sendCommand(std::string command, int* outputLength);
        int sock;
        int transition;


};



