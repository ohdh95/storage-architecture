make -B
./ftl_test input.txt a.txt
make CUSTOM_HEADER=1 -B
./ftl_test ./lab2_tc/input1.txt 1.txt
make CUSTOM_HEADER=2 -B
./ftl_test ./lab2_tc/input2.txt 2.txt
make CUSTOM_HEADER=3 -B
./ftl_test ./lab2_tc/input3.txt 3.txt
make CUSTOM_HEADER=4 -B
./ftl_test ./lab2_tc/input4.txt 4.txt
make CUSTOM_HEADER=5 -B
./ftl_test ./lab2_tc/input5.txt 5.txt
make CUSTOM_HEADER=6 -B
./ftl_test ./lab2_tc/input6.txt 6.txt
make CUSTOM_HEADER=7 -B
./ftl_test ./lab2_tc/input7.txt 7.txt
make CUSTOM_HEADER=8 -B
./ftl_test ./lab2_tc/input8.txt 8.txt

diff a.txt output.txt > result.txt
diff 1.txt ./lab2_tc/output1.txt > result1.txt
diff 2.txt ./lab2_tc/output2.txt > result2.txt
diff 3.txt ./lab2_tc/output3.txt > result3.txt
diff 4.txt ./lab2_tc/output4.txt > result4.txt
diff 5.txt ./lab2_tc/output5.txt > result5.txt
diff 6.txt ./lab2_tc/output6.txt > result6.txt
diff 7.txt ./lab2_tc/output7.txt > result7.txt
diff 8.txt ./lab2_tc/output8.txt > result8.txt