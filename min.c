int min(int a, int b) {
    bool cond = greater_than(b, a); // b > a → a < b
    return ifelse(cond, a, b);
}
