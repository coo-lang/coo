EXAMPLES_PATH=test/examples
OUTPUT_PATH=test/output

sum=0
success=0

echo "start testing..."
echo "======================================\n"

for f in `find ${EXAMPLES_PATH}/*.coo -type f`
do
    sum=`expr $sum + 1`
    echo "compile ${f}..."
    ./coo ${f} ${OUTPUT_PATH}/$(basename ${f%.*}).ll
    if [ $? -eq 0 ]; then
        echo "OK"
        success=`expr $success + 1`
    else
        echo "Fail"
    fi
    echo "======================================\n"
done

echo "total test cases: ${sum}, successed ${success}, failed `expr ${sum} - ${success}`"