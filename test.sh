#!/bin/bash
assert() {
  expected="$1"
  input="$2"

  ./xcc "$input" > tmp.s
  cc -o tmp samplefn.o tmp.s
  ./tmp
  actual="$?"

  if [ "$actual" = "$expected" ]; then
    echo "$input => $actual"
  else
    echo "$input => $expected expected, but got $actual"
    exit 1
  fi
}

assert 0 '0;'
assert 42 '42;'

assert 21 '5+20-4;'
assert 41 ' 12 + 34 - 5; '

assert 47 '5+6*7;'
assert 15 '5*(9-6);'
assert 4 '(3+5)/2;'

assert 10 '-10+20;'
assert 10 '- -10;'
assert 10 '- - +10;'

assert 0 '0==1;'
assert 1 '42==42;'
assert 1 '0!=1;'
assert 0 '42!=42;'

assert 1 '0<1;'
assert 0 '1<1;'
assert 0 '2<1;'
assert 1 '0<=1;'
assert 1 '1<=1;'
assert 0 '2<=1;'

assert 1 '1>0;'
assert 0 '1>1;'
assert 0 '1>2;'
assert 1 '1>=0;'
assert 1 '1>=1;'
assert 0 '1>=2;'

assert 10 'a=10;'
assert 10 'b=10;'
assert 10 'a=10;a;'
assert 10 'a=9;a+1;'
assert 10 'a=3;b=7;a+b;'
assert 1 'a=10;a==10;'
assert 1 'a=10;a!=0;'
assert 1 'a=10;a>9;'

assert 10 'abc=10;'
assert 10 'def=10;'
assert 10 'abc=10;abc;'
assert 10 'abc=9;abc+1;'
assert 10 'abc=3;def=7;abc+def;'
assert 1 'abc=10;abc==10;'
assert 1 'abc=10;abc!=0;'
assert 1 'abc=10;abc>9;'

assert 10 'return 10;'
assert 10 '0; return 10;'
assert 10 'a=10; return a;'
assert 10 'a=9; return a+1;'
assert 10 'return 10; 9;'

assert 10 'if (0==0) 10;'
assert 10 'if (0==1) 9; 10;'
assert 10 'if (0==0) 10; else 9;'
assert 10 'if (0==1) 9; else 10;'

assert 10 'x = 0; while (x < 10) x = x + 1; x;'
assert 10 'x = 10; while (x < 0) x = x + 1; x;'

assert 10 'for (x = 0; x < 10; x = x + 1) x;'
assert 10 'for (x = 10; x < 0; x = x + 1) x;'
assert 10 'x = 0; for (; x < 10;) x = x + 1;'
assert 10 'for (;;) return 10;'

assert 10 '{10;}'
assert 10 '{9; 10;}'
assert 10 '{return 10; 9;}'
assert 10 '{} 10;'
assert 10 '{0;0;0;0;0;0;0;0;0;10;}'   # 10個
assert 10 '{0;0;0;0;0;0;0;0;0;0;10;}' # 11個

assert 10 'foo();10;'

echo OK
