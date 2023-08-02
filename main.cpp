#include <bits/stdc++.h>

using namespace std;

typedef struct instruction {
	vector<int> mask;
	vector<int> constant;
} Instruction;

int main(){
	vector<int> DataArray;
	vector<int> DataPointer;
	vector<Instruction> InstructionsArray;
	vector<int> InstructionPointer;
}

vector<int> leftShift(vector<int>& arr){
	int n = arr.size();
	vector<int> ret;
	for(int i = 0; i < n; i++){
		ret.push_back(arr[(i+1)%n]);
	}
	return ret;
}

vector<int> rightShift(vector<int>& arr){
	int n = arr.size();
	vector<int> ret;
	for(int i = 0; i < n; i++){
		ret.push_back(arr[(i-1+n)%n]);
	}
	return ret;
}

vector<int> toggle(vector<int>& arr1, vector<int>& arr2){
	int n = arr1.size();
	vector<int> ret;
	for(int i = 0; i < n; i++){
		ret.push_back(arr1[i]^arr2[i]);
	}
	return ret;
}

Instruction getInstruction(vector<Instruction>& instructions, vector<int>& InstructionPointer){
	int n = InstructionPointer.size();
	Instruction instr;
	instr.mask.assign(instructions[0].mask.size(), 0);
	instr.constant.assign(instructions[0].constant.size(), 0);
	for(int i = 0; i < n; i++){
		int l = instructions[0].mask.size();
		for(int j = 0; j < l; j++){
			instr.mask[j] += InstructionPointer[i] * instructions[i].mask[j];
		}
		l = instructions[0].constant.size();
		for(int j = 0; j < l; j++){
			instr.constant[j] += InstructionPointer[i] * instructions[i].constant[j];
		}
	}
	return instr;
}

vector<int> LoopOpen(vector<int>& DA, vector<int>& DP, vector<int>& IP, Instruction& instr){
	int cond = 0;
	int n = DA.size();
	for(int i = 0; i < n; i++){
		cond += DA[i] * DP[i];
	}
	int oppcond = 1 - cond;
	vector<int> alt = rightShift(IP);
	vector<int> ret(IP.size(), 0);
	for(int i = 0; i < IP.size(); i++){
		ret[i] = cond * alt[i] + oppcond * instr.constant[i];
	}
	return ret;
}

vector<int> LoopClose(vector<int>& DA, vector<int>& DP, vector<int>& IP, Instruction& instr){
	int cond = 0;
	int n = DA.size();
	for(int i = 0; i < n; i++){
		cond += DA[i] * DP[i];
	}
	int oppcond = 1 - cond;
	vector<int> alt = rightShift(IP);
	vector<int> ret(IP.size(), 0);
	for(int i = 0; i < IP.size(); i++){
		ret[i] = oppcond * alt[i] + cond * instr.constant[i];
	}
	return ret;
}
