make -B
./ftl_test input.txt b.txt
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