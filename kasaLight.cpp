//kasaLight.cpp

#include </home/light/Desktop/kasaLight.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string>
#include <iostream>
#include <regex>
#include <thread>
#include <chrono>
#include <unistd.h>


std::string getMatch(std::regex r, std::string s){
    std::smatch match;
    if(!std::regex_search(s, match, r)){
        return "";
    }
    return match.str(1);
}


bulbInfo rawInfoToStruct(std::string info){
    
    if(info.empty()){
      bulbInfo b;
      b.errorCode = -1;
      return b;
    }

    std::smatch match;
    if(!std::regex_search(info, match, std::regex("\"light_state.+preferred_state\":"))){
        //error
    }
    std::string focusedInfo = match.str(0);
    
    bulbInfo b;
    int on = stoi(getMatch(std::regex("\"on_off\":([0-9]+)"), focusedInfo));
    b.on = on;
    b.mode = getMatch(std::regex("\"mode\":([0-9]+)"), focusedInfo);;
    b.brightness = stoi(getMatch(std::regex("\"brightness\":([0-9]+)"), focusedInfo));
    b.hue = stoi(getMatch(std::regex("\"hue\":([0-9]+)"), focusedInfo));
    b.saturation = stoi(getMatch(std::regex("\"saturation\":([0-9]+)"), focusedInfo));
    b.temp = stoi(getMatch(std::regex("\"color_temp\":([0-9]+)"), focusedInfo));
    b.errorCode = stoi(getMatch(std::regex("\"err_code\":([0-9]+)"), info));
    b.alias = getMatch(std::regex("\"alias\":\"([^\"]+)\","), info);
    
    return b;

}


bulbInfo rawInfoToStruct(unsigned char* info, int length){

    char convertedData[length + 1];
    for(int i = 0; i < length; i++){
        convertedData[i] = static_cast<char>(info[i]);
    }
    convertedData[length] = '\0';

    std::string infoString(convertedData);
    return rawInfoToStruct(infoString);

}


unsigned char* encode(const char* data, int length){

    unsigned char* encodedData = new unsigned char[length];
    unsigned char xorByte = 171;
    for(int i = 0; i < length; i++){
        encodedData[i] = data[i] ^ xorByte;
        xorByte = encodedData[i];
    }

    return encodedData;

}


unsigned char* encode(unsigned char* data, int length){

    unsigned char* encodedData = new unsigned char[length];
    unsigned char xorByte = 171;
    for(int i = 0; i < length; i++){
        encodedData[i] = data[i] ^ xorByte;
        xorByte = encodedData[i];
    }

    return encodedData;

}


unsigned char* decode(unsigned char* data, int length){

    unsigned char* decoded = new unsigned char[length];
    for(int i = length - 1; i > 0; i--){
        decoded[i] = data[i] ^ data[i - 1]; 
    }

    decoded[0] = data[0] ^ 171;

    return decoded;

}


kasaLight::kasaLight(std::string address){

    this->sock = socket(AF_INET, SOCK_STREAM, 0);
    //error
    sockaddr_in bulbAddress;
    bulbAddress.sin_family = AF_INET;
    bulbAddress.sin_port = htons(9999);
    bulbAddress.sin_addr.s_addr = inet_addr(address.c_str());

    int connection = connect(sock, (struct sockaddr*)&bulbAddress, sizeof(bulbAddress));
    std::cout << address << ": " << connection << "\n";
    this->transition = 1;
    //connect error

}

//turn light on
bool kasaLight::ON(){

    return this->customCommand("{\"smartlife.iot.smartbulb.lightingservice\":{\"transition_light_state\":{\"on_off\":1,\"ignore_default\":1,\"transition_period\":" + std::to_string(this->transition) + "}}}");

}

//turn light off
bool kasaLight::OFF(){

    return this->customCommand("{\"smartlife.iot.smartbulb.lightingservice\":{\"transition_light_state\":{\"on_off\":0,\"ignore_default\":1,\"transition_period\":" + std::to_string(this->transition) + "}}}");

}


//set bulb brightness
bool kasaLight::setBrightness(int brightness){

    if(brightness > 100 || brightness < 0){
        return false;
    }
    
    std::string command = "{\"smartlife.iot.smartbulb.lightingservice\":{\"transition_light_state\":{\"brightness\":" + std::to_string(brightness) + ",\"transition_period\":" + std::to_string(this->transition) + "}}}";
    int* len = new int;
    *len = command.length();
    
    unsigned char* resp = this->sendCommand(command, len);
    
    for(int i = 0; i < *len; i++){
      std::cout << (char)resp[i];
    }
    std::cout << "\n";
    
    if(*len < 1){
      return false;
    }
    
    std::string comp = "\"brightness\":";
    int pos = 0;
    for(int i = 0; i < *len; i++){
    
      if(resp[i] == comp.at(pos)){
        pos++;
      }
      else{
        pos = 0;
      }
      if(pos == comp.length()){
        std::string val = "";
        for(int j = i + 1; j < *len; j++){
          if(resp[j] > 47 && resp[j] < 58){
            val += resp[j];
          }
          else{
            if(val.length() > 0 && stoi(val) == brightness){
              return true;
            }
            else{
              return false;
            }
          }
        }
        return false;
      }
    }
    return false;
}


bool kasaLight::setColor(int hue){

    return this->setColor(hue, 100);

}


bool kasaLight::setColor(int hue, int saturation){

    if(hue < 0 || hue > 300 || saturation < 0 || saturation > 100){
        return false;
    }
    return this->customCommand("{\"smartlife.iot.smartbulb.lightingservice\":{\"transition_light_state\":{\"color_temp\":0,\"hue\":" + std::to_string(hue) + ",\"saturation\":" + std::to_string(saturation) + ",\"transition_period\":" + std::to_string(this->transition) + "}}}");

}


unsigned char* kasaLight::sendCommand(std::string command, int* outputLength){

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

    unsigned char recvBuffer[4];
    recv(this->sock, recvBuffer, 4, 0);
    unsigned char recvBuffer2[4];
    recvBuffer2[0] = recvBuffer[3];
    recvBuffer2[1] = recvBuffer[2];
    recvBuffer2[2] = recvBuffer[1];
    recvBuffer2[3] = recvBuffer[0];

    //length of incoming data
    uint32_t len = ((uint32_t)recvBuffer2[0] << 24) | ((uint32_t)recvBuffer2[1] << 16) | ((uint32_t)recvBuffer2[2] << 8) | ((uint32_t)recvBuffer2[3]);
    
    uint32_t bytesRead = 0;
    unsigned char* recvEncodedDataBuffer = new unsigned char[len];

    //read entire incoming message
    int initRead;
    while(bytesRead < len){
        initRead = recv(this->sock, recvEncodedDataBuffer + bytesRead, len - bytesRead, 0);
        if(initRead == -1){
          std::cout << "errorRead\n";
          *outputLength = -1; 
          return recvEncodedDataBuffer;
        }
        bytesRead += initRead;
    }
    
    recvEncodedDataBuffer = decode(recvEncodedDataBuffer, len);

    *outputLength = len;
    return recvEncodedDataBuffer;

}


bulbInfo kasaLight::customCommandReturnInfo(std::string command){

    int* length = new int;
    *length = 0;
    unsigned char* dData = sendCommand(command, length);


    if(*length == -1){
        bulbInfo errorB;
        errorB.errorCode = -1;
        return errorB;
    }
  
    bulbInfo b = rawInfoToStruct(dData, *length);

    return b;

}


bool kasaLight::customCommand(std::string command){

    int* length = new int;
    *length = 0;
    sendCommand(command, length);
    if(*length == -1){
        return false;
    }
    return true;

}

//set bulb whiteness
bool kasaLight::setWhite(int temp){

    if(temp > 6500 || temp < 2500){
        return false;
    }
    return this->customCommand("{\"smartlife.iot.smartbulb.lightingservice\":{\"transition_light_state\":{\"color_temp\":" + std::to_string(temp) + ",\"transition_period\":" + std::to_string(this->transition) + "}}}");

}


bool kasaLight::setWhite(){

    return this->setWhite(4500);

}

void kasaLight::setTransition(int transition){

    if(transition < 0){
        return;
    }
    this->transition = transition;

}


int kasaLight::getSocket(){

    return this->sock;

}


std::string kasaLight::getAlias(){

    bulbInfo b = getAllInfo();
    if(b.errorCode == -1){
        return "";
    }
    return b.alias;

}


int kasaLight::getTransition(){

    return this->transition;

}


bulbInfo kasaLight::getAllInfo(){

    return this->customCommandReturnInfo("{\"system\":{\"get_sysinfo\":{}}}");

}

bool kasaLight::getPower(){

    bulbInfo b = getAllInfo();
    if(b.errorCode == -1){
        return false;
    }
    return b.on;

}

//encrypt
std::string kasaLight::encryptCommand(std::string command){

    unsigned char* encoded = encode(command.c_str(), command.length());
    std::string* encodedString = new std::string(reinterpret_cast<char*>(encoded));
    return *encodedString;

}


bool kasaLight::blink(int times, int delay){

    bool on = this->getPower();

    for(int i = 0; i < times; i++){
        if(!this->ON()){
            return false;
        }
        std::this_thread::sleep_for(std::chrono::microseconds(delay));
        if(!this->OFF()){
            return false;
        }
        std::this_thread::sleep_for(std::chrono::microseconds(delay));
    }

    if(on){
        if(!this->ON()){
            return false;
        }
    }

    return true;

}


bool kasaLight::blink(int times){

    return this->blink(times, 1000);

}

bool kasaLight::blink(){

    return this->blink(3, 2000);

}

kasaLight::~kasaLight(){

    close(this->sock);

}

std::string kasaLight::getInfoString(){

    bulbInfo b = getAllInfo();
    if(b.errorCode == -1){
        return "error";
    }
    return b.alias + "\n" + std::to_string(b.on) + "\n" + b.mode + "\n" + std::to_string(b.brightness) + "\n" + std::to_string(b.hue) + "\n" + std::to_string(b.saturation) + "\n" + std::to_string(b.temp) + "\n" + std::to_string(b.errorCode);
}

