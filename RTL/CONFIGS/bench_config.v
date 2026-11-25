// Default femtosoc configuration file for IceStick

// `define NRV_NEGATIVE_RESET
/************************* Devices **********************************************************************************/

`define NRV_IO_UART      
`define NRV_IO_HUB75      
`define NRV_MAPPED_SPI_FLASH 

/************************* Processor configuration ******************************************************************/

`define NRV_FEMTORV32_QUARK // RV32I 

`define NRV_FREQ 20                 // Frequency in MHz. Recomm: 15 MHz   Overclocking: 20-25 MHz
`define NRV_RESET_ADDR 32'h00820000 // Jump execution to SPI Flash (800000h, +128k(20000h) for FPGA bitstream)

/************************* RAM (in bytes, needs to be a multiple of 4)***********************************************/

// Using the 128 kbytes of single-ported RAM of the ice40-up5k
// Note: cannot initialize it from .hex file, need to run from SPI Flash

`define ICE40UP5K_SPRAM
`define NRV_RAM 131072 

/************************* Advanced devices configuration *********************************************************/

`define NRV_RUN_FROM_SPI_FLASH 
`define NRV_IO_HARDWARE_CONFIG 
                               
/******************************************************************************************************************/

`define NRV_CONFIGURED

