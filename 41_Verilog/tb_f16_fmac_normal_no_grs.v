`timescale 1ns/1ns
module tb_f16_fmac_normal_no_grs;

reg [15:0] x;
reg [15:0] y;
reg [15:0] z;

wire [15:0] result;
reg  [15:0] expected;

 f16_fmac_normal_no_grs u_fmac(
    .x(x),
    .y(y),
    .z(z),
    .result(result)
);

initial begin
    $dumpfile("f16_fmac.vcd");
    $dumpvars(1);
end

initial begin
    x = 16'h0000; y = 16'h0000; z = 16'h0000; expected = 16'h0000;
    
    #10 x = 16'h440D; y = 16'hB9CF; z = 16'hFFFE; expected = 16'hFFFE;    // 00   <------  PASS        <- Z is NaN!  <- Expected is NaN!
    #10 x = 16'h7FFE; y = 16'hFFFE; z = 16'hB8F7; expected = 16'h7FFE;    // 00   <------  FAIL FFFF   <- X is NaN!  <- Y is NaN!  <- Expected is NaN!
    #10 x = 16'h6804; y = 16'h443E; z = 16'h8081; expected = 16'h7042;    // 01   <------  PASS        <- Z is Underflow!
    #10 x = 16'hC3FC; y = 16'hFFFE; z = 16'hFFFE; expected = 16'hFFFE;    // 00   <------  FAIL 7FFF   <- Y is NaN!  <- Z is NaN!  <- Expected is NaN!
    #10 x = 16'h7FFE; y = 16'h0843; z = 16'h086C; expected = 16'h7FFE;    // 00   <------  FAIL 4C41   <- X is NaN!  <- Expected is NaN!
    
    #10 x = 16'h93E3; y = 16'hFFFE; z = 16'hCF84; expected = 16'hFFFE;    // 00   <------  FAIL 5600   <- Y is NaN!  <- Expected is NaN!
    #10 x = 16'h7FFE; y = 16'h00FC; z = 16'hFFFE; expected = 16'h7FFE;    // 00   <------  FAIL FFFE   <- Y is Underflow!  <- X is NaN!  <- Z is NaN!  <- Expected is NaN!
    #10 x = 16'h7D80; y = 16'h4774; z = 16'hBCD7; expected = 16'h7F80;    // 10   <------  FAIL 7FFF   <- X is NaN!  <- Expected is NaN!
    #10 x = 16'h7FFE; y = 16'hFFFE; z = 16'hFFFE; expected = 16'h7FFE;    // 00   <------  FAIL FFFF   <- X is NaN!  <- Y is NaN!  <- Z is NaN!  <- Expected is NaN!
    #10 x = 16'h2F40; y = 16'h0E80; z = 16'h0000; expected = 16'h02F2;    // 00   <------  FAIL 0000   <- Expected is Underflow!
    
    #10 x = 16'h8000; y = 16'h0000; z = 16'hAFBE; expected = 16'hAFBE;    // 00   <------  PASS      
    #10 x = 16'h93F6; y = 16'hC13D; z = 16'h42F6; expected = 16'h42F7;    // 01   <------  PASS      
    #10 x = 16'h53F8; y = 16'h0000; z = 16'h0000; expected = 16'h0000;    // 00   <------  PASS      
    #10 x = 16'h8000; y = 16'h41C6; z = 16'h6BFC; expected = 16'h6BFC;    // 00   <------  PASS      
    #10 x = 16'h9DFE; y = 16'h0000; z = 16'h2EDD; expected = 16'h2EDD;    // 00   <------  PASS      
    
    #10 x = 16'h8000; y = 16'h8B7A; z = 16'h0000; expected = 16'h0000;    // 00   <------  PASS      
    #10 x = 16'h8B1F; y = 16'h1040; z = 16'hC39F; expected = 16'hC39F;    // 01   <------  PASS      
    #10 x = 16'h8000; y = 16'h0000; z = 16'h0000; expected = 16'h0000;    // 00   <------  PASS      
    #10 x = 16'hD409; y = 16'hFCBE; z = 16'h0001; expected = 16'hFEBE;    // 10   <------  FAIL 7FFF   <- Z is Underflow!  <- Y is NaN!  <- Expected is NaN!
    #10 x = 16'h8000; y = 16'h0000; z = 16'hA35F; expected = 16'hA35F;    // 00   <------  PASS      
    
    #10 x = 16'h3077; y = 16'hE882; z = 16'hCC41; expected = 16'hDD4C;    // 01   <------  PASS      
    #10 x = 16'hDFFB; y = 16'h0000; z = 16'h0001; expected = 16'h0001;    // 00   <------  FAIL 0000   <- Z is Underflow!  <- Expected is Underflow!
    #10 x = 16'h8000; y = 16'hC007; z = 16'h6BF1; expected = 16'h6BF1;    // 00   <------  PASS      
    #10 x = 16'h0404; y = 16'h0000; z = 16'hAE10; expected = 16'hAE10;    // 00   <------  PASS      
    #10 x = 16'h8000; y = 16'h13C3; z = 16'h0001; expected = 16'h0001;    // 00   <------  FAIL 0000   <- Z is Underflow!  <- Expected is Underflow!
    
    #10 x = 16'hFA86; y = 16'hFAFB; z = 16'hC2FA; expected = 16'h7C00;    // 05   <------  FAIL 7FFF   <- Expected is +-Inf!
    #10 x = 16'h8000; y = 16'h0000; z = 16'h0001; expected = 16'h0001;    // 00   <------  FAIL 0000   <- Z is Underflow!  <- Expected is Underflow!
    #10 x = 16'h31FF; y = 16'h69EE; z = 16'h03FF; expected = 16'h6072;    // 01   <------  FAIL 6071   <- Z is Underflow!
    #10 x = 16'h8000; y = 16'h0000; z = 16'h7C05; expected = 16'h7E05;    // 10   <------  FAIL 7C05   <- Z is NaN!  <- Expected is NaN!
    #10 x = 16'h873F; y = 16'h3876; z = 16'h4A3F; expected = 16'h4A3F;    // 01   <------  FAIL 4A3E 
    
    #10 x = 16'hC3FF; y = 16'h0000; z = 16'h03FF; expected = 16'h03FF;    // 00   <------  FAIL 0000   <- Z is Underflow!  <- Expected is Underflow!
    #10 x = 16'h8000; y = 16'h4F7B; z = 16'hBBF7; expected = 16'hBBF7;    // 00   <------  PASS      
    #10 x = 16'hDFBB; y = 16'h0000; z = 16'h0081; expected = 16'h0081;    // 00   <------  FAIL 0000   <- Z is Underflow!  <- Expected is Underflow!
    #10 x = 16'h8000; y = 16'hE883; z = 16'h03FF; expected = 16'h03FF;    // 00   <------  FAIL 0000   <- Z is Underflow!  <- Expected is Underflow!
    #10 x = 16'h1382; y = 16'h305A; z = 16'hF7F8; expected = 16'hF7F8;    // 01   <------  PASS      
    
    #10 x = 16'h8000; y = 16'h0000; z = 16'h03FF; expected = 16'h03FF;    // 00   <------  FAIL 0000   <- Z is Underflow!  <- Expected is Underflow!
    #10 x = 16'h41CF; y = 16'h9091; z = 16'h03FE; expected = 16'h9662;    // 01   <------  FAIL 96A1   <- Z is Underflow!
    #10 x = 16'h8000; y = 16'h0000; z = 16'h1000; expected = 16'h1000;    // 00   <------  PASS      
    #10 x = 16'hF240; y = 16'hCBB8; z = 16'h4080; expected = 16'h7C00;    // 05   <------  FAIL 7FFF   <- Expected is +-Inf!
    #10 x = 16'h3328; y = 16'h0000; z = 16'h03FE; expected = 16'h03FE;    // 00   <------  FAIL 0000   <- Z is Underflow!  <- Expected is Underflow!
    
    #10 x = 16'h8000; y = 16'hB3E5; z = 16'hCBEE; expected = 16'hCBEE;    // 00   <------  PASS      
    #10 x = 16'h4CFB; y = 16'h0000; z = 16'h7E62; expected = 16'h7E62;    // 00   <------  PASS        <- Z is NaN!  <- Expected is NaN!
    #10 x = 16'h8000; y = 16'hFF7D; z = 16'h03FE; expected = 16'hFF7D;    // 00   <------  FAIL 0000   <- Z is Underflow!  <- Y is NaN!  <- Expected is NaN!
    #10 x = 16'h1BDF; y = 16'h5D7F; z = 16'hBD0F; expected = 16'h2D95;    // 01   <------  PASS      
    #10 x = 16'h8000; y = 16'h0000; z = 16'h03FE; expected = 16'h03FE;    // 00   <------  FAIL 0000   <- Z is Underflow!  <- Expected is Underflow!
    
    #10 x = 16'h4920; y = 16'h40FB; z = 16'h0400; expected = 16'h4E62;    // 01   <------  FAIL 4E61 
    #10 x = 16'h8000; y = 16'h0000; z = 16'hBD17; expected = 16'hBD17;    // 00   <------  PASS      
    #10 x = 16'hB6D4; y = 16'hDBCC; z = 16'h9C21; expected = 16'h56A8;    // 01   <------  FAIL 56A7 
    #10 x = 16'h98DA; y = 16'h0000; z = 16'h0400; expected = 16'h0400;    // 00   <------  PASS      
    #10 x = 16'h8000; y = 16'h3108; z = 16'h053F; expected = 16'h053F;    // 00   <------  PASS      
    
    #10 $finish;
end

endmodule
