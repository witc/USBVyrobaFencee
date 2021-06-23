/*!
 * \file      radio.c
 *
 * \brief     Radio driver API definition
 *
 * \copyright Revised BSD License, see section \ref LICENSE.
 *
 * \code
 *                ______                              _
 *               / _____)             _              | |
 *              ( (____  _____ ____ _| |_ _____  ____| |__
 *               \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 *               _____) ) ____| | | || |_| ____( (___| | | |
 *              (______/|_____)_|_|_| \__)_____)\____)_| |_|
 *              (C)2013-2017 Semtech
 *
 * \endcode
 *
 * \author    Miguel Luis ( Semtech )
 *
 * \author    Gregory Cristian ( Semtech )
 */
#include <radio_general.h>
#include "main.h"
#include "radio.h"
#include "sx126x.h"


extern bool GlLocalSleepRfBoosterTx;
 /*!
 * FSK bandwidth definition
 */
typedef struct
{
    uint32_t bandwidth;
    uint8_t  RegValue;
}FskBandwidth_t;

/*!
 * Precomputed FSK bandwidth registers values
 */
const FskBandwidth_t FskBandwidths[] =
{
    { 4800  , 0x1F },
    { 5800  , 0x17 },
    { 7300  , 0x0F },
    { 9700  , 0x1E },
    { 11700 , 0x16 },
    { 14600 , 0x0E },
    { 19500 , 0x1D },
    { 23400 , 0x15 },
    { 29300 , 0x0D },
    { 39000 , 0x1C },
    { 46900 , 0x14 },
    { 58600 , 0x0C },
    { 78200 , 0x1B },
    { 93800 , 0x13 },
    { 117300, 0x0B },
    { 156200, 0x1A },
    { 187200, 0x12 },
    { 234300, 0x0A },
    { 312000, 0x19 },
    { 373600, 0x11 },
    { 467000, 0x09 },
    { 500000, 0x00 }, // Invalid Bandwidth
};

const RadioLoRaBandwidths_t Bandwidths[] = { LORA_BW_125, LORA_BW_250, LORA_BW_500 };

//                                          SF12    SF11    SF10    SF9    SF8    SF7
static double RadioLoRaSymbTime[3][6] = {{ 32.768, 16.384, 8.192, 4.096, 2.048, 1.024 },  // 125 KHz
                                         { 16.384, 8.192,  4.096, 2.048, 1.024, 0.512 },  // 250 KHz
                                         { 8.192,  4.096,  2.048, 1.024, 0.512, 0.256 }}; // 500 KHz

uint8_t MaxPayloadLength = 0xFF;

uint32_t TxTimeout = 0;
uint32_t RxTimeout = 0;

bool RxContinuous = false;


PacketStatus_t RadioPktStatus;
uint8_t RadioRxPayload[255];

/*
 * SX126x DIO IRQ callback functions prototype
 */

/*!
 * \brief DIO 0 IRQ callback
 */
void RadioOnDioIrq( void );

/*!
 * \brief Tx timeout timer callback
 */
void RadioOnTxTimeoutIrq( void );

/*!
 * \brief Rx timeout timer callback
 */
void RadioOnRxTimeoutIrq( void );

/*
 * Private global variables
 */


/*!
 * Holds the current network type for the radio
 */
typedef struct
{
    bool Previous;
    bool Current;
}RadioPublicNetwork_t;

static RadioPublicNetwork_t RadioPublicNetwork = { false };


/*
 * Public global variables
 */

/*!
 * Radio hardware and global parameters
 */
SX126x_t SX126x;


void RadioInit(void )
{
	SX126xInit();
    SX126xSetStandby( STDBY_RC  );
    SX126xSetRegulatorMode( USE_DCDC  );

    SX126xSetBufferBaseAddress( 0x00, 0x00  );
    SX126xSetTxParams( 0, RADIO_RAMP_200_US  );
    SX126xSetDioIrqParams( IRQ_RADIO_ALL, IRQ_RADIO_ALL, IRQ_RADIO_NONE, IRQ_RADIO_NONE  );

}

void RadioDeinit( )
{
	SX126xSetStandby( STDBY_RC  );
	(void)SX126xGetIrqStatus();
	SX126xClearIrqStatus(IRQ_RADIO_ALL);
	RadioSleep();
}


RadioState_t RadioGetStatus( )
{
    switch(SX126xGetOperatingMode())
    {
        case MODE_TX:
            return RF_TX_RUNNING;
        case MODE_RX:
            return RF_RX_RUNNING;
        case MODE_CAD:
            return RF_CAD;
        default:
            return RF_IDLE;
    }
}


void RadioSetModem( RadioModems_t modem  )
{
    switch( modem )
    {
    default:
    case MODEM_FSK:
        SX126xSetPacketType( PACKET_TYPE_GFSK  );
        // When switching to GFSK mode the LoRa SyncWord register value is reset
        // Thus, we also reset the RadioPublicNetwork variable
        RadioPublicNetwork.Current = false;
        break;
    case MODEM_LORA:
        SX126xSetPacketType( PACKET_TYPE_LORA );
        // Public/Private network register is reset when switching modems
        if( RadioPublicNetwork.Current != RadioPublicNetwork.Previous )
        {
            RadioPublicNetwork.Current = RadioPublicNetwork.Previous;
            RadioSetPublicNetwork( RadioPublicNetwork.Current  );
        }
        break;
    }
}

void RadioSetChannel( uint32_t freq  )
{
    SX126xSetRfFrequency( freq  );
}


void RadioSetRxConfig( uint32_t frequency, RadioModems_t modem, uint32_t bandwidth,
                         uint32_t datarate, uint8_t coderate,
                         uint32_t bandwidthAfc, uint16_t preambleLen,
                         uint16_t symbTimeout, bool fixLen,
                         uint8_t payloadLen,
                         bool crcOn, bool freqHopOn, uint8_t hopPeriod,
                         bool iqInverted, bool rxContinuous  )
{

	RadioSetChannel(frequency );
	bandwidth-=7;
    RxContinuous = rxContinuous;

    if( fixLen == true )
    {
        MaxPayloadLength = payloadLen;
    }
    else
    {
        MaxPayloadLength = 0xFF;
    }

    switch( modem )
    {
        case MODEM_FSK:

            break;

        case MODEM_LORA:
            SX126xSetStopRxTimerOnPreambleDetect( false  );
            SX126xSetLoRaSymbNumTimeout( symbTimeout );
            SX126x.ModulationParams.PacketType = PACKET_TYPE_LORA;
            SX126x.ModulationParams.Params.LoRa.SpreadingFactor = ( RadioLoRaSpreadingFactors_t )datarate;
            SX126x.ModulationParams.Params.LoRa.Bandwidth = Bandwidths[bandwidth];
            SX126x.ModulationParams.Params.LoRa.CodingRate = ( RadioLoRaCodingRates_t )coderate;

            if( ( ( bandwidth == 0 ) && ( ( datarate == 11 ) || ( datarate == 12 ) ) ) ||
            ( ( bandwidth == 1 ) && ( datarate == 12 ) ) )
            {
                SX126x.ModulationParams.Params.LoRa.LowDatarateOptimize = 0x01;
            }
            else
            {
                SX126x.ModulationParams.Params.LoRa.LowDatarateOptimize = 0x00;
            }

            SX126x.PacketParams.PacketType = PACKET_TYPE_LORA;

            if( ( SX126x.ModulationParams.Params.LoRa.SpreadingFactor == LORA_SF5 ) ||
                ( SX126x.ModulationParams.Params.LoRa.SpreadingFactor == LORA_SF6 ) )
            {
                if( preambleLen < 12 )
                {
                    SX126x.PacketParams.Params.LoRa.PreambleLength = 12;
                }
                else
                {
                    SX126x.PacketParams.Params.LoRa.PreambleLength = preambleLen;
                }
            }
            else
            {
                SX126x.PacketParams.Params.LoRa.PreambleLength = preambleLen;
            }

            SX126x.PacketParams.Params.LoRa.HeaderType = ( RadioLoRaPacketLengthsMode_t )fixLen;

            SX126x.PacketParams.Params.LoRa.PayloadLength = MaxPayloadLength;
            SX126x.PacketParams.Params.LoRa.CrcMode = ( RadioLoRaCrcModes_t )crcOn;
            SX126x.PacketParams.Params.LoRa.InvertIQ = ( RadioLoRaIQModes_t )iqInverted;

            RadioSetModem( ( SX126x.ModulationParams.PacketType == PACKET_TYPE_GFSK ) ? MODEM_FSK : MODEM_LORA  );
            SX126xSetModulationParams( &SX126x.ModulationParams  );
            SX126xSetPacketParams( &SX126x.PacketParams );

            // Timeout Max, Timeout handled directly in SetRx function
            RxTimeout = 0xFFFF;

            break;
    }
}

void RadioSetTxConfig(uint32_t frequency, RadioModems_t modem, int8_t power, uint32_t fdev,
                        uint32_t bandwidth, uint32_t datarate,
                        uint8_t coderate, uint16_t preambleLen,
                        bool fixLen, bool crcOn, bool freqHopOn,
                        uint8_t hopPeriod, bool iqInverted, uint32_t timeout  )
{

	RadioSetChannel(frequency);
	bandwidth-=7;

    switch( modem )
    {
        case MODEM_FSK:

            break;

        case MODEM_LORA:
            SX126x.ModulationParams.PacketType = PACKET_TYPE_LORA;
            SX126x.ModulationParams.Params.LoRa.SpreadingFactor = ( RadioLoRaSpreadingFactors_t ) datarate;
            SX126x.ModulationParams.Params.LoRa.Bandwidth =  Bandwidths[bandwidth];
            SX126x.ModulationParams.Params.LoRa.CodingRate= ( RadioLoRaCodingRates_t )coderate;

            if( ( ( bandwidth == 0 ) && ( ( datarate == 11 ) || ( datarate == 12 ) ) ) ||
            ( ( bandwidth == 1 ) && ( datarate == 12 ) ) )
            {
                SX126x.ModulationParams.Params.LoRa.LowDatarateOptimize = 0x01;
            }
            else
            {
                SX126x.ModulationParams.Params.LoRa.LowDatarateOptimize = 0x00;
            }

            SX126x.PacketParams.PacketType = PACKET_TYPE_LORA;

            if( ( SX126x.ModulationParams.Params.LoRa.SpreadingFactor == LORA_SF5 ) ||
                ( SX126x.ModulationParams.Params.LoRa.SpreadingFactor == LORA_SF6 ) )
            {
                if( preambleLen < 12 )
                {
                    SX126x.PacketParams.Params.LoRa.PreambleLength = 12;
                }
                else
                {
                    SX126x.PacketParams.Params.LoRa.PreambleLength = preambleLen;
                }
            }
            else
            {
                SX126x.PacketParams.Params.LoRa.PreambleLength = preambleLen;
            }

            SX126x.PacketParams.Params.LoRa.HeaderType = ( RadioLoRaPacketLengthsMode_t )fixLen;
            SX126x.PacketParams.Params.LoRa.PayloadLength = MaxPayloadLength;
            SX126x.PacketParams.Params.LoRa.CrcMode = ( RadioLoRaCrcModes_t )crcOn;
            SX126x.PacketParams.Params.LoRa.InvertIQ = ( RadioLoRaIQModes_t )iqInverted;

            RadioStandby();
            RadioSetModem( ( SX126x.ModulationParams.PacketType == PACKET_TYPE_GFSK ) ? MODEM_FSK : MODEM_LORA  );
            SX126xSetModulationParams( &SX126x.ModulationParams  );
            SX126xSetPacketParams( &SX126x.PacketParams  );
            break;
    }
    SX126xSetRfTxPower( power  );
    TxTimeout = timeout;
}

bool RadioCheckRfFrequency( uint32_t frequency  )
{
    return true;
}

uint32_t RadioTimeOnAir( RadioModems_t modem, uint8_t pktLen  )
{
    uint32_t airTime = 0;

    switch( modem )
    {
    case MODEM_FSK:
        {
           airTime = rint( ( 8 * ( SX126x.PacketParams.Params.Gfsk.PreambleLength +
                                     ( SX126x.PacketParams.Params.Gfsk.SyncWordLength >> 3 ) +
                                     ( ( SX126x.PacketParams.Params.Gfsk.HeaderType == RADIO_PACKET_FIXED_LENGTH ) ? 0.0 : 1.0 ) +
                                     pktLen +
                                     ( ( SX126x.PacketParams.Params.Gfsk.CrcLength == RADIO_CRC_2_BYTES ) ? 2.0 : 0 ) ) /
                                     SX126x.ModulationParams.Params.Gfsk.BitRate ) * 1e3 );
        }
        break;
    case MODEM_LORA:
        {
            double ts = RadioLoRaSymbTime[SX126x.ModulationParams.Params.LoRa.Bandwidth - 4][12 - SX126x.ModulationParams.Params.LoRa.SpreadingFactor];
            // time of preamble
            double tPreamble = ( SX126x.PacketParams.Params.LoRa.PreambleLength + 4.25 ) * ts;
            // Symbol length of payload and time
            double tmp = ceil( ( 8 * pktLen - 4 * SX126x.ModulationParams.Params.LoRa.SpreadingFactor +
                                 28 + 16 * SX126x.PacketParams.Params.LoRa.CrcMode -
                                 ( ( SX126x.PacketParams.Params.LoRa.HeaderType == LORA_PACKET_FIXED_LENGTH ) ? 20 : 0 ) ) /
                                 ( double )( 4 * ( SX126x.ModulationParams.Params.LoRa.SpreadingFactor -
                                 ( ( SX126x.ModulationParams.Params.LoRa.LowDatarateOptimize > 0 ) ? 2 : 0 ) ) ) ) *
                                 ( ( SX126x.ModulationParams.Params.LoRa.CodingRate % 4 ) + 4 );
            double nPayload = 8 + ( ( tmp > 0 ) ? tmp : 0 );
            double tPayload = nPayload * ts;
            // Time on air
            double tOnAir = tPreamble + tPayload;
            // return milli seconds
            airTime = floor( tOnAir + 0.999 );
        }
        break;
    }
    return airTime;
}

void RadioSend (uint8_t *buffer, uint8_t size )
{
    SX126xSetDioIrqParams (IRQ_TX_DONE | IRQ_RX_TX_TIMEOUT,
			 IRQ_TX_DONE | IRQ_RX_TX_TIMEOUT, IRQ_RADIO_NONE,
			 IRQ_RADIO_NONE );

  if (SX126xGetPacketType () == PACKET_TYPE_LORA)
    {
      SX126x.PacketParams.Params.LoRa.PayloadLength = size;
    }
  else
    {
      SX126x.PacketParams.Params.Gfsk.PayloadLength = size;
    }
  SX126xSetPacketParams (&SX126x.PacketParams );

  SX126xSendPayload (buffer, size, 0 );

}

void RadioSleep(void)
{
    SleepParams_t params = { 0 };

    params.Fields.WarmStart = 1;
    SX126xSetSleep( params  );

    osDelay( 2 );

}

void RadioStandby(void )
{
	spiDevice2.RadioRFSwitch(SWITCH_RX);

    SX126xSetStandby( STDBY_RC  );
}

void RadioRx( uint32_t timeout  )	//timeout je v ms - posunuti o 6 krat doleva je prepocet na ms..
{
	spiDevice2.RadioRFSwitch(SWITCH_RX);

    SX126xSetDioIrqParams( IRQ_RX_DONE|IRQ_CRC_ERROR |IRQ_RX_TX_TIMEOUT, // "1" - ulozi se do registru
    					   IRQ_RX_DONE|IRQ_RX_TX_TIMEOUT, // "1" - zpusobi preruseni
                           IRQ_RADIO_NONE,
                           IRQ_RADIO_NONE  );

    if( RxContinuous == true )
    {
        SX126xSetRx( 0xFFFFFF  ); // Rx Continuous
    }
    else
    {
        SX126xSetRx( timeout<< 6 ); //
    }
}

void RadioRxBoosted(  uint32_t timeout   ) //timeout je v ms- posunuti o 6 krat doleva je prepocet na ms..
{
	spiDevice2.RadioRFSwitch(SWITCH_RX);

	SX126xSetDioIrqParams( IRQ_RX_DONE|IRQ_CRC_ERROR |IRQ_RX_TX_TIMEOUT, // "1" - ulozi se do registru
	    					   IRQ_RX_DONE|IRQ_RX_TX_TIMEOUT, // "1" - zpusobi preruseni
	                           IRQ_RADIO_NONE,
	                           IRQ_RADIO_NONE  );

    if( RxContinuous == true )
    {
        SX126xSetRxBoosted( 0xFFFFFF  ); // Rx Continuous
    }
    else
    {
        SX126xSetRxBoosted( timeout << 6  );
    }
}

void RadioSetRxDutyCycle( uint32_t rxTime, uint32_t sleepTime  )
{
    SX126xSetRxDutyCycle( rxTime, sleepTime  );
}

void RadioStartCad(   )
{
	spiDevice2.RadioRFSwitch(SWITCH_RX);
    SX126xSetCad();
}

void RadioTx( uint32_t timeout  )
{
	spiDevice2.RadioRFSwitch(SWITCH_TX);
    SX126xSetTx( timeout << 6  );
}

void RadioSetTxContinuousWave( uint32_t freq, int8_t power, uint16_t time  )
{
    SX126xSetRfFrequency( freq  );
    SX126xSetRfTxPower( power  );
    SX126xSetTxContinuousWave();
}

int16_t RadioRssi( RadioModems_t modem  )
{
    return SX126xGetRssiInst();
}

void RadioWrite( uint16_t addr, uint8_t data  )
{
    RG_SX126xWriteRegister( addr, data );
}

uint8_t RadioRead( uint16_t addr  )
{
    return RG_SX126xReadRegister( addr  );
}

void RadioWriteBuffer( uint16_t addr, uint8_t *buffer, uint8_t size  )
{
    RG_SX126xWriteRegisters( addr, buffer, size  );
}

void RadioReadBuffer( uint16_t addr, uint8_t *buffer, uint8_t size  )
{
    RG_SX126xReadRegisters( addr, buffer, size  );
}

void RadioWriteFifo( uint8_t *buffer, uint8_t size  )
{
    RG_SX126xWriteBuffer( 0, buffer, size  );
}

void RadioReadFifo( uint8_t *buffer, uint8_t size  )
{
    RG_SX126xReadBuffer( 0, buffer, size  );
}

void RadioSetPublicNetwork( bool enable  )
{
    RadioPublicNetwork.Current = RadioPublicNetwork.Previous = enable;

    RadioSetModem( MODEM_LORA );
    if( enable == true )
    {
        // Change LoRa modem SyncWord
        RG_SX126xWriteRegister( REG_LR_SYNCWORD, ( LORA_MAC_PUBLIC_SYNCWORD >> 8 ) & 0xFF  );
        RG_SX126xWriteRegister( REG_LR_SYNCWORD + 1, LORA_MAC_PUBLIC_SYNCWORD & 0xFF  );
    }
    else
    {
        // Change LoRa modem SyncWord
        RG_SX126xWriteRegister( REG_LR_SYNCWORD, ( LORA_MAC_PRIVATE_SYNCWORD >> 8 ) & 0xFF  );
        RG_SX126xWriteRegister( REG_LR_SYNCWORD + 1, LORA_MAC_PRIVATE_SYNCWORD & 0xFF  );
    }
}

void RadioSetMaxPayloadLength( RadioModems_t modem, uint8_t max  )
{
    if( modem == MODEM_LORA )
    {
        SX126x.PacketParams.Params.LoRa.PayloadLength = MaxPayloadLength = max;
        SX126xSetPacketParams( &SX126x.PacketParams );
    }
    else
    {
        if( SX126x.PacketParams.Params.Gfsk.HeaderType == RADIO_PACKET_VARIABLE_LENGTH )
        {
            SX126x.PacketParams.Params.Gfsk.PayloadLength = MaxPayloadLength = max;
            SX126xSetPacketParams( &SX126x.PacketParams );
        }
    }
}

uint32_t RadioGetWakeupTime(   )
{
    return( RADIO_TCXO_SETUP_TIME + RADIO_WAKEUP_TIME );
}

