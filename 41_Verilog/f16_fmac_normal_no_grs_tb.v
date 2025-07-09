`timescale 1ns/1ns
`include "f16_fmac_normal_no_grs.v"

module f16_fmac_normal_no_grs_tb;

    reg [15:0] x, x_d;
    reg [15:0] y, y_d;
    reg [15:0] z, z_d;
    wire [15:0] result;
    reg  [15:0] expected, expected_d;

    integer infile, outfile, i, i_d, r;
    integer pass_cnt, fail_cnt;

    f16_fmac_normal_no_grs u_fmac(
        .x(x),
        .y(y),
        .z(z),
        .result(result)
    );

    initial begin
        $dumpfile("f16_fmac_normal_no_grs_tb.vcd");
        $dumpvars(1);
    end

    initial begin
        // 입력 파일 열기 (읽기 모드)
        infile = $fopen("07_fmac_verilog_input_1.txt", "r");
        if (infile == 0) begin
            $display("입력 파일을 열 수 없습니다.");
            $finish;
        end

        // 출력 파일 열기 (쓰기 모드)
        outfile = $fopen("output.txt", "w");
        if (outfile == 0) begin
            $display("출력 파일을 열 수 없습니다.");
            $finish;
        end

        // 파일 끝까지 읽어서 출력 파일에 기록
        // while (!$feof(infile)) begin
        pass_cnt = 0;
        fail_cnt = 0;
        // for (i = 0; i < 10000 + 1; i = i + 1) begin
        for (i = 0; i < 3066634 + 1; i = i + 1) begin
            #10;
            r = $fscanf(infile, "%h %h %h %h\n", x, y, z, expected);
            if(i == 0) begin
                // continue
            end
            else if (r == 4) begin
                if (expected_d == result) begin
                    $fdisplay(outfile, "#%07d %h %h %h %h   <------  PASS",      i_d+1, x_d, y_d, z_d, expected_d);
                    pass_cnt = pass_cnt + 1;
                end
                else begin
                    $fdisplay(outfile, "#%07d %h %h %h %h   <------  FAIL %04X", i_d+1, x_d, y_d, z_d, expected_d, result);
                    fail_cnt = fail_cnt + 1;
                end
            end
            else begin
                 $display("Input Fail : %0d testcase!", i_d+1);
                $finish;
            end
            
            i_d = i;
            x_d = x;
            y_d = y;
            z_d = z;
            expected_d = expected;
        end

        $fdisplay(outfile, "Pass Rate : %.2f%% (%0d/%0d)", pass_cnt*100/(pass_cnt+fail_cnt), pass_cnt, (pass_cnt+fail_cnt));

        $fclose(infile);
        $fclose(outfile);
        $display("%07d testcase Done!", i_d);
        $finish;
    end
endmodule
