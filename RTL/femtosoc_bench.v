
/*
 * Testbench for femtosoc / femtorv32
 *
 * Supports:
 *   - Verilator
 *   - Icarus Verilog
 *
 * The external ports follow the femtosoc configuration.
 */

`timescale 1ns/1ns

`include "femtosoc.v"


`ifdef VERILATOR

module femtoRV32_bench(

    input pclk,

`ifdef NRV_IO_HUB75
    output r0, g0, b0,
    output r1, g1, b1,
    output clk_out,
    output latch,
    output oe,
    output [4:0] addr,
`endif

`ifdef NRV_IO_BUTTONS
    input [3:0] BUTTONS,
`endif

`ifdef NRV_IO_UART
    output TXD,
    input RXD,
`endif

`ifdef NRV_SPI_FLASH
    inout spi_mosi,
    inout spi_miso,
    output spi_cs_n,
    output spi_clk,
`endif

    input RESET
);

`else

module femtoRV32_bench();

    reg pclk;

`ifdef NRV_IO_HUB75
    wire r0, g0, b0;
    wire r1, g1, b1;
    wire clk_out, latch, oe;
    wire [4:0] addr;
`endif

`ifdef NRV_IO_BUTTONS
    wire [3:0] BUTTONS;
`endif

`ifdef NRV_IO_UART
    wire TXD;
    wire RX;
    initial RXD = 1'b1;
`endif

`ifdef NRV_SPI_FLASH
    wire spi_mosi, spi_miso;
    wire spi_cs_n;
 `ifndef ULX3S
    wire spi_clk;
 `endif
`endif

    wire RESET = 1'b0;

`endif


/********************* Clock generation ************************/

`ifndef VERILATOR

initial begin
    pclk = 1'b0;

    forever begin
        #1 pclk = ~pclk;
    end
end

`endif


/********************* SoC instance ****************************/

femtosoc uut (

    .pclk(pclk),
    .RESET(RESET),

`ifdef NRV_IO_BUTTONS
  .BUTTONS(BUTTONS),
`endif

`ifdef NRV_IO_UART
    .TXD(TXD),
    .RXD(RXD),
`endif

`ifdef NRV_IO_LEDS
    .D1(D1),
    .D2(D2),
    .D3(D3),
    .D4(D4),
    .D5(D5),
`endif

`ifdef NRV_IO_HUB75
    .r0(r0),
    .g0(g0),
    .b0(b0),

    .r1(r1),
    .g1(g1),
    .b1(b1),

    .addr(addr),

    .clk_out(clk_out),
    .latch(latch),
    .oe(oe),
`endif

`ifdef NRV_SPI_FLASH
    .spi_mosi(spi_mosi),
    .spi_miso(spi_miso),
    .spi_cs_n(spi_cs_n),
 `ifndef ULX3S
    .spi_clk(spi_clk)
 `endif
`endif

);

endmodule

