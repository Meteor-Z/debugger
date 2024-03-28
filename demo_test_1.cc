
void d() {
    // do something
}
void c() { d(); }
void b() { c(); }
void a() { b(); }
int main() { a(); }