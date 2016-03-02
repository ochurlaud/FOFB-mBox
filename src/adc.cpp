#include "adc.h"

#include <chrono>
#include <thread>

#include "dma.h"
#include "logger/logger.h"
#include "rfmdriver.h"

ADC::ADC(RFMDriver *driver, DMA *dma)
    : m_driver(driver)
    , m_dma(dma)
    , m_node(ADC_NODE)
{}

ADC::~ADC()
{

}

int ADC::init()
{
    Logger::log("LOG") << "Init ADC" << Logger::flush;
    RFM2G_INT32 ctrlBuffer[128];
//    short Navr = DAC_freq/freq;
    ctrlBuffer[0] = 512; // RFM2G_LOOP_MAX
    ctrlBuffer[1] = 0;
    ctrlBuffer[2] = 0;
    ctrlBuffer[3] = 2;   // Navr

    std::this_thread::sleep_for(std::chrono::seconds(2));

    // Write ADC CTRL
    Logger::log("LOG") << "\tADC write sampling config" << Logger::flush;

    RFM2G_UINT32 threshold = 0;
    // see if DMA threshold and buffer are intialized
    m_driver->getDMAThreshold(&threshold);

    int data_size = 512;
    RFM2G_INT32 writeError;
    if (data_size < threshold) {
       // use PIO transfer
       writeError = m_driver->write(0, &ctrlBuffer, 512);
    } else {
       RFM2G_INT32 *dst = (RFM2G_INT32*)m_dma->memory();
       for (int i = 0 ; i < 128 ; i++) {
           dst[i] = ctrlBuffer[i];
       }
       writeError = m_driver->write(0, (void*)m_dma->memory(), data_size);
    }
    if (writeError) {
        Logger::error(_ME_) << "Can't write ADC config\n";
        return 1;
    }

    // Enable ADC
    Logger::log("LOG") << "\tADC enable sampling" << Logger::flush;
    RFM2G_STATUS sendEventError = m_driver->sendEvent(m_node, ADC_DAC_EVENT, ADC_ENABLE);
    if (sendEventError) {
        Logger::error(_ME_) << "Can't enable ADC\n";
        return 1;
    }

    // Start Sampling
    Logger::log("LOG") << "\tADC start sampling." << Logger::flush;
    sendEventError = m_driver->sendEvent(m_node, ADC_DAC_EVENT, ADC_START);
    if (sendEventError) {
        Logger::error(_ME_) << "Can't start sampling\n";
        return 1;
    }
    Logger::log("LOG") << "\tADC started" << Logger::flush;
    return 0;
}

int ADC::stop()
{
    Logger::log("LOG") << "ADC stoping sampling...." << Logger::flush;

    RFM2G_STATUS sendEventError = m_driver->sendEvent(m_node, ADC_DAC_EVENT, ADC_STOP);
    if (sendEventError) {
        Logger::error(_ME_) << "Can't stop ADC.\n";
        return 1;
    }
    Logger::log("LOG") << "\tADC stopped." << Logger::flush;

    return 0;
}


int ADC::read()
{
    RFM2GEVENTINFO eventInfo;              // Info about received interrupts
    eventInfo.Event = ADC_EVENT;           // We'll wait on this interrupt
    eventInfo.Timeout = ADC_TIMEOUT;       // We'll wait this many milliseconds

    // Wait on an interrupt from the other Reflective Memory board
    RFM2G_STATUS waitError = this->waitForEvent(eventInfo);
    if (waitError) {
        Logger::error(_ME_) << "Wait Error\n";
        return 1;
    }

    if ( m_dma->status() == NULL ) {
        Logger::error(_ME_) << "Null status\n";
        return 1;
    }

    m_dma->status()->loopPos = eventInfo.ExtendedInfo;
    RFM2G_NODE otherNodeId = eventInfo.NodeId;

    /* Now read data from the other board from BPM_MEMPOS */
    RFM2G_UINT32 threshold = 0;
    /* see if DMA threshold and buffer are intialized */
    m_driver->getDMAThreshold( &threshold );

    int data_size = ADC_BUFFER_SIZE  *sizeof( RFM2G_INT16 );

    if (data_size  < threshold) {
        // use PIO transfer
        RFM2G_STATUS readError = m_driver->read(ADC_MEMPOS + ( m_dma->status()->loopPos * data_size),
                                                (void*)m_buffer, data_size);
        if (readError) {
            Logger::error(_ME_) << "Read error\n";

            return 1;
        }

    } else {
        RFM2G_STATUS readError = m_driver->read(ADC_MEMPOS + ( m_dma->status()->loopPos * data_size),
                                                (void*) m_dma->memory(), data_size);
        if (readError) {
            Logger::error(_ME_) << "Read error DMA\n";

            return 1;
        }

        RFM2G_INT16 *src = (RFM2G_INT16*) m_dma->memory();
        for (int i = 0 ; i < ADC_BUFFER_SIZE ; ++i) {
            m_buffer[i] = src[i];
        }
    }

    // Send an interrupt to the IOC Reflective Memory board
    RFM2G_STATUS sendEventError = m_driver->sendEvent(otherNodeId, ADC_EVENT, 0);
    if (sendEventError) {
        Logger::error(_ME_) << "[ADC::read] Event not sent\n";

        return 1;
    }
    return 0;
}

RFM2G_STATUS ADC::waitForEvent(RFM2GEVENTINFO &eventInfo)
{
    RFM2G_STATUS eventError = m_driver->clearEvent(eventInfo.Event);
    if (eventError) {
        Logger::error(_ME_) << "in clearError, error: " << eventError << '\n';
        return eventError;
    }
    eventError = m_driver->enableEvent(eventInfo.Event);
    if (eventError) {
        Logger::error(_ME_) << "in enableEvent, error: " << eventError << '\n';
        return eventError;
    }

    RFM2G_STATUS waitForEventError = m_driver->waitForEvent(&eventInfo);
    return waitForEventError;
}
