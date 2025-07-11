`timescale 1ns/1ns
`include "f16_fmac_normal_no_grs.v"

module f16_fmac_normal_no_grs_tb;
    // Inputs to the DUT
    reg [15:0] x;
    reg [15:0] y;
    reg [15:0] z;

    // Output from the DUT
    wire [15:0] result;

    // For verification
    reg [15:0] expected;

    // File handles and counters
    integer infile, outfile, r, i;
    integer pass_cnt, fail_cnt;

    // Define the number of test cases to run
    // Set to a large number to read the whole file, or a specific number.
    parameter NUM_TEST_CASES = 3066634;
    // parameter NUM_TEST_CASES = 100;
    parameter INPUT_FILE = "07_fmac_verilog_input_2.txt";
    parameter OUTPUT_FILE = "07_fmac_normal_no_grs_verilog_output_2.txt";

    // Instantiate the Design Under Test (DUT)
    f16_fmac_normal_no_grs u_fmac (
        .x(x),
        .y(y),
        .z(z),
        .result(result)
    );

    // Setup waveform dumping for debugging
    initial begin
        $dumpfile("f16_fmac_normal_no_grs_tb.vcd");
        $dumpvars(1, f16_fmac_normal_no_grs_tb);
    end

    // Main simulation block
    initial begin
        // Open input file for reading
        infile = $fopen(INPUT_FILE, "r");
        if (infile == 0) begin
            $display("ERROR: Cannot open input file '%s'", INPUT_FILE);
            $finish;
        end

        // Open output file for writing
        outfile = $fopen(OUTPUT_FILE, "w");
        if (outfile == 0) begin
            $display("ERROR: Cannot open output file '%s'", OUTPUT_FILE);
            $finish;
        end

        pass_cnt = 0;
        fail_cnt = 0;

        // Loop for the specified number of test cases
        for (i = 0; i < NUM_TEST_CASES; i = i + 1) begin: test_loop
            // Stop if we reach the end of the input file
            if ($feof(infile)) begin
                $display("Info: End of file reached after %7d tests.", i);
                disable test_loop;
            end

            // Read one line of test data into DUT inputs
            r = $fscanf(infile, "%h %h %h %h\n", x, y, z, expected);

            // Wait for a moment to let combinational logic settle.
            // #1 is enough for the simulator to schedule the update.
            #1; 

            // Now, compare the result with the expected value from this cycle
            if (result === expected) begin
                // $fdisplay(outfile, "#%07d: Inputs (%h, %h, %h) -> Expected: %h, Got: %h | PASS", i + 1, x, y, z, expected, result);
                $fdisplay(outfile, "#%07d %h %h %h %h   <------  PASS", i + 1, x, y, z, expected);
                pass_cnt = pass_cnt + 1;
            end else begin
                // $fdisplay(outfile, "#%07d: Inputs (%h, %h, %h) -> Expected: %h, Got: %h | FAIL", i + 1, x, y, z, expected, result);
                $fdisplay(outfile, "#%07d %h %h %h %h   <------  FAIL %h   <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<", i + 1, x, y, z, expected, result);
                fail_cnt = fail_cnt + 1;
            end
        end

        // Print summary to the output file
        $fdisplay(outfile, "\n============================================");
        $fdisplay(outfile,   "==  TEST SUMMARY                          ==");
        $fdisplay(outfile,   "============================================");
        $fdisplay(outfile, "Total Test Cases : %7d", pass_cnt + fail_cnt);
        $fdisplay(outfile, "PASS : %7d", pass_cnt);
        $fdisplay(outfile, "FAIL : %7d", fail_cnt);
        if ((pass_cnt + fail_cnt) > 0) begin
            $fdisplay(outfile, "Pass Rate : %.2f%%", (pass_cnt * 100.0) / (pass_cnt + fail_cnt));
        end
        $fdisplay(outfile, "============================================");

        // Also print summary to the console
        $display("\n============================================");
        $display(  "==  TEST SUMMARY                          ==");
        $display(  "============================================");
        $display("Total Test Cases : %7d", pass_cnt + fail_cnt);
        $display("PASS : %7d", pass_cnt);
        $display("FAIL : %7d", fail_cnt);
        if ((pass_cnt + fail_cnt) > 0) begin
            $display("Pass Rate : %.2f%%", (pass_cnt * 100.0) / (pass_cnt + fail_cnt));
        end
        $display("============================================");

        // Clean up
        $fclose(infile);
        $fclose(outfile);

        $display("Simulation finished.\nRun %0d test cases.", pass_cnt + fail_cnt);
        $finish;
    end

endmodule
