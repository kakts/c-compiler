#!/bin/bash

# assert用関数
# 入力値と期待される出力の値を受け取る
# 実際に9ccの結果をアセンブルし、実際の結果を期待される値と比較
assert() {
    expected="$1"
    input="$2"

    ./9cc "$input" > tmp.s
    cc -o tmp tmp.s
    ./tmp
    actual="$?"

    if [ "$actual" = "$expected" ]; then
        echo "$input => $actual"
    else
        echo "$input => $expected expected, but got $actual"
        exit 1
    fi
}

assert 0 0
assert 42 42

echo OK