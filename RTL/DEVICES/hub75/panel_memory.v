module panel_memory #(
    parameter COLOR_BITS = 4
) (
    input  wire        clk,
    input  wire [11:0] addr_top,
    input  wire [11:0] addr_bottom,

    output wire [(3*COLOR_BITS)-1:0] pix_top,
    output wire [(3*COLOR_BITS)-1:0] pix_bottom,

    input wire        wr_clk,
    input wire        wr_en,
    input wire [1:0]  wr_sel,
    input wire [11:0] wr_addr,
    input wire [(3*COLOR_BITS)-1:0] wr_data
);

  localparam W = 3*COLOR_BITS;
  // Magenta key: R = max, G = 0, B = max (0xF0F when COLOR_BITS = 4)
  localparam [W-1:0] KEY = {{COLOR_BITS{1'b1}}, {COLOR_BITS{1'b0}}, {COLOR_BITS{1'b1}}};

  reg [W-1:0] BACKGROUND_MEM [0:4095];
  reg [W-1:0] SPRITE_MEM     [0:4095];

  reg [W-1:0] background_top, background_bottom;
  reg [W-1:0] sprite_top, sprite_bottom;

  // integer i;
  // initial begin
  //   for (i = 0; i < 4096; i = i + 1) SPRITE_MEM[i] = KEY;  // sprite starts fully transparent
  //   $readmemh("RTL/DEVICES/hub75/background.hex", BACKGROUND_MEM);
  // end

  always @(posedge clk) begin
    if (wr_en) begin
      case (wr_sel)
        2'b00:   BACKGROUND_MEM[wr_addr] <= wr_data;
        2'b01:   SPRITE_MEM[wr_addr]     <= wr_data;
        default: ;
      endcase
    end
  end

  always @(posedge clk) begin
    background_top    <= BACKGROUND_MEM[addr_top];
    background_bottom <= BACKGROUND_MEM[addr_bottom];
    sprite_top        <= SPRITE_MEM[addr_top];
    sprite_bottom     <= SPRITE_MEM[addr_bottom];
  end

  wire [W-1:0] mix_top    = (sprite_top    != KEY) ? sprite_top    : background_top;
  wire [W-1:0] mix_bottom = (sprite_bottom != KEY) ? sprite_bottom : background_bottom;

  assign pix_top    = (mix_top    == KEY) ? {W{1'b0}} : mix_top;
  assign pix_bottom = (mix_bottom == KEY) ? {W{1'b0}} : mix_bottom;

endmodule
