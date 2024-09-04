#include "seal/seal.h"
#include <iostream>
#include <iomanip>
#include <vector>
#include <cmath>

using namespace std;
using namespace seal;

class Homo{
public:
    uint64_t plain_modulus = 257;
    SecretKey secret_key;
    PublicKey public_key;
    unique_ptr<Encryptor> encryptor;
    unique_ptr<Evaluator> evaluator;
    unique_ptr<Decryptor> decryptor;
    RelinKeys relin_keys;
    
    Homo(){
        // Step 1: Set up encryption parameters
        EncryptionParameters parms(scheme_type::bfv);
        size_t poly_modulus_degree = 4096 * 2 * 2;
        parms.set_poly_modulus_degree(poly_modulus_degree);
        parms.set_coeff_modulus(CoeffModulus::BFVDefault(poly_modulus_degree));
        parms.set_plain_modulus(plain_modulus);

        // Create SEALContext
        SEALContext context(parms);

        // Step 2: Key generation
        KeyGenerator keygen(context);
        secret_key = keygen.secret_key();
        keygen.create_public_key(public_key);
        
        // Create encryptor, evaluator, and decryptor
        encryptor = make_unique<Encryptor>(context, public_key);
        evaluator = make_unique<Evaluator>(context);
        decryptor = make_unique<Decryptor>(context, secret_key);

        keygen.create_relin_keys(relin_keys);
    
    }
    Ciphertext enc(uint64_t msg){
        Ciphertext c;
        encryptor->encrypt(seal::util::uint_to_hex_string(&msg, std::size_t(1)), c);
        return c;
    }
    Plaintext dec(Ciphertext c){
        Plaintext p;
        decryptor->decrypt(c, p);
        return p;
    }
    Ciphertext add(Ciphertext a, Ciphertext b){
        Ciphertext c;
        evaluator->add(a, b, c);
        return c;
    }

    Ciphertext sub(Ciphertext a, Ciphertext b){
        Ciphertext c;
        evaluator->sub(a, b, c);
        return c;
    }

    Ciphertext mul(Ciphertext a, Ciphertext b){
        Ciphertext c;
        evaluator->multiply(a, b, c);
        evaluator->relinearize_inplace(c, relin_keys);
        return c;
    }

    void refresh(Ciphertext& a){
        Plaintext tmp;
        // TODO: replace this with sending to client
        decryptor->decrypt(a, tmp);
        encryptor->encrypt(tmp, a);
    }

    Ciphertext exp_constant(Ciphertext base_ct, uint64_t exp){
        // Result starts as 1
        Ciphertext result_ct;
        Plaintext one_plaintext("1");
        encryptor->encrypt(one_plaintext, result_ct);

        // Current base starts as the base_ct
        Ciphertext current_ct = base_ct;
        
        while (exp > 0) {
            refresh(current_ct);
            if (exp % 2 == 1) {
                // result_ct = result_ct * current_ct
                Ciphertext temp_ct;
                evaluator->multiply(result_ct, current_ct, temp_ct);
                evaluator->relinearize_inplace(temp_ct, relin_keys);
                result_ct = temp_ct;

                refresh(result_ct);

            }
            // current_ct = current_ct^2
            Ciphertext temp_ct;
            evaluator->square(current_ct, temp_ct);
            evaluator->relinearize_inplace(temp_ct, relin_keys);
            current_ct = temp_ct;
            exp /= 2;
        }
        return result_ct;
    }

    // Function to create an encrypted result indicating equality using Fermat's Little Theorem
    Ciphertext isEqual(const Ciphertext &ct1, const Ciphertext &ct2) {
        // Compute the difference between ct1 and ct2
        Ciphertext diff_ct;
        evaluator->sub(ct1, ct2, diff_ct);

        // Compute (A - B) ^ (p - 1)
        Ciphertext power_ct;
        power_ct = exp_constant(diff_ct , plain_modulus - 1);
        
        Ciphertext result_ct;
        evaluator->sub(enc(1), power_ct, result_ct);
        return result_ct;
    }

    Ciphertext notEqual(const Ciphertext &ct1, const Ciphertext &ct2) {
        return sub(enc(1), isEqual(ct1, ct2));
    }

    Ciphertext at(const vector<Ciphertext>& arr, const Ciphertext& idx){
        auto res = enc(0);
        for(int i = 0; i < arr.size(); i++){
            res = add(res, mul(arr[i], isEqual(enc(i), idx)));
            refresh(res);
        }
        return res;
    }

    void inc_at(vector<Ciphertext>& arr, const Ciphertext& idx, const Ciphertext& mask){
        for(int i = 0; i < arr.size(); i++){
            arr[i] = add(arr[i], mul(mask, isEqual(enc(i), idx)));
        }
    }

    void red_at(vector<Ciphertext>& arr, const Ciphertext& idx, const Ciphertext& mask){
        for(int i = 0; i < arr.size(); i++){
            arr[i] = sub(arr[i], mul(mask, isEqual(enc(i), idx)));
        }
    }
};

int main(){
    Homo* homo = new Homo();
    auto x1 = homo->enc(1);
    auto x2 = homo->enc(1);
    auto x3 = homo->isEqual(x1, x2);
    cout << homo->dec(x3).to_string() << endl;

    vector<Ciphertext> tape;
    for(int i = 0; i < 30; i++)
        tape.push_back(homo->enc(0));
    
    vector<Ciphertext> code;
    std::string code_plain = "++++++++[>++++++++<-]>+.";
    code_plain = ">+>+>->->><<<<<<";
    for(char c : code_plain)
        code.push_back(homo->enc(c));
    cout << homo->dec(homo->at(code, homo->enc(5))).to_string() << endl;
    int iterations = 100;
    // > 0 
    // < 1
    // + 2
    // - 3
    // . 4
    // , 5
    // [ 6
    // ] 7

    auto pc = homo->enc(0);
    auto tape_ptr = homo->enc(0);
    const auto END = homo->enc(tape.size() - 1);
    const auto START = homo->enc(0);
    while(iterations--){
        cout << "ITER::" << 100 - iterations << endl;
        auto curr_op = homo->at(code, pc);
        
        /// >
        auto update_mask = homo->isEqual(curr_op, homo->enc('>'));
        update_mask = homo->mul(update_mask, homo->notEqual(END, pc));
        pc = homo->add(pc, homo->mul(homo->enc(1), update_mask));
        tape_ptr = homo->add(tape_ptr, homo->mul(homo->enc(1), update_mask));
        homo->refresh(pc);
        homo->refresh(tape_ptr);
        
        /// <
        update_mask = homo->isEqual(curr_op, homo->enc('<'));
        update_mask = homo->mul(update_mask, homo->notEqual(START, pc));
        pc = homo->add(pc, homo->mul(homo->enc(1), update_mask));
        tape_ptr = homo->sub(tape_ptr, homo->mul(homo->enc(1), update_mask));
        homo->refresh(pc);
        homo->refresh(tape_ptr);
        
        
        /// +
        update_mask = homo->isEqual(curr_op, homo->enc('+'));
        homo->inc_at(tape, tape_ptr, update_mask);
        pc = homo->add(pc, homo->mul(homo->enc(1), update_mask));
        homo->refresh(pc);

        /// -
        update_mask = homo->isEqual(curr_op, homo->enc('-'));
        homo->red_at(tape, tape_ptr, update_mask);
        pc = homo->add(pc, homo->mul(homo->enc(1), update_mask));
        homo->refresh(pc);

        cout << "PC::" << homo->dec(pc).to_string() << endl;
        cout << "TAPE_PTR::" << homo->dec(tape_ptr).to_string() << endl;
        cout << "TAPE_VAL::" << homo->dec(homo->at(tape, tape_ptr)).to_string() << endl;
    }

}