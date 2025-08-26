#pragma once

#include <gmp.h>
#include <string>
#include <memory>
#include <climits>
#include <cmath>

namespace GMPWrapper {
    // RAII wrapper for mpz_t
    class BigInt {
    private:
        mpz_t value;
        bool initialized;
        
    public:
        BigInt() : initialized(false) {
            mpz_init(value);
            initialized = true;
        }
        
        BigInt(const std::string& str) : initialized(false) {
            mpz_init(value);
            mpz_set_str(value, str.c_str(), 10);
            initialized = true;
        }
        
        BigInt(long long val) : initialized(false) {
            mpz_init(value);
            mpz_set_si(value, val);
            initialized = true;
        }
        
        BigInt(const BigInt& other) : initialized(false) {
            mpz_init(value);
            mpz_set(value, other.value);
            initialized = true;
        }
        
        BigInt(BigInt&& other) noexcept : initialized(false) {
            mpz_init(value);
            mpz_set(value, other.value);
            initialized = true;
            other.initialized = false;
        }
        
        ~BigInt() {
            if (initialized) {
                mpz_clear(value);
            }
        }
        
        BigInt& operator=(const BigInt& other) {
            if (this != &other) {
                mpz_set(value, other.value);
            }
            return *this;
        }
        
        BigInt& operator=(BigInt&& other) noexcept {
            if (this != &other) {
                mpz_set(value, other.value);
                other.initialized = false;
            }
            return *this;
        }
        
        // Arithmetic operations
        BigInt operator+(const BigInt& other) const {
            BigInt result;
            mpz_add(result.value, value, other.value);
            return result;
        }
        
        BigInt operator-(const BigInt& other) const {
            BigInt result;
            mpz_sub(result.value, value, other.value);
            return result;
        }
        
        BigInt operator*(const BigInt& other) const {
            BigInt result;
            mpz_mul(result.value, value, other.value);
            return result;
        }
        
        BigInt operator/(const BigInt& other) const {
            BigInt result;
            mpz_fdiv_q(result.value, value, other.value);
            return result;
        }
        
        BigInt operator%(const BigInt& other) const {
            BigInt result;
            mpz_fdiv_r(result.value, value, other.value);
            return result;
        }
        
        // Comparison operators
        bool operator==(const BigInt& other) const {
            return mpz_cmp(value, other.value) == 0;
        }
        
        bool operator!=(const BigInt& other) const {
            return mpz_cmp(value, other.value) != 0;
        }
        
        bool operator<(const BigInt& other) const {
            return mpz_cmp(value, other.value) < 0;
        }
        
        bool operator<=(const BigInt& other) const {
            return mpz_cmp(value, other.value) <= 0;
        }
        
        bool operator>(const BigInt& other) const {
            return mpz_cmp(value, other.value) > 0;
        }
        
        bool operator>=(const BigInt& other) const {
            return mpz_cmp(value, other.value) >= 0;
        }
        
        // Conversion methods
        std::string toString() const {
            char* str = mpz_get_str(nullptr, 10, value);
            std::string result(str);
            free(str);
            return result;
        }
        
        long long toLongLong() const {
            return mpz_get_si(value);
        }
        
        double toDouble() const {
            return mpz_get_d(value);
        }
        
            // Check if fits in long long without loss of precision
    bool fitsInLongLong() const {
        return mpz_fits_slong_p(value) && 
               mpz_get_si(value) <= LLONG_MAX && 
               mpz_get_si(value) >= LLONG_MIN;
    }
    
    // Check if fits in double without loss of precision
    bool fitsInDouble() const {
        return mpz_fits_slong_p(value) && 
               mpz_get_si(value) <= 9007199254740991LL && 
               mpz_get_si(value) >= -9007199254740991LL;
    }
        
        // Static factory methods
        static BigInt fromString(const std::string& str) {
            return BigInt(str);
        }
        
        static BigInt fromLongLong(long long val) {
            return BigInt(val);
        }
        
        static BigInt fromDouble(double val) {
            return BigInt(static_cast<long long>(val));
        }
    };
    
    // Utility functions
    inline bool shouldPromoteToBigInt(double d) {
        // Check if double is an integer and would lose precision
        if (d != std::floor(d)) return false;
        
        const double MAX_SAFE_INTEGER = 9007199254740991.0; // 2^53 - 1
        const double MIN_SAFE_INTEGER = -9007199254740991.0; // -(2^53 - 1)
        
        return d > MAX_SAFE_INTEGER || d < MIN_SAFE_INTEGER;
    }
    
    inline bool shouldPromoteToBigInt(long long d) {
        // For long long, we only promote if it would overflow in arithmetic
        // This is a conservative approach - we'll let arithmetic operations decide
        return false; // Let arithmetic operations handle overflow detection
    }
    
    inline BigInt doubleToBigInt(double d) {
        return BigInt::fromLongLong(static_cast<long long>(d));
    }
}
