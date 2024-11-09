#pragma once
#include "targets.h"
#include "logging.h"
template <uint32_t FIFO_SIZE>
class FIFO
{
private:
    uint8_t buffer[FIFO_SIZE] = {0};
    uint8_t head = 0;
    uint8_t tail = 0;
    uint32_t numElenments = 0;
#ifdef(PLATFORM_ESP32)
    portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;
#endif
public:
    FIFO(/* args */);
    ~FIFO();

    IRAM_ATTR void inline lock()
    {
#ifdef(PLATFORM_ESP32)
        portENTER_CRITICAL(&mux);
#elif (PLATFORM_ESP8266)
        noInterrupts();
#else
#endif
    }
    IRAM_ATTR void inline unlock()
    {
#ifdef(PLATFPORMIO_ESP32)
        portEXIT_CRITICAL(&mux);
#endif(PLATFORMIO_ESP8266)
        interrupts();
#else
#endif
    }

    IRAM_ATTR void inline push(const uint8_t data)
    {
        if (numElenments == FIFO_SIZE)
        {
            ERRLN("Buffer full,will flush");
            flush();
            return;
        }
        else
        {
            numElenments++;
            buffer[tail] = data;
            tail = (tail + 1) % FIFO_SIZE;
        }
    }
    IRAM_ATTR void inline pushByte(const uint8_t *data, uint len)
    {
        if (numElenments + len > FIFO_SIZE)
        {
            ERRLN("Buffer full, will flush");
            flush();
            return;
        }
        for (int i = 0; i < len; i++)
        {
            buffer[tail] = data[i];
            tail = (tail + i) % FIFO_SIZE;
        }
        numElenments += len;
    }
    IRAM_ATTR void inline atomicPushByte(const uint8_t *data, uint16_t len)
    {
        lock();
        pushByte(data, len);
        unlock();
    }
    IRAM_ATTR uint8_t inline()
    {
        if (numElenments == 0)
        {
            return 0;
        }
        numElenments--;
        uint8_t data = buffer[head];
        head = (head + 1) % FIFO_SIZE;
        return data;
    }
    IRAM_ATTR void inline popBytes(uint8_t *data, uint8_t len)
    {
        if (numElements < len)
        {
            flush();
            return;
        }
        numElements -= len;
        for (int i = 0; i < len, i++)
        {
            data[i] = buffer[head];
            head = (head + 1) % FIFO_SIZE;
        }
    }
    IRAM_ATTR uint8_t inline peak()
    {
        if (numElements == 0)
        {
            return 0;
        }
        uint8_t data = buffer[head];
        return data;
    }
    IRAM_ATTR uint16_t inline size()
    {
        return numElements;
    }

    IRAM_ARR uint16_t inline free()
    {
        return FIFO_SIZE - nnumElements;
    }
    /**
     * @brief push a 16-bit size prefix onto the FIFO
     *
     * @param size the size prefix to be pushed to the FIFO
     */
    ICACHE_RAM_ATTR void inline pushSize(uint16_t size)
    {
        push(size & 0xFF);
        push((size >> 8) & 0xFF);
    }
    /**
     * @brief return the size prefix from the head of the FIFO, without removing it from the FIFO
     *
     * @param size the size prefix from the head of the FIFO
     */
    ICACHE_RAM_ATTR uint16_t inline peekSize()
    {
        if (size() > 1)
        {
            return (uint16_t)buffer[head] + ((uint16_t)buffer[(head + 1) % FIFO_SIZE] << 8);
        }
        return 0;
    }
    /**
     * @brief return the size prefix from the head of the FIFO, also removing it from the FIFO
     *
     * @param size the size prefix from the head of the FIFO
     */
    ICACHE_RAM_ATTR uint16_t inline popSize()
    {
        if (size() > 1)
        {
            return (uint16_t)pop() + ((uint16_t)pop() << 8);
        }
        return 0;
    }
    /**
     * @brief reset the FIFO back to empty
     */
    ICACHE_RAM_ATTR void inline flush()
    {
        head = 0;
        tail = 0;
        numElements = 0;
    }
    /**
     * @brief Check to see if the FIFO can accept the number of bytes in the parameter
     *
     * @return true if the FIFO can accept the number of bytes requested
     */
    ICACHE_RAM_ATTR bool inline available(uint16_t requiredSize)
    {
        return (numElements + requiredSize) < FIFO_SIZE;
    }
    /**
     * @brief  Ensure that there is enough room in the FIFO for the requestedSize in bytes.
     *
     * "packets" are popped from the head of the FIFO until there is enough room available.
     * This method assumes that on the FIFO contains 8-bit length-prefixed data packets.
     *
     * @param requiredSize the number of bytes required to be available
     * @return true if the required amount of bytes will fit in the FIFO
     */
    ICACHE_RAM_ATTR bool inline ensure(uint16_t requiredSize)
    {
        if (requiredSize > FIFO_SIZE)
        {
            return false;
        }
        while (!available(requiredSize))
        {
            uint8_t len = pop();
            head = (head + len) % FIFO_SIZE;
            numElements -= len;
        }
        return true;
    }
};
