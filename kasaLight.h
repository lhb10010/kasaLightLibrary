#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string>

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
        void setTransition(int transition);
        int getTransition();
        int getSocket();

    private:
        int sock;
        int transition;


};