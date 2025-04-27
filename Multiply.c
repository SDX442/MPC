int multiply(int a, int b) {
    unsigned int ua = (unsigned int)(a < 0 ? -a : a);
    unsigned int ub = (unsigned int)(b < 0 ? -b : b);
    unsigned int result = 0;

    result += (-((ub >>  0) & 1) & (ua <<  0));
    result += (-((ub >>  1) & 1) & (ua <<  1));
    result += (-((ub >>  2) & 1) & (ua <<  2));
    result += (-((ub >>  3) & 1) & (ua <<  3));
    result += (-((ub >>  4) & 1) & (ua <<  4));
    result += (-((ub >>  5) & 1) & (ua <<  5));
    result += (-((ub >>  6) & 1) & (ua <<  6));
    result += (-((ub >>  7) & 1) & (ua <<  7));
    result += (-((ub >>  8) & 1) & (ua <<  8));
    result += (-((ub >>  9) & 1) & (ua <<  9));
    result += (-((ub >> 10) & 1) & (ua << 10));
    result += (-((ub >> 11) & 1) & (ua << 11));
    result += (-((ub >> 12) & 1) & (ua << 12));
    result += (-((ub >> 13) & 1) & (ua << 13));
    result += (-((ub >> 14) & 1) & (ua << 14));
    result += (-((ub >> 15) & 1) & (ua << 15));
    result += (-((ub >> 16) & 1) & (ua << 16));
    result += (-((ub >> 17) & 1) & (ua << 17));
    result += (-((ub >> 18) & 1) & (ua << 18));
    result += (-((ub >> 19) & 1) & (ua << 19));
    result += (-((ub >> 20) & 1) & (ua << 20));
    result += (-((ub >> 21) & 1) & (ua << 21));
    result += (-((ub >> 22) & 1) & (ua << 22));
    result += (-((ub >> 23) & 1) & (ua << 23));
    result += (-((ub >> 24) & 1) & (ua << 24));
    result += (-((ub >> 25) & 1) & (ua << 25));
    result += (-((ub >> 26) & 1) & (ua << 26));
    result += (-((ub >> 27) & 1) & (ua << 27));
    result += (-((ub >> 28) & 1) & (ua << 28));
    result += (-((ub >> 29) & 1) & (ua << 29));
    result += (-((ub >> 30) & 1) & (ua << 30));
    result += (-((ub >> 31) & 1) & (ua << 31));

    // Determine sign
    int sign = ((a >> 31) ^ (b >> 31)); // 1 if signs differ
    return (result ^ -sign) + sign;     // Two's complement negation if needed
}
