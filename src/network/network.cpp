#include "network.h"

void setupNetwork()
{
#ifndef NO_NET
    setupWiFi();
    setupMQTT();
#endif
}

void handleNetwork()
{
#ifndef NO_NET
    handleWiFi();
    handleNTP();
    handleDataSources();
    handleMQTT();
#endif
}