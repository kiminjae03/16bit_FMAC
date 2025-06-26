`timescale 1ns/1ns
module tb_test_gate;

reg a;
reg b;

wire and_out;
wire or_out;
wire xor_out;

test_gate u_test_gate(
    .a(a),
    .b(b),
    .and_out(and_out),
    .or_out(or_out),
    .xor_out(xor_out)
);

initial begin
    $dumpfile("test_gate.vcd");
    $dumpvars(1);
end

initial begin
    a = 1'b0; b = 1'b0;
    #10 a = 1'b0; b = 1'b1;
    #10 a = 1'b1; b = 1'b0;
    #10 a = 1'b1; b = 1'b1;
    #10 $finish;
end

endmodule