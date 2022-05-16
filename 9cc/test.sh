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

# 数式テスト
assert 21 "5+20-4"
assert 17 "10-8+15"

# 数式テスト　空白文字を含んだ数式を解釈できる
assert 41 " 12 + 34 - 5"
assert 100 "90 + 38 - 28"

assert 47 "5+6*7"
assert 15 '5*(9-6)'
assert 4 '(3+5)/2'
assert 10 '-10+20'
assert 10 '- -10'
assert 10 '- - +10'

echo OK
