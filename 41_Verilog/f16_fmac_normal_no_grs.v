module f16_fmac_normal_no_grs (
    input  [15:0] x,
    input  [15:0] y,
    input  [15:0] z,
    output [15:0] result
);

// parameter == constant
parameter NORMAL    = 2'b00;
parameter OVERFLOW  = 2'b01;
parameter UNDERFLOW = 2'b10;
parameter BIAS      = 4'd15;

// Seperate (Sign(1-bit) / Expnent(5-bit) / Mantissa(10-bit))
wire sign_x = x[15];
wire sign_y = y[15];
wire sign_z = z[15];

wire [4:0] expo_x = x[14:10];
wire [4:0] expo_y = y[14:10];
wire [5:0] expo_z = z[14:10] + BIAS; // 6-bit(Exponent + Bias)

wire [9:0] mant_x = x[9:0];
wire [9:0] mant_y = y[9:0];
wire [9:0] mant_z = z[9:0];

// Significand(11-bit / 22-bit) : Hidden 1-bit + Mantissa
wire [10:0] sigf_x = {1'b1, mant_x};
wire [10:0] sigf_y = {1'b1, mant_y};
wire [21:0] sigf_z = (expo_z == BIAS) ? 22'd0 : {1'b1, mant_z, 11'd0};

// Zero = (? x 0) or (0 x ?) -> t_falg = true
wire t_flag = (expo_x == 5'd0 || expo_y == 5'd0) ? 1'b1 : 1'b0;

// Multiply: significand_x * significand_y (22-bit)
wire [21:0] mult = sigf_x * sigf_y;

// Temp Sign
wire sign_t = sign_x ^ sign_y;

// Temp Exponent and Significand
reg [5:0] expo_t;
reg expo_add;
reg [21:0] aligned_t;
always@(*) begin
    if(t_flag) begin
        expo_t = 6'd0;
        aligned_t = 22'd0;
    end else begin
        if(mult[21]) begin
            aligned_t = mult;
            expo_add = 1'b1;
        end else begin
            aligned_t = mult << 1;
            expo_add = 1'b0;
        end
        expo_t = expo_x + expo_y + expo_add;
    end
end

// Magnitude Compare : {Exp_t + aligned_t} <> {Exp_z + sigf_z}
wire great_value = ({expo_t, aligned_t} >= {expo_z, sigf_z}) ? 1'b1 : 1'b0;

// Sign
wire sign_xor = sign_t ^ sign_z;
wire sign = (sign_t == sign_z) ? sign_t : 
            (great_value) ? sign_t : sign_z;

// Large Magnitude: l, Small Magnitude: s
wire [5:0]  expo_l, expo_s;
wire [21:0] sigf_l, sigf_s;
assign expo_l = great_value ? expo_t : expo_z;
assign expo_s = great_value ? expo_z : expo_t;
assign sigf_l = great_value ? aligned_t : sigf_z;
assign sigf_s = great_value ? sigf_z : aligned_t;

// Exponent Difference(6-bit)
wire [5:0] expo_diff = expo_l - expo_s;

// Aligning
wire [21:0] aligned_l, aligned_s;
assign aligned_l = sigf_l;
assign aligned_s = (expo_diff >= 6'd21) ? 22'd0 : sigf_s >> expo_diff;

// 2's Complement and Add
wire [22:0] aligned = sign_xor ? (aligned_l + ~aligned_s + 1) : (aligned_l + aligned_s);

// aligned == 0 -> Zero
wire flag_zero = (aligned == 23'd0) ? 1 : 0;

// Leading Zero Counter(23-bit)
reg[4:0] shift_exp;
reg carry;
always@(*) begin
    shift_exp = 5'd0;
    carry = 1'b0;
    if(aligned[22]) carry = 1'b1;
    if(aligned[21]) shift_exp = 5'd0;
    if(aligned[20]) shift_exp = 5'd1;
    if(aligned[19]) shift_exp = 5'd2;
    if(aligned[19]) shift_exp = 5'd3;
    if(aligned[18]) shift_exp = 5'd4;
    if(aligned[17]) shift_exp = 5'd5;
    if(aligned[16]) shift_exp = 5'd6;
    if(aligned[15]) shift_exp = 5'd7;
    if(aligned[14]) shift_exp = 5'd8;
    if(aligned[13]) shift_exp = 5'd9;
    if(aligned[12]) shift_exp = 5'd10;
    if(aligned[11]) shift_exp = 5'd11;
    if(aligned[10]) shift_exp = 5'd12;
    if(aligned[9])  shift_exp = 5'd13;
    if(aligned[8])  shift_exp = 5'd14;
    if(aligned[7])  shift_exp = 5'd15;
    if(aligned[6])  shift_exp = 5'd16;
    if(aligned[5])  shift_exp = 5'd17;
    if(aligned[4])  shift_exp = 5'd18;
    if(aligned[3])  shift_exp = 5'd19;
    if(aligned[2])  shift_exp = 5'd20;
    if(aligned[1])  shift_exp = 5'd21;
    if(aligned[0])  shift_exp = 5'd22;
end

// Normalizing
wire [5:0] expo = ((expo_l - shift_exp + carry - BIAS) >= 0) ? (expo_l - shift_exp + carry - BIAS) : 0;
wire [1:0] flag = (expo >= 6'd32) ? OVERFLOW : 
                  (expo == 6'd0) ? UNDERFLOW : NORMAL;
wire [21:0] normalized = carry ? aligned >> 1 : aligned << shift_exp;

wire [9:0] mant = normalized[21:12];

assign result = (flag_zero) {sign, 5'd0, 10'd0} :
                (flag == NORMAL) {sign, expo, mant} :
                (flag == OVERFLOW) {sign, 5'h1F, 10'h3FF} : {sign, 5'd0, 10'd0};
    
endmodule
