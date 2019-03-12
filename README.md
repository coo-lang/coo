## Coo

Coo is a programming language designed and implemented by [pwxcoo](https://github.com/pwxcoo).

There is documentation about coo-lang. [Coo-docs](https://coo-docs.readthedocs.io/en/latest/).

## Prerequisites

- `LLVM 6.0`

## Usage

```sh
$ make
```

There will be a `coo` executable compiler in root directory. You can compile a text file suffixed with `.coo` to object file with it.

**PS: When you write coo, you can install [coo-vscode](https://marketplace.visualstudio.com/items?itemName=pwxcoo.coo-vscode) extension in vscode. It support coo-lang in vscode editor.**

## Test

```sh
$ ./test-cli test
```

This command will run all test case in `test/`.

## Example

Here are samples program written in `Coo`:

- fibonacci.coo
    ```coo
    def fibonacci(n: int): int {
        if n <= 1 {
            ret n
        } else {
            ret fibonacci(n - 1) + fibonacci(n - 2)
        }
    }

    var i: int
    for i = 1; i <= 30; i = i + 1 {
        print("%d level fibonacci result is %d", i, fibonacci(i))
    }
    ```

- bubble_sort.coo
    ```coo
    /**
    * declare a random array and using bubble sort to sort it.
    */

    // initialize array
    var n: int = 100
    var arr: [100]int = {73,79,49,11,78,6,86,95,63,96,92,5,47,56,100,25,51,1,46,59,94,99,2,67,58,97,21,80,98,74,84,29,90,31,68,43,18,23,37,69,30,24,88,48,44,34,42,17,62,91,36,28,66,26,83,61,53,89,33,15,38,71,12,9,64,85,7,35,50,70,41,19,52,45,81,4,13,76,14,60,40,16,27,39,72,10,32,54,20,82,57,3,77,55,87,65,8,93,22,75}

    // O(n^2) bubble sort algorithm
    for var i = n - 1; i >= 0; i = i - 1 {
        for var j = 0; j < i; j = j + 1 {
            if arr[j] > arr[j + 1] {
                var temp = arr[j]
                arr[j] = arr[j + 1]
                arr[j + 1] = temp
            }
        }
    }

    // print sorted array
    for i = 0; i < n; i = i + 1 {
        print("%d", arr[i])
    }
    ```



