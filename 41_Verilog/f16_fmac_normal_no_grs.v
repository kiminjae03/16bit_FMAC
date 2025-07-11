`timescale 1ns/1ns

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
    wire [5:0] expo_z = z[14:10] + BIAS; // 6-bit(Exponent + Bias)

    wire [9:0] mant_x = x[9:0];
    wire [9:0] mant_y = y[9:0];
    wire [9:0] mant_z = z[9:0];

    // Significand(11-bit / 22-bit) : Hidden 1-bit + Mantissa
    wire [10:0] sigf_x = {1'b1, mant_x};
    wire [10:0] sigf_y = {1'b1, mant_y};
    wire [21:0] sigf_z = (expo_z == BIAS) ? 22'd0 : {1'b1, mant_z, 11'd0};

    // Zero = (? x 0) or (0 x ?) -> t_falg = true
    wire t_flag = (expo_x == 5'd0 || expo_y == 5'd0);

    // Multiply: significand_x * significand_y (22-bit)
    wire [21:0] mult = sigf_x * sigf_y;
    wire expo_add = mult[21];

    // Temp Sign
    wire sign_t = sign_x ^ sign_y;

    // Temp Exponent and Significand
    reg [5:0] expo_t;
    reg [21:0] aligned_t;
    always@(*) begin
        if(t_flag) begin
            expo_t = 6'd0;
            aligned_t = 22'd0;
        end else begin
            if(mult[21]) begin
                aligned_t = mult;
            end else begin
                // aligned_t = mult << 1;
                aligned_t = {mult[20:0], 1'b0};
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
    wire [5:0]  expo_l = great_value ? expo_t    : expo_z;
    wire [5:0]  expo_s = great_value ? expo_z    : expo_t;
    wire [21:0] sigf_l = great_value ? aligned_t : sigf_z;
    wire [21:0] sigf_s = great_value ? sigf_z    : aligned_t;

    // Exponent Difference(6-bit)
    wire [5:0] expo_diff = expo_l - expo_s;

    // Aligning
    wire [21:0] aligned_l = sigf_l;
    wire [21:0] aligned_s = (expo_diff > 6'd20) ? 22'd0 : sigf_s >> expo_diff;


    // 2's Complement and Add
    wire [22:0] aligned = sign_xor ? {1'b0, (aligned_l - aligned_s)} : (aligned_l + aligned_s);

    // aligned == 0 -> Zero
    wire flag_zero = (aligned == 23'd0);

    // Leading Zero Counter(23-bit)
    reg carry;
    reg [4:0] shift_exp;
    always@(*) begin
        if (aligned[22]) begin
            shift_exp = 5'd0;
            carry = 1'b1;
        end else begin
            // Use casez for a synthesis-friendly priority encoder
            casez(aligned[21:0])
                22'b1?????????????????????: begin shift_exp = 5'd0; carry = 1'b0; end
                22'b01????????????????????: begin shift_exp = 5'd1; carry = 1'b0; end
                22'b001???????????????????: begin shift_exp = 5'd2; carry = 1'b0; end
                22'b0001??????????????????: begin shift_exp = 5'd3; carry = 1'b0; end
                22'b00001?????????????????: begin shift_exp = 5'd4; carry = 1'b0; end
                22'b000001????????????????: begin shift_exp = 5'd5; carry = 1'b0; end
                22'b0000001???????????????: begin shift_exp = 5'd6; carry = 1'b0; end
                22'b00000001??????????????: begin shift_exp = 5'd7; carry = 1'b0; end
                22'b000000001?????????????: begin shift_exp = 5'd8; carry = 1'b0; end
                22'b0000000001????????????: begin shift_exp = 5'd9; carry = 1'b0; end
                22'b00000000001???????????: begin shift_exp = 5'd10; carry = 1'b0; end
                22'b000000000001??????????: begin shift_exp = 5'd11; carry = 1'b0; end
                22'b0000000000001?????????: begin shift_exp = 5'd12; carry = 1'b0; end
                22'b00000000000001????????: begin shift_exp = 5'd13; carry = 1'b0; end
                22'b000000000000001???????: begin shift_exp = 5'd14; carry = 1'b0; end
                22'b0000000000000001??????: begin shift_exp = 5'd15; carry = 1'b0; end
                22'b00000000000000001?????: begin shift_exp = 5'd16; carry = 1'b0; end
                22'b000000000000000001????: begin shift_exp = 5'd17; carry = 1'b0; end
                22'b0000000000000000001???: begin shift_exp = 5'd18; carry = 1'b0; end
                22'b00000000000000000001??: begin shift_exp = 5'd19; carry = 1'b0; end
                22'b000000000000000000001?: begin shift_exp = 5'd20; carry = 1'b0; end
                22'b0000000000000000000001: begin shift_exp = 5'd21; carry = 1'b0; end
                default:                    begin shift_exp = 5'd0;  carry = 1'b0; end // All zeros or carry case
            endcase
        end
    end

    // Normalizing (Exponent)
    wire signed [6:0] sub_res = {1'b0, expo_l} + carry - {1'b0, shift_exp} - {2'b0, BIAS};

    reg [5:0] updated_expo;
    always@(*) begin
        if (sub_res[6]) begin
            updated_expo = 6'd0;
        end else begin
            updated_expo = sub_res[5:0];
        end
    end

    // Normalizing (Mantissa)
    reg [1:0] flag;
    reg [21:0] normalized;
    always@(*) begin
        if (6'd31 < updated_expo) begin
            flag = OVERFLOW;
        end else if(6'd0 < updated_expo && updated_expo < 6'd32) begin
            flag = NORMAL;
            if (carry) normalized = aligned[22:1];
            else normalized = aligned << shift_exp;
        end else begin  // updated_expo < 6'd1
            flag = UNDERFLOW;
        end
    end

    // Exponent
    // wire [4:0] expo = updated_expo[4:0];

    // Mantissa
    // wire [9:0] mant = normalized[20:11];

    always@(*) begin
        if (flag_zero) result = {sign, 5'h00, 10'h000};
        else begin
            if(flag == NORMAL   ) result = {sign, updated_expo[4:0], normalized[20:11]};
            // if(flag == NORMAL   ) result = {sign, expo, mant};
            else if(flag == OVERFLOW ) result = {sign, 5'h1F, 10'h3FF};
            else if(flag == UNDERFLOW) result = {sign, 5'h00, 10'h000};
            else result = {sign, 5'h00, 10'h000};
        end
    end
        
endmodule
