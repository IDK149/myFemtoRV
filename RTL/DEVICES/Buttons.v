// femtorv32, a minimalistic RISC-V RV32I core
//       Bruno Levy, 2020-2021
//
// This file: driver for the buttons (does nearly nothing,
// could include some filtering here).

module Buttons #(
    parameter NBTN          = 7,
    parameter DEBOUNCE_BITS = 20
)(
    input wire             clk,
    input wire             sel,
    input wire             rstrb,
    output wire [31:0]     rdata,

    input wire [NBTN-1:0]  BUTTONS,
    output wire            irq
);

   wire [NBTN-1:0] pulse;
   reg  [NBTN-1:0] pending = 0;

   genvar i;
   generate
      for (i = 0; i < NBTN; i = i + 1) begin : btn
         reg [1:0]               sync    = 2'b00;
         reg [DEBOUNCE_BITS-1:0] cnt     = 0;
         reg                     clean   = 1'b0;
         reg                     clean_d = 1'b0;

         always @(posedge clk) begin
            sync <= {sync[0], ~BUTTONS[i]};   // ~ because of the pull-up
            if (sync[1] == clean)
               cnt <= 0;
            else begin
               cnt <= cnt + 1;
               if (&cnt) clean <= sync[1];
            end
            clean_d <= clean;
         end

         assign pulse[i] = clean & ~clean_d;
      end
   endgenerate

   wire clear = sel & rstrb;

   always @(posedge clk)
      pending <= (clear ? {NBTN{1'b0}} : pending) | pulse;

   assign irq   = |pulse;
   assign rdata = sel ? {{(32-NBTN){1'b0}}, pending} : 32'b0;

endmodule
