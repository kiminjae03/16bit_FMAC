# c 시뮬레이션 결과를 verilog 입력 파일로 생성
# c 시뮬레이션 결과 : #00001  43E8 880C 1076 03B1 03   <------  FAIL 0000   <- Expected is Underflow!
# verilog 입력 :     43E8 880C 1076 0000
#
# c 시뮬레이션 결과 : #00002  BDF2 8C0B 102F 1330 01   <------  PASS      
# verilog 입력 :      BDF2 8C0B 102F 1330

input_filename = '31_F16_Fmac_Normal_No_GRS_O/08_fmac_normal_no_grs_output__1.txt'
output_filename  = '07_fmac_verilog_input_1.txt'

with open(output_filename, 'w', encoding='utf-8') as out_f:
    out_f.write('')
    
with open(input_filename, 'r', encoding='utf-8') as f, open(output_filename, 'a', encoding='utf-8') as out_f:
    for line in f:
        if '#' in line:
            tokens = line.split()
            if len(tokens) < 8:
                continue
            if tokens[7] == 'PASS':
                results = ' '.join(tokens[1:5])
                out_f.write(results + '\n')
            elif tokens[7] == 'FAIL':
                results = ' '.join(tokens[1:4])
                results = results + ' ' + tokens[8]
                out_f.write(results + '\n')
            
        
