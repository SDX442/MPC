int absolute(int a) {
    int mask = a >> 31;
    return (a + mask) ^ mask;
}
