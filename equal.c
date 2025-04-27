bool equal(int a, int b) {
    int diff = subtract(a, b);
    int neg_diff = ~diff + 1;
    int non_zero = ((diff | neg_diff) >> 31) & 1;
    return non_zero == 0;
}
