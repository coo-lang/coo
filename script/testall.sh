EXAMPLES_PATH=test/examples
OUTPUT_PATH=test/output
EXPECT_PATH=test/expect

sum=0
success=0

echo ""
echo "start testing..."
echo "======================================"

failed_arr=()

for f in `find ${EXAMPLES_PATH}/*.coo -type f`
do
    sum=`expr $sum + 1`
    echo "compile ${f}..."
    name=$(basename ${f%.*})
    ./coo ${f} ${OUTPUT_PATH}/${name}
    if [ $? -eq 0 ]; then
        echo "BUILD FINE"
        clang -o ${OUTPUT_PATH}/${name} ${OUTPUT_PATH}/${name}.o ./build/obj/builtin.o
        ./${OUTPUT_PATH}/${name} > ./${OUTPUT_PATH}/${name}.result
        if cmp ./${OUTPUT_PATH}/${name}.result ./${EXPECT_PATH}/${name}.expect; then # cmp return `true` if same
            success=`expr $success + 1`
        else
            failed_arr+=(${f})
            echo "Fail"
        fi
    else
        failed_arr+=(${f})
        echo "Fail"
    fi
    echo "======================================"
done

echo "total test cases: ${sum}, successed ${success}, failed `expr ${sum} - ${success}`"

echo "======================================"
echo "Failed tes cases:"
printf '%s\n' "${failed_arr[@]}"