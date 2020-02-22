#include <criterion/criterion.h>
#include <criterion/logging.h>
#include "const.h"
#include "myheader.h"

Test(basecode_tests_suite, validargs_help_test) {
    int argc = 2;
    char *argv[] = {"bin/sequitur", "-h", NULL};
    int ret = validargs(argc, argv);
    int exp_ret = 0;
    int opt = global_options;
    int flag = 0x1;
    cr_assert_eq(ret, exp_ret, "Invalid return for valid args.  Got: %d | Expected: %d",
		 ret, exp_ret);
    cr_assert_eq(opt & flag, flag, "Correct bit (0x1) not set for -h. Got: %x", opt);
}

Test(basecode_tests_suite, validargs_compress_test) {
   //  int argc = 4;
   //  char *argv[] = {"bin/sequitur", "-c", "-b", "10", NULL};
   //  int ret = validargs(argc, argv);
   //  int exp_ret = 0;
   //  int opt = global_options;
   //  int flag = 0x2;
   //  int exp_size = 10;
   //  int size = (opt >> 16) & 0xffff;
   //  cr_assert_eq(ret, exp_ret, "Invalid return for valid args.  Got: %d | Expected: %d",
		 // ret, exp_ret);
   //  cr_assert(opt & flag, "Compress mode bit wasn't set. Got: %x", opt);
   //  cr_assert_eq(exp_size, size, "Block size not properly set. Got: %d | Expected: %d",
		 // size, exp_size);
    int argc = 4;
    char *argv[] = {"bin/sequitur", "-c", "-b", "1024", NULL};
    int ret = validargs(argc, argv);
    int exp_ret = 0;
    int opt = global_options;
    int flag = 0x2;
    int exp_size = 1024;
    int size = (opt >> 16) & 0xffff;
    cr_assert_eq(ret, exp_ret, "Invalid return for valid args.  Got: %d | Expected: %d",
         ret, exp_ret);
    cr_assert(opt & flag, "Compress mode bit wasn't set. Got: %x", opt);
    cr_assert_eq(exp_size, size, "Block size not properly set. Got: %d | Expected: %d",
         size, exp_size);
}

Test(basecode_tests_suite, validargs_error_test) {
    int argc = 4;
    char *argv[] = {"bin/sequitur", "-d", "-b", "10", NULL};
    int ret = validargs(argc, argv);
    int exp_ret = -1;
    cr_assert_eq(ret, exp_ret, "Invalid return for valid args.  Got: %d | Expected: %d",
		 ret, exp_ret);
}

Test(basecode_tests_suite, help_system_test) {
    char *cmd = "bin/sequitur -h";

    // system is a syscall defined in stdlib.h
    // it takes a shell command as a string and runs it
    // we use WEXITSTATUS to get the return code from the run
    // use 'man 3 system' to find out more
    int return_code = WEXITSTATUS(system(cmd));

    cr_assert_eq(return_code, EXIT_SUCCESS,
                 "Program exited with %d instead of EXIT_SUCCESS",
		 return_code);
}

Test(basecode_tests_suite, valid_UTF8_test) {
    int byte=0xbf;
    int ret = valid_UTF8(byte);
    int exp_ret = 1;
    cr_assert_eq(ret, exp_ret, "Invalid return for valid_UTF8. Got: %d | Expected: %d", ret, exp_ret);
    byte = 0xff;
    ret = valid_UTF8(byte);
    exp_ret = 0;
    cr_assert_eq(ret, exp_ret, "Invalid return for valid_UTF8. Got: %d | Expected: %d", ret, exp_ret);
}

Test(basecode_tests_suite, UTF8_to_value_test) {
    int bytes = 0xc480;
    int bytec = 2;
    int ret = UTF8_to_value(bytes, bytec);
    int exp_ret = 0x100;
    cr_assert_eq(ret, exp_ret, "Invalid return for UTF8_to_value. Got: %d | Expected: %d", ret, exp_ret);

    bytes = 0xdbb7;
    bytec = 3;
    ret = UTF8_to_value(bytes, bytec);
    exp_ret = 0x6f7;
    cr_assert_eq(ret, exp_ret, "Invalid return for UTF8_to_value. Got: %d | Expected: %d", ret, exp_ret);

    bytes = 0xefb0b0;
    bytec = 3;
    ret = UTF8_to_value(bytes, bytec);
    exp_ret = 0xfc30;
    cr_assert_eq(ret, exp_ret, "Invalid return for UTF8_to_value. Got: %d | Expected: %d", ret, exp_ret);

    bytes = 0xf7b0b0b0;
    bytec = 4;
    ret = UTF8_to_value(bytes, bytec);
    exp_ret = 0x1f0c30;
    cr_assert_eq(ret, exp_ret, "Invalid return for UTF8_to_value. Got: %d | Expected: %d", ret, exp_ret);

    bytes = 0xf7bfbfbf;
    bytec = 4;
    ret = UTF8_to_value(bytes, bytec);
    exp_ret = 0x1fffff;
    cr_assert_eq(ret, exp_ret, "Invalid return for UTF8_to_value. Got: %d | Expected: %d", ret, exp_ret);

}

Test(basecode_tests_suite, value_to_UTF8_test) {
    int value = 0x109;
    int ret = value_to_UTF8(value);
    int exp_ret = 0xc489;
    cr_assert_eq(ret, exp_ret, "Invalid return for value_to_UTF8. Got: %d | Expected: %d", ret, exp_ret);

    value = 0x6f7;
    ret = value_to_UTF8(value);
    exp_ret = 0xdbb7;
    cr_assert_eq(ret, exp_ret, "Invalid return for value_to_UTF8. Got: %d | Expected: %d", ret, exp_ret);

    value = 0xfc30;
    ret = value_to_UTF8(value);
    exp_ret = 0xefb0b0;
    cr_assert_eq(ret, exp_ret, "Invalid return for value_to_UTF8. Got: %x | Expected: %x", ret & 0xffffffff, exp_ret & 0xffffffff);

    value = 0x1f0c30;
    ret = value_to_UTF8(value);
    exp_ret = 0xf7b0b0b0;
    cr_assert_eq(ret, exp_ret, "Invalid return for value_to_UTF8. Got: %x | Expected: %x", ret & 0xffffffff, exp_ret & 0xffffffff);

    value = 0x1fffff;
    ret = value_to_UTF8(value);
    exp_ret = 0xf7bfbfbf;
    cr_assert_eq(ret, exp_ret, "Invalid return for value_to_UTF8. Got: %x | Expected: %x", ret & 0xffffffff, exp_ret & 0xffffffff);
}
