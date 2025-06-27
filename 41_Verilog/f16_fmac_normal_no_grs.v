module f16_fmac_normal_no_grs (
    input      [15:0] x,
    input      [15:0] y,
    input      [15:0] z,
    output reg [15:0] result
);

// parameter == const
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
wire [5:0] expo_z = {1'b0, z[14:10]} + {1'b0, BIAS}; // 6-bit(Exponent + Bias)

wire [9:0] mant_x = x[9:0];
wire [9:0] mant_y = y[9:0];
wire [9:0] mant_z = z[9:0];

// Significand(11-bit / 22-bit) : Hidden 1-bit + Mantissa
wire [10:0] sigf_x = {1'b1, mant_x};
wire [10:0] sigf_y = {1'b1, mant_y};
wire [21:0] sigf_z = (expo_z == {1'b0, BIAS}) ? 22'd0 : {1'b1, mant_z, 11'd0};

// Zero = (? x 0) or (0 x ?) -> t_falg = true
wire t_flag = (expo_x == 5'd0 || expo_y == 5'd0);

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
wire [27:0] mag_t = {expo_t, aligned_t};
wire [27:0] mag_z = {expo_z, sigf_z};
wire great_value = (mag_t >= mag_z);

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
wire [22:0] aligned = sign_xor ? (aligned_l - aligned_s) : (aligned_l + aligned_s);

// aligned == 0 -> Zero
wire flag_zero = (aligned == 23'd0);

// Leading Zero Counter(23-bit)
wire carry = aligned[22];
reg[4:0] shift_exp;
always@(*) begin
    if      (aligned[21]) shift_exp = 5'd0;
    else if (aligned[20]) shift_exp = 5'd1;
    else if (aligned[19]) shift_exp = 5'd2;
    else if (aligned[18]) shift_exp = 5'd3;
    else if (aligned[17]) shift_exp = 5'd4;
    else if (aligned[16]) shift_exp = 5'd5;
    else if (aligned[15]) shift_exp = 5'd6;
    else if (aligned[14]) shift_exp = 5'd7;
    else if (aligned[13]) shift_exp = 5'd8;
    else if (aligned[12]) shift_exp = 5'd9;
    else if (aligned[11]) shift_exp = 5'd10;
    else if (aligned[10]) shift_exp = 5'd11;
    else if (aligned[9])  shift_exp = 5'd12;
    else if (aligned[8])  shift_exp = 5'd13;
    else if (aligned[7])  shift_exp = 5'd14;
    else if (aligned[6])  shift_exp = 5'd15;
    else if (aligned[5])  shift_exp = 5'd16;
    else if (aligned[4])  shift_exp = 5'd17;
    else if (aligned[3])  shift_exp = 5'd18;
    else if (aligned[2])  shift_exp = 5'd19;
    else if (aligned[1])  shift_exp = 5'd20;
    else if (aligned[0])  shift_exp = 5'd21;
    else                  shift_exp = 5'd0;
end

// Normalizing
reg [5:0] updated_expo;
reg [4:0] expo;
reg [1:0] flag;
reg [21:0] normalized;
always@(*) begin
    if ((expo_l - shift_exp + carry - BIAS) >= 0) begin
        updated_expo = expo_l - shift_exp + carry - BIAS;
    end else begin
        updated_expo = 6'd0;
    end
    normalized = 22'd0;
    if (updated_expo >= 6'd32) begin
        flag = OVERFLOW;
    end else begin
        expo = updated_expo[4:0];
        if (updated_expo == 6'd0) begin
            flag = UNDERFLOW;
        end else begin
            flag = NORMAL;
            if (carry) normalized = aligned >> 1;
            else normalized = aligned << shift_exp;    
        end
    end 
end

// Mantissa (normal)
wire [9:0] mant = normalized[20:11];

always@(*) begin
    if (flag_zero) result = {sign, 5'h00, 10'h000};
    else begin
        if(flag == NORMAL   ) result = {sign, expo, mant};
        if(flag == OVERFLOW ) result = {sign, 5'h1F, 10'h3FF};
        if(flag == UNDERFLOW) result = {sign, 5'h00, 10'h000};
    end
end
    
endmodule

