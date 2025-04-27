int subtract(int a, int b) {
    unsigned int ua = (unsigned int)a;
    unsigned int ub = (unsigned int)b;
    unsigned int result = 0;
    unsigned int borrow = 0;

    // Bit 0
    unsigned int a0 = (ua >> 0) & 1;
    unsigned int b0 = (ub >> 0) & 1;
    unsigned int d0 = a0 ^ b0 ^ borrow;
    result |= (d0 << 0);
    borrow = ((~a0 & b0) | (~(a0 ^ b0) & borrow)) & 1;

    // Bit 1
    unsigned int a1 = (ua >> 1) & 1;
    unsigned int b1 = (ub >> 1) & 1;
    unsigned int d1 = a1 ^ b1 ^ borrow;
    result |= (d1 << 1);
    borrow = ((~a1 & b1) | (~(a1 ^ b1) & borrow)) & 1;

    // Bit 2
    unsigned int a2 = (ua >> 2) & 1;
    unsigned int b2 = (ub >> 2) & 1;
    unsigned int d2 = a2 ^ b2 ^ borrow;
    result |= (d2 << 2);
    borrow = ((~a2 & b2) | (~(a2 ^ b2) & borrow)) & 1;

    // Bit 3
    unsigned int a3 = (ua >> 3) & 1;
    unsigned int b3 = (ub >> 3) & 1;
    unsigned int d3 = a3 ^ b3 ^ borrow;
    result |= (d3 << 3);
    borrow = ((~a3 & b3) | (~(a3 ^ b3) & borrow)) & 1;

    // Bit 4
    unsigned int a4 = (ua >> 4) & 1;
    unsigned int b4 = (ub >> 4) & 1;
    unsigned int d4 = a4 ^ b4 ^ borrow;
    result |= (d4 << 4);
    borrow = ((~a4 & b4) | (~(a4 ^ b4) & borrow)) & 1;

    // Bit 5
    unsigned int a5 = (ua >> 5) & 1;
    unsigned int b5 = (ub >> 5) & 1;
    unsigned int d5 = a5 ^ b5 ^ borrow;
    result |= (d5 << 5);
    borrow = ((~a5 & b5) | (~(a5 ^ b5) & borrow)) & 1;

    // Bit 6
    unsigned int a6 = (ua >> 6) & 1;
    unsigned int b6 = (ub >> 6) & 1;
    unsigned int d6 = a6 ^ b6 ^ borrow;
    result |= (d6 << 6);
    borrow = ((~a6 & b6) | (~(a6 ^ b6) & borrow)) & 1;

    // Bit 7
    unsigned int a7 = (ua >> 7) & 1;
    unsigned int b7 = (ub >> 7) & 1;
    unsigned int d7 = a7 ^ b7 ^ borrow;
    result |= (d7 << 7);
    borrow = ((~a7 & b7) | (~(a7 ^ b7) & borrow)) & 1;

    // Bit 8
    unsigned int a8 = (ua >> 8) & 1;
    unsigned int b8 = (ub >> 8) & 1;
    unsigned int d8 = a8 ^ b8 ^ borrow;
    result |= (d8 << 8);
    borrow = ((~a8 & b8) | (~(a8 ^ b8) & borrow)) & 1;

    // Bit 9
    unsigned int a9 = (ua >> 9) & 1;
    unsigned int b9 = (ub >> 9) & 1;
    unsigned int d9 = a9 ^ b9 ^ borrow;
    result |= (d9 << 9);
    borrow = ((~a9 & b9) | (~(a9 ^ b9) & borrow)) & 1;

    // Bit 10
    unsigned int a10 = (ua >> 10) & 1;
    unsigned int b10 = (ub >> 10) & 1;
    unsigned int d10 = a10 ^ b10 ^ borrow;
    result |= (d10 << 10);
    borrow = ((~a10 & b10) | (~(a10 ^ b10) & borrow)) & 1;

    // Bit 11
    unsigned int a11 = (ua >> 11) & 1;
    unsigned int b11 = (ub >> 11) & 1;
    unsigned int d11 = a11 ^ b11 ^ borrow;
    result |= (d11 << 11);
    borrow = ((~a11 & b11) | (~(a11 ^ b11) & borrow)) & 1;

    // Bit 12
    unsigned int a12 = (ua >> 12) & 1;
    unsigned int b12 = (ub >> 12) & 1;
    unsigned int d12 = a12 ^ b12 ^ borrow;
    result |= (d12 << 12);
    borrow = ((~a12 & b12) | (~(a12 ^ b12) & borrow)) & 1;

    // Bit 13
    unsigned int a13 = (ua >> 13) & 1;
    unsigned int b13 = (ub >> 13) & 1;
    unsigned int d13 = a13 ^ b13 ^ borrow;
    result |= (d13 << 13);
    borrow = ((~a13 & b13) | (~(a13 ^ b13) & borrow)) & 1;

    // Bit 14
    unsigned int a14 = (ua >> 14) & 1;
    unsigned int b14 = (ub >> 14) & 1;
    unsigned int d14 = a14 ^ b14 ^ borrow;
    result |= (d14 << 14);
    borrow = ((~a14 & b14) | (~(a14 ^ b14) & borrow)) & 1;

    // Bit 15
    unsigned int a15 = (ua >> 15) & 1;
    unsigned int b15 = (ub >> 15) & 1;
    unsigned int d15 = a15 ^ b15 ^ borrow;
    result |= (d15 << 15);
    borrow = ((~a15 & b15) | (~(a15 ^ b15) & borrow)) & 1;

    // Bit 16
    unsigned int a16 = (ua >> 16) & 1;
    unsigned int b16 = (ub >> 16) & 1;
    unsigned int d16 = a16 ^ b16 ^ borrow;
    result |= (d16 << 16);
    borrow = ((~a16 & b16) | (~(a16 ^ b16) & borrow)) & 1;

    // Bit 17
    unsigned int a17 = (ua >> 17) & 1;
    unsigned int b17 = (ub >> 17) & 1;
    unsigned int d17 = a17 ^ b17 ^ borrow;
    result |= (d17 << 17);
    borrow = ((~a17 & b17) | (~(a17 ^ b17) & borrow)) & 1;

    // Bit 18
    unsigned int a18 = (ua >> 18) & 1;
    unsigned int b18 = (ub >> 18) & 1;
    unsigned int d18 = a18 ^ b18 ^ borrow;
    result |= (d18 << 18);
    borrow = ((~a18 & b18) | (~(a18 ^ b18) & borrow)) & 1;

    // Bit 19
    unsigned int a19 = (ua >> 19) & 1;
    unsigned int b19 = (ub >> 19) & 1;
    unsigned int d19 = a19 ^ b19 ^ borrow;
    result |= (d19 << 19);
    borrow = ((~a19 & b19) | (~(a19 ^ b19) & borrow)) & 1;

    // Bit 20
    unsigned int a20 = (ua >> 20) & 1;
    unsigned int b20 = (ub >> 20) & 1;
    unsigned int d20 = a20 ^ b20 ^ borrow;
    result |= (d20 << 20);
    borrow = ((~a20 & b20) | (~(a20 ^ b20) & borrow)) & 1;

    // Bit 21
    unsigned int a21 = (ua >> 21) & 1;
    unsigned int b21 = (ub >> 21) & 1;
    unsigned int d21 = a21 ^ b21 ^ borrow;
    result |= (d21 << 21);
    borrow = ((~a21 & b21) | (~(a21 ^ b21) & borrow)) & 1;

    // Bit 22
    unsigned int a22 = (ua >> 22) & 1;
    unsigned int b22 = (ub >> 22) & 1;
    unsigned int d22 = a22 ^ b22 ^ borrow;
    result |= (d22 << 22);
    borrow = ((~a22 & b22) | (~(a22 ^ b22) & borrow)) & 1;

    // Bit 23
    unsigned int a23 = (ua >> 23) & 1;
    unsigned int b23 = (ub >> 23) & 1;
    unsigned int d23 = a23 ^ b23 ^ borrow;
    result |= (d23 << 23);
    borrow = ((~a23 & b23) | (~(a23 ^ b23) & borrow)) & 1;

    // Bit 24
    unsigned int a24 = (ua >> 24) & 1;
    unsigned int b24 = (ub >> 24) & 1;
    unsigned int d24 = a24 ^ b24 ^ borrow;
    result |= (d24 << 24);
    borrow = ((~a24 & b24) | (~(a24 ^ b24) & borrow)) & 1;

    // Bit 25
    unsigned int a25 = (ua >> 25) & 1;
    unsigned int b25 = (ub >> 25) & 1;
    unsigned int d25 = a25 ^ b25 ^ borrow;
    result |= (d25 << 25);
    borrow = ((~a25 & b25) | (~(a25 ^ b25) & borrow)) & 1;

    // Bit 26
    unsigned int a26 = (ua >> 26) & 1;
    unsigned int b26 = (ub >> 26) & 1;
    unsigned int d26 = a26 ^ b26 ^ borrow;
    result |= (d26 << 26);
    borrow = ((~a26 & b26) | (~(a26 ^ b26) & borrow)) & 1;

    // Bit 27
    unsigned int a27 = (ua >> 27) & 1;
    unsigned int b27 = (ub >> 27) & 1;
    unsigned int d27 = a27 ^ b27 ^ borrow;
    result |= (d27 << 27);
    borrow = ((~a27 & b27) | (~(a27 ^ b27) & borrow)) & 1;

    // Bit 28
    unsigned int a28 = (ua >> 28) & 1;
    unsigned int b28 = (ub >> 28) & 1;
    unsigned int d28 = a28 ^ b28 ^ borrow;
    result |= (d28 << 28);
    borrow = ((~a28 & b28) | (~(a28 ^ b28) & borrow)) & 1;

    // Bit 29
    unsigned int a29 = (ua >> 29) & 1;
    unsigned int b29 = (ub >> 29) & 1;
    unsigned int d29 = a29 ^ b29 ^ borrow;
    result |= (d29 << 29);
    borrow = ((~a29 & b29) | (~(a29 ^ b29) & borrow)) & 1;

    // Bit 30
    unsigned int a30 = (ua >> 30) & 1;
    unsigned int b30 = (ub >> 30) & 1;
    unsigned int d30 = a30 ^ b30 ^ borrow;
    result |= (d30 << 30);
    borrow = ((~a30 & b30) | (~(a30 ^ b30) & borrow)) & 1;

    // Bit 31
    unsigned int a31 = (ua >> 31) & 1;
    unsigned int b31 = (ub >> 31) & 1;
    unsigned int d31 = a31 ^ b31 ^ borrow;
    result |= (d31 << 31);

    return (int)result;
}
