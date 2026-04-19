#pragma once

#include <Arduino.h>
#include <SPI.h>
#include <mcp_can.h>

#define CAN_CS_PIN 5
#define CAN_INT_PIN 4

struct BaudConfig {
    uint8_t baud;
    const char* label;
};

class CanSniffer {
   public:
    CanSniffer();
    void begin();
    void handle();

   private:
    void setupInterface();
    bool initMCP();
    void recordId(uint32_t id);
    void printSummary();

    MCP_CAN mcp;
    static const BaudConfig BAUD_LIST[];
    uint32_t seenIds[64];
    uint32_t seenCounts[64];
    uint8_t seenCount;
    unsigned long lastSummary;
};
