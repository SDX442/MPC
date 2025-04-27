bool greater_than(int a, int b) {
    int diff = subtract(a, b);
    int sign_bit = (diff >> 31) & 1;
    int neg_diff = ~diff + 1;
    int non_zero = ((diff | neg_diff) >> 31) & 1;
    int result = (~sign_bit) & non_zero;
    return result != 0;
}
