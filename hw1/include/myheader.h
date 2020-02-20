void add_symbol(SYMBOL *rule, SYMBOL *symbol);
int UTF8_to_value(int bytes, int bytec);
void recursive_print(SYMBOL *head, FILE *out, int* countp, SYMBOL *referenced);
int value_to_UTF8(int value);