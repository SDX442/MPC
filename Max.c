int max(int a, int b) {
    bool cond = greater_than(a, b);
    return ifelse(a, b, cond);
}
