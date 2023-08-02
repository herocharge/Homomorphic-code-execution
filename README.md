## Homomorphic code execution

###### Motivation:
I want to achieve fully encrypted code execution, i.e., given code that one writes, we should be able encrypt it and the data that it will work and send it to some untrusted PC/system/Cloud Provider which can then perform the computation without even know what it executed. This would prevent fingerprinting totally.


##### What is this repo?
Usin google's (fully homomorphic encryption)[https://github.com/google/fully-homomorphic-encryption], I would like some sort of turing complete language, as a proof of concept that it is possible. I chose `boolfuck` as the language that I would implement. It is very simple to implement. Once implemented, I would like to try out what kind of things I can infer from the execution.

