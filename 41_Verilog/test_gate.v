module test_gate (
    input  a,
    input  b,
    output and_out,
    output or_out,
    output xor_out
);
    assign and_out = a & b;
    assign or_out  = a | b;
    assign xor_out = a ^ b;
endmodule