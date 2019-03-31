## Coo

Coo is a programming language designed and implemented by [pwxcoo](https://github.com/pwxcoo).

There is documentation about coo-lang. [Coo-docs](https://coo-docs.readthedocs.io/en/latest/).

Coo has following characteristics:

- Turing Completeness
- Statically Typed
- Simple Type Inferring
- Lambda Expression
- Lazy Evaluation
- ...

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

This command will run all test cases in `test/`.

## Example

Here are sample programs written in `Coo`:

- fibonacci.coo
    ```coo
    def fibonacci(n: int): int {
        if n <= 1 {
            ret n
        } else {
            ret fibonacci(n - 1) + fibonacci(n - 2)
        }
    }

    for var i = 1; i <= 30; i = i + 1 {
        println("%d level fibonacci result is %d", i, fibonacci(i))
    }
    ```

- bubble_sort.coo
    ```coo
    /**
    * declare a random array and using bubble sort to sort it (using lambda expression).
    */

    // sort function with comparison function
    def sort(array: []int, n: int, cmp: (int, int)->bool) : void {
        for var i = 0; i < n; i = i + 1 {
            for var j = 0; j < n - 1 - i; j = j + 1 {
                if cmp(array[j], array[j + 1]) == false {
                    var temp = array[j]
                    array[j] = array[j + 1]
                    array[j + 1] = temp
                }
            }
        }
    }

    // invoke sort() with lambda expression [O(n^2) bubble sort algorithm]
    var arr: [10]int = {8, 2, 1, 5, 6, 3, 4, 0, 9, 7}
    sort(arr, 10, (a: int, b: int): bool-> {
        if a <= b {
            ret true
        } else {
            ret false
        }
    })

    // print sorted array
    for var i = 0; i < 10; i = i + 1 {
        println("%d", arr[i])
    }
    ```
- draw_love.coo
    ```coo
    for var y = 1.5f; y > -1.5f; y = y - 0.1f {
        for var x = -1.5f; x < 1.5f; x = x + 0.05f {
            var a = x * x + y * y - 1.0f
            if a * a * a - x * x * y * y * y <= 0.0f {
                put("*")
            } else {
                put(".")
            }
        }
        println(".")
    }

    .............................................................
    .............................................................
    .............................................................
    ................*********...........*********................
    ............*****************...*****************............
    ..........****************************************...........
    .........*******************************************.........
    ........*********************************************........
    ........*********************************************........
    ........*********************************************........
    ........*********************************************........
    ........*********************************************........
    ........*********************************************........
    .........*******************************************.........
    ..........*****************************************..........
    ..........****************************************...........
    ............*************************************............
    .............***********************************.............
    ..............*********************************..............
    ................*****************************................
    ..................*************************..................
    ....................*********************....................
    .......................***************.......................
    ..........................*********..........................
    .............................***.............................
    .............................................................
    .............................................................
    .............................................................
    .............................................................
    .............................................................

    ```



