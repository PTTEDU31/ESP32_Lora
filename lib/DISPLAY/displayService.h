#include "messageService.h"
#include "common.h"
class DisplayService : public MessageService
{
public:
    static DisplayService &getInstance()
    {
        static DisplayService instance;
        return instance;
    }
    void processReceivedMessage(messagePort port, DataMessage *message);
    
    DataMessage* getDataMessage(JsonObject data);

private:
    DisplayService() : MessageService(DisplayApp, "Display")
    {
        commandService = displayCommandService;
    };
    std::vector<String> displayTextVector;
}