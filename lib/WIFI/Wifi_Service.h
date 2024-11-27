#include "dataMessage.h"

class Wifi_Service {
public :
    static Wifi_Service& getInstance(){
        static Wifi_Service instance;
        return instance;
    }
    bool isConnected();
    void sendMessage(DataMessage* message);
    String getIP();

    String getSSID();

    String getPassword();

private :
    
    bool connected = false;

    bool initialized = false;
};