#include <kasaLight.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string>
#include <iostream>

unsigned char* encode(const char* data, int length){

    unsigned char* encodedData = new unsigned char[length];
    unsigned char xorByte = 171;
    for(int i = 0; i < length; i++){
        encodedData[i] = data[i] ^ xorByte;
        xorByte = encodedData[i];
    }

    return encodedData;

}


kasaLight::kasaLight(std::string address){

    this->sock = socket(AF_INET, SOCK_STREAM, 0);
    //error
    sockaddr_in bulbAddress;
    bulbAddress.sin_family = AF_INET;
    bulbAddress.sin_port = htons(9999);
    bulbAddress.sin_addr.s_addr = inet_addr(address.c_str());

    int connection = connect(sock, (struct sockaddr*)&bulbAddress, sizeof(bulbAddress));
    //connect error

}

bool kasaLight::ON(){

    return this->customCommand("{\"smartlife.iot.smartbulb.lightingservice\":{\"transition_light_state\":{\"on_off\":1,\"ignore_default\":1,\"transition_period\":1}}}");

}


bool kasaLight::OFF(){

    return this->customCommand("{\"smartlife.iot.smartbulb.lightingservice\":{\"transition_light_state\":{\"on_off\":0,\"ignore_default\":1,\"transition_period\":1}}}");

}


bool kasaLight::setBrightness(int brightness){

    if(brightness > 100 || brightness < 0){
        return false;
    }
    return this->customCommand("{\"smartlife.iot.smartbulb.lightingservice\":{\"transition_light_state\":{\"brightness\":" + std::to_string(brightness) + ",\"transition_period\":1}}}");

}


bool kasaLight::setColor(int hue){

    return this->setColor(hue, 100);

}


bool kasaLight::setColor(int hue, int saturation){

    if(hue < 0 || hue > 300 || saturation < 0 || saturation > 100){
        return false;
    }
    return this->customCommand("{\"smartlife.iot.smartbulb.lightingservice\":{\"transition_light_state\":{\"color_temp\":0,\"hue\":" + std::to_string(hue) + ",\"saturation\":" + std::to_string(saturation) + ",\"transition_period\":1}}}");

}


bool kasaLight::customCommand(std::string command){

    int length = command.length();
    uint32_t len32 = static_cast<uint32_t>(length);
    unsigned char* len8s = reinterpret_cast<unsigned char*>(&len32);

    unsigned char sendData[4 + length];
    sendData[0] = len8s[3];
    sendData[1] = len8s[2];
    sendData[2] = len8s[1];
    sendData[3] = len8s[0];

    const char* messageC = command.c_str();
    unsigned char* encodedData = encode(messageC, length);
    for(int i = 0; i < length; i++){
        sendData[i + 4] = encodedData[i];
    }

    send(this->sock, sendData, length + 4, 0);
    //error

    return true;

}
