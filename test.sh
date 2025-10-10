GC_POLICY=$1

# 기본값 설정 (입력값이 없을 경우)
if [ -z "$GC_POLICY" ]; then
    echo "Usage: ./test.sh [GREEDY|CB|CAT]"
    exit 1
fi

make H=1 -B P=$GC_POLICY
./ftl_test ../lab3_tc/input1.txt 1.txt
make H=2 -B P=$GC_POLICY
./ftl_test ../lab3_tc/input2.txt 2.txt
make H=3 -B P=$GC_POLICY
./ftl_test ../lab3_tc/input3.txt 3.txt
make H=4 -B P=$GC_POLICY
./ftl_test ../lab3_tc/input4.txt 4.txt
make H=5 -B P=$GC_POLICY
./ftl_test ../lab3_tc/input5.txt 5.txt
make H=6 -B P=$GC_POLICY
./ftl_test ../lab3_tc/input6.txt 6.txt
make H=7 -B P=$GC_POLICY
./ftl_test ../lab3_tc/input7.txt 7.txt
make H=8 -B P=$GC_POLICY
./ftl_test ../lab3_tc/input8.txt 8.txt

diff 1.txt ../lab3_tc/output1.txt > result1.txt
diff 2.txt ../lab3_tc/output2.txt > result2.txt
diff 3.txt ../lab3_tc/output3.txt > result3.txt
diff 4.txt ../lab3_tc/output4.txt > result4.txt
diff 5.txt ../lab3_tc/output5.txt > result5.txt
diff 6.txt ../lab3_tc/output6.txt > result6.txt
diff 7.txt ../lab3_tc/output7.txt > result7.txt
diff 8.txt ../lab3_tc/output8.txt > result8.txt