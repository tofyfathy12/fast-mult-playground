#!/usr/bin/env python3
# find_ntt_primes.py
# Search for primes p = k * 2^m + 1 and small primitive root g
#
# Usage: python3 find_ntt_primes.py
#
# Notes:
#  - This script uses a Miller-Rabin test that is deterministic for 64-bit values
#    (with the known base set). For larger values it uses extra MR rounds and
#    the Baillie-PSW-like approach (via an extra MR base) but remains probabilistic.
#  - For a *deterministic proof* of primality use an external prover (PARI/GP's isprime, Primo, ECPP).
#    Instructions to do that follow after the script.

import random, math, sys, time

# -------------------------
# Miller-Rabin primality test
# -------------------------
def is_probable_prime(n, rounds=10):
    """Probabilistic Miller-Rabin primality test.
       For 64-bit n we use a deterministic base set.
    """
    if n < 2:
        return False
    # small primes quick check
    small_primes = [2,3,5,7,11,13,17,19,23,29,31,37,41,43]
    for p in small_primes:
        if n % p == 0:
            return n == p
    # write n-1 = d * 2^s
    d = n - 1
    s = 0
    while d % 2 == 0:
        d //= 2
        s += 1

    def check_a(a):
        x = pow(a, d, n)
        if x == 1 or x == n - 1:
            return True
        for _ in range(s - 1):
            x = (x * x) % n
            if x == n - 1:
                return True
        return False

    # deterministic bases for testing 64-bit integers:
    # testing bases [2,325,9375,28178,450775,9780504,1795265022] is deterministic for n < 2^64
    if n < (1 << 64):
        bases = [2, 325, 9375, 28178, 450775, 9780504, 1795265022]
        for a in bases:
            if a % n == 0:
                return True
            if not check_a(a):
                return False
        return True

    # fallback: random bases, repeated rounds
    for _ in range(rounds):
        a = random.randrange(2, n - 1)
        if not check_a(a):
            return False
    # optional: one extra fixed base (Baillie-PSW style heuristic)
    return True

# -------------------------
# small-factorization of k (we only factor k which will be small)
# -------------------------
def distinct_prime_factors(n):
    """Return list of distinct prime factors of integer n (simple trial division)."""
    res = []
    t = n
    if t % 2 == 0:
        res.append(2)
        while t % 2 == 0:
            t //= 2
    f = 3
    while f * f <= t:
        if t % f == 0:
            res.append(f)
            while t % f == 0:
                t //= f
        f += 2
    if t > 1:
        res.append(t)
    return res

# -------------------------
# find primitive root
# -------------------------
def find_primitive_root(p, phi_factors):
    """Try small candidate g values until one is a primitive root modulo p.
       phi_factors is a list of distinct prime factors of p-1.
    """
    # small candidate list
    candidates = [2,3,5,7,11,13,17,19,23,29,31,37,41,43,47,53,59,61,67,71,73,79,83,89,97,101,103,107,109,113]
    for g in candidates:
        if g % p == 0: 
            continue
        ok = True
        for q in phi_factors:
            if pow(g, (p - 1) // q, p) == 1:
                ok = False
                break
        if ok:
            return g
    # fallback: try odd numbers beyond table
    cand = candidates[-1] + 2
    while cand < 10000:
        if cand % p == 0:
            cand += 2
            continue
        ok = True
        for q in phi_factors:
            if pow(cand, (p - 1) // q, p) == 1:
                ok = False
                break
        if ok:
            return cand
        cand += 2
    return None

# -------------------------
# main search loop
# -------------------------
def search_primes(m_min=33, m_max=48, k_max=20000, target=20):
    found = []
    t0 = time.time()
    for m in range(m_min, m_max + 1):
        two_m = 1 << m
        # iterate odd k only (k must be odd for p to be odd)
        for k in range(1, k_max + 1, 2):
            p = k * two_m + 1
            # quick primality test
            if not is_probable_prime(p, rounds=8):
                continue
            # p is a probable prime — factor k to get distinct factors of p-1
            k_factors = distinct_prime_factors(k)
            # include 2 as a factor (since p-1 includes 2^m)
            phi_factors = list(sorted(set(k_factors + [2])))
            g = find_primitive_root(p, phi_factors)
            if g is None:
                continue
            found.append((p, m, k, g))
            # print("Found p={} (m={}, k={}), g={}, elapsed {:.3f}s".format(p, m, k, g, time.time()-t0)) #debugging
            if len(found) >= target:
                return found
    return found

if __name__ == "__main__":
    # example parameters (same as my earlier run)
    m_min = 33
    m_max = 63
    k_max = 20000
    primes_number = 1000
    primes = search_primes(m_min, m_max, k_max, primes_number)
    print("\nCollected primes (count = {}):".format(len(primes)))
    for enum, prime in enumerate(primes, 1):
        
        print("{} : p = {}, m = {}, k = {}, g = {}".format(enum, prime[0], prime[1], prime[2], prime[3])) # p, m, k, g
