#!/bin/bash

# Implementation of coo-test-cli script for test work

# Script name
BASH_CLI_SCRIPT_NAME="test-cli.sh"

# Option name
BASH_CLI_OPT_NAME[0]="-f"
BASH_CLI_OPT_NAME[1]="create"
BASH_CLI_OPT_NAME[2]="remove"
BASH_CLI_OPT_NAME[3]="test"
BASH_CLI_OPT_NAME[4]="show"

# Alternative option name
BASH_CLI_OPT_ALT_NAME[0]="--filename"
BASH_CLI_OPT_ALT_NAME[1]="new"
BASH_CLI_OPT_ALT_NAME[2]="delete"
BASH_CLI_OPT_ALT_NAME[3]="run"
BASH_CLI_OPT_ALT_NAME[4]="look"

# Data type consists of string, boolean, and cmd.
#   - string does not allow you set empty option value
#   - object allows you flag the option without giving value
#   - cmd is the command used in various situations in your script.
BASH_CLI_OPT_DATA_TYPE[0]="string"
BASH_CLI_OPT_DATA_TYPE[1]="cmd"
BASH_CLI_OPT_DATA_TYPE[2]="cmd"
BASH_CLI_OPT_DATA_TYPE[3]="cmd"
BASH_CLI_OPT_DATA_TYPE[4]="cmd"

# Setting mandatory and optional parameters for cmd "create"
#
# Mandatory parameter:
#  - BASH_CLI_OPT_NAME[1]="create" requires BASH_CLI_OPT_NAME[0]="-f"
#
# Non-Mandatory parameters:
#
BASH_CLI_MANDATORY_PARAM[1]="0"
BASH_CLI_MANDATORY_PARAM[2]="0"
BASH_CLI_NON_MANDATORY_PARAM[3]="0"
# BASH_CLI_NON_MANDATORY_PARAM[3]="1,2"

# Setting description of the option
BASH_CLI_OPT_DESC[0]="filename"
BASH_CLI_OPT_DESC[1]="To create test case with the value of -f"
BASH_CLI_OPT_DESC[2]="To delete test case with the value of -f"
BASH_CLI_OPT_DESC[3]="To run test cases in test directory, add -f will run designated test case"
BASH_CLI_OPT_DESC[4]="show all test case"

# Implementation of "create" command
#
# Getting parameter values
#
# BASH_CLI_OPT_VALUE[] is an array variable that declared by the template base.sh
# The value of BASH_CLI_OPT_VALUE[] will be managed by the template
#
create() {
    local filename=${BASH_CLI_OPT_VALUE[0]}

    if [ -e "./test/examples/${filename}.coo" ] && [ -e "./test/expect/${filename}.expect" ]; then
        echo "the test case exists already!"
        exit
    fi

    echo "[Start Creating]creating ./test/examples/${filename}.coo ./test/expect/${filename}.expect ..."
    touch "./test/examples/${filename}.coo"
    touch "./test/expect/${filename}.expect"

    exit
}

remove() {
    local filename=${BASH_CLI_OPT_VALUE[0]}

    echo "[Start Removing]removing ./test/examples/${filename}.coo ./test/expect/${filename}.expect ..."
    rm "./test/examples/${filename}.coo"
    rm "./test/expect/${filename}.expect"

    exit
}

test() {
    make

    local filename=${BASH_CLI_OPT_VALUE[0]}

    if [ ${filename} == "<undefined>" ]; then
        echo "[Start Testing]running all test cases..."
        bash ./script/testall.sh
    else
        echo "[Start Testing One File]running ${filename}"
        ./coo "test/examples/${filename}.coo" "test/output/${filename}"
        if [ $? -eq 0 ]; then
            echo "Building well, linking obejct..."
            clang -o "test/output/${filename}" "test/output/${filename}.o" "./build/obj/builtin.o"
            echo "Running test/output/${filename}"
            "test/output/${filename}" > "test/output/${filename}.result"
            echo "Comparing test/output/${filename}.result with test/expect/${filename}.expect"
            if cmp "test/output/${filename}.result" "test/expect/${filename}.expect"; then # cmp return `true` if same
                echo "[OK]pass ${filename} test case"
            else
                failed_arr+=(${f})
                echo "Execution Result Incorrect"
            fi
        else
            failed_arr+=(${f})
            echo "Build Fail"
        fi
    fi

    exit
}

show() {
    echo -e "[Showing Tests]showing test cases \n"

    for f in `find ./test/examples/*.coo -type f`
    do
        printf '%-50s\t' ${f}
        echo "`head ${f} -n 1`"
    done

    exit
}

source ./script/base.sh