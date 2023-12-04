# French Conjugations

Get French Conjugations from [Reverso Conjugator](conjugator.reverso.net).

Currently, there is a Windows implementation of a socket which (for reasons unbeknownst to me) does not work.


## Windows Compilation
```sh
> gcc src/main.c -lws2_32 -o fconj.exe
```