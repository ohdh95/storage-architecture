make h=1 -B
./ftl_test ./testcase/input1.txt 1.txt
make h=2 -B
./ftl_test ./testcase/input2.txt 2.txt
make h=3 -B
./ftl_test ./testcase/input3.txt 3.txt
make h=4 -B
./ftl_test ./testcase/input4.txt 4.txt
make h=5 -B
./ftl_test ./testcase/input5.txt 5.txt
make h=6 -B
./ftl_test ./testcase/input6.txt 6.txt
make h=7 -B
./ftl_test ./testcase/input7.txt 7.txt
make h=8 -B
./ftl_test ./testcase/input8.txt 8.txt

diff 1.txt ./testcase/output1.txt > result1.txt
diff 2.txt ./testcase/output2.txt > result2.txt
diff 3.txt ./testcase/output3.txt > result3.txt
diff 4.txt ./testcase/output4.txt > result4.txt
diff 5.txt ./testcase/output5.txt > result5.txt
diff 6.txt ./testcase/output6.txt > result6.txt
diff 7.txt ./testcase/output7.txt > result7.txt
diff 8.txt ./testcase/output8.txt > result8.txt