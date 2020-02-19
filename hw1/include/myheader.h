void add_symbol(SYMBOL *rule, SYMBOL *symbol);
int toUTF8(int bytes, int bytec);
void recursive_print(SYMBOL *head, FILE *out, int* countp, SYMBOL *referenced);