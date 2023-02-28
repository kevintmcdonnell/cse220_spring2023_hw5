/*
DO NOT CHANGE THE CONTENTS OF THIS FILE IN CASE A NEW VERSION IS DISTRIBUTED.
PUT YOUR OWN TEST CASES IN student_tests.c
*/

#include "unit_tests.h"
 
static char test_log_outfile[100];

int run_using_system(char *test_name) {
    char executable[100];
    sprintf(executable, "./bin/%s", test_name);
    assert(access(executable, F_OK) == 0);

    char cmd[500];
    sprintf(test_log_outfile, "%s/%s.log", TEST_OUTPUT_DIR, test_name);
#if defined(__linux__)
    sprintf(cmd, "valgrind -s --leak-check=full --show-leak-kinds=all --track-origins=yes --trace-children=yes --error-exitcode=37 ./bin/%s > %s 2>&1",
	    test_name, test_log_outfile);
#else
    cr_log_warn("Skipping valgrind tests. Run tests on Linux or GitHub for full output.\n");
    sprintf(cmd, "./bin/%s > %s 2>&1", test_name, test_log_outfile);
#endif
    return system(cmd);
}

int run_using_system_no_valgrind(char *test_name) {
    char executable[100];
    sprintf(executable, "./bin/%s", test_name);
    assert(access(executable, F_OK) == 0);

    char cmd[500];
    sprintf(test_log_outfile, "%s/%s.log", TEST_OUTPUT_DIR, test_name);
    sprintf(cmd, "./bin/%s > %s 2>&1", test_name, test_log_outfile);
    return system(cmd);
}

void expect_outfile_matches(char *name) {
    char cmd[500];
    sprintf(cmd, "diff %s/%s.txt %s ", TEST_INPUT_DIR, name, test_log_outfile);
    int err = system(cmd);
    cr_expect_eq(err, 0, "The output was not what was expected (diff exited with status %d).\n", WEXITSTATUS(err));
}

void expect_normal_exit(int status) {
    cr_expect_eq(status, 0, "The program did not exit normally (status = 0x%x).\n", status);
}

void expect_error_exit(int status) {
    cr_expect_eq(WEXITSTATUS(status), 0xff,
		 "The program did not exit with status 0xff (status was 0x%x).\n", status);
}

void expect_no_valgrind_errors(int status) {
    cr_expect_neq(WEXITSTATUS(status), 37, "Valgrind reported errors -- see %s", test_log_outfile);
    if (WEXITSTATUS(status) == 37) {
        char cmd[200];
        sprintf(cmd, "cat %s", test_log_outfile);
        system(cmd);
    }
}

void print_char_array(char *chars, int chars_len) {
    for (int i = 0; i < chars_len; i++) {
        if(isprint(chars[i]))
            printf("%c", chars[i]);
        else
            printf("\\x%02x", chars[i]);
    }
}

TestSuite(base_output, .timeout=TEST_TIMEOUT, .disabled=false);
TestSuite(base_return, .timeout=TEST_TIMEOUT, .disabled=false);
TestSuite(base_valgrind, .timeout=TEST_TIMEOUT, .disabled=false);

Test(base_output, print_packet01, .description="Print the contents of a packet") {
    char *test_name = "print_packet01";
    run_using_system_no_valgrind(test_name); 
    expect_outfile_matches(test_name);
}
Test(base_valgrind, print_packet01) {
    expect_no_valgrind_errors(run_using_system("print_packet01"));
}

Test(base_return, checksum01, .description="Compute a checksum") {
    unsigned char packet[] = "\x00\x00\x00\x30\x39\x00\x00\x01\x09\x3b\x20\x40\x00\x00\x00\x10\x00\x00\x00\x1d\x00\xa5\x25\x17\x41\x42\x43\x44\x45RANDOM GARBAGE YOU SHOULD NOT SEE THIS";
    unsigned int actual = checksum_sf(packet);
    unsigned int expected = 0x149f1;
    cr_expect_eq(actual, expected, "The return value was 0x%x, but it should have been 0x%x.\n", actual, expected);
}

Test(base_output, reconstruct01, .description="Function given more than enough memory to reconstruct the message.") {
    char *packets[] = {
        "\x38\x91\x8a\x27\x31\x1e\x82\xca\xb0\x29\x20\x40\x00\x00\x0c\x03\x1f\x00\x00\x24\x14\x54\xdb\x60\x6f\x20\x77\x61\x79\x73\x20\x74\x6f\x20\x77\x72",
        "\x38\x91\x8a\x27\x31\x1e\x82\xca\xb0\x29\x20\x40\x00\x00\x30\x03\x1f\x00\x00\x24\x14\x54\xdb\x84\x20\x6f\x6e\x6c\x79\x20\x74\x68\x65\x20\x74\x68",
        "\x38\x91\x8a\x27\x31\x1e\x82\xca\xb0\x29\x20\x40\x00\x00\x3c\x03\x1f\x00\x00\x24\x14\x54\xdb\x90\x69\x72\x64\x20\x6f\x6e\x65\x20\x77\x6f\x72\x6b",
        "\x38\x91\x8a\x27\x31\x1e\x82\xca\xb0\x29\x20\x40\x00\x00\x18\x03\x1f\x00\x00\x24\x14\x54\xdb\x6c\x69\x74\x65\x20\x65\x72\x72\x6f\x72\x2d\x66\x72",
        "\x38\x91\x8a\x27\x31\x1e\x82\xca\xb0\x29\x20\x40\x00\x00\x00\x03\x1f\x00\x00\x24\x14\x54\xdb\x54\x54\x68\x65\x72\x65\x20\x61\x72\x65\x20\x74\x77",
        "\x38\x91\x8a\x27\x31\x1e\x82\xca\xb0\x29\x20\x40\x00\x00\x24\x03\x1f\x00\x00\x24\x14\x54\xdb\x78\x65\x65\x20\x70\x72\x6f\x67\x72\x61\x6d\x73\x3b",
        "\x38\x91\x8a\x27\x31\x1e\x82\xca\xb0\x29\x20\x40\x00\x00\x54\x03\x1f\x00\x00\x1f\x14\x54\xdb\xa3\x20\x50\x65\x72\x6c\x69\x73",
        "\x38\x91\x8a\x27\x31\x1e\x82\xca\xb0\x29\x20\x40\x00\x00\x48\x03\x1f\x00\x00\x24\x14\x54\xdb\x9c\x73\x2e\x20\x2d\x20\x41\x6c\x61\x6e\x20\x4a\x2e",
    };
    unsigned int packets_len = 8;
    unsigned int message_len = 120; // original message has 91 characters
    char *message_act = malloc(message_len*sizeof(char));

    // fill memory with "random" garbage
    srand(2513);
    for (unsigned int i = 0; i < message_len; i++)
        message_act[i] = (char)(rand() % 200 + 33);

    reconstruct_sf((unsigned char **)packets, packets_len, message_act, message_len);
    char *message_exp = "There are two ways to write error-free programs; only the third one works. - Alan J. Perlis";
    cr_expect_arr_eq(message_act, message_exp, strlen(message_exp)+1,
            "Contents of message incorrect.\nActual:   %s\nExpected: %s", message_act, message_exp);
    free(message_act);
}
Test(base_return, reconstruct01, .description="Function given more than enough memory to reconstruct the message.") {
    char *packets[] = {
        "\x38\x91\x8a\x27\x31\x1e\x82\xca\xb0\x29\x20\x40\x00\x00\x0c\x03\x1f\x00\x00\x24\x14\x54\xdb\x60\x6f\x20\x77\x61\x79\x73\x20\x74\x6f\x20\x77\x72",
        "\x38\x91\x8a\x27\x31\x1e\x82\xca\xb0\x29\x20\x40\x00\x00\x30\x03\x1f\x00\x00\x24\x14\x54\xdb\x84\x20\x6f\x6e\x6c\x79\x20\x74\x68\x65\x20\x74\x68",
        "\x38\x91\x8a\x27\x31\x1e\x82\xca\xb0\x29\x20\x40\x00\x00\x3c\x03\x1f\x00\x00\x24\x14\x54\xdb\x90\x69\x72\x64\x20\x6f\x6e\x65\x20\x77\x6f\x72\x6b",
        "\x38\x91\x8a\x27\x31\x1e\x82\xca\xb0\x29\x20\x40\x00\x00\x18\x03\x1f\x00\x00\x24\x14\x54\xdb\x6c\x69\x74\x65\x20\x65\x72\x72\x6f\x72\x2d\x66\x72",
        "\x38\x91\x8a\x27\x31\x1e\x82\xca\xb0\x29\x20\x40\x00\x00\x00\x03\x1f\x00\x00\x24\x14\x54\xdb\x54\x54\x68\x65\x72\x65\x20\x61\x72\x65\x20\x74\x77",
        "\x38\x91\x8a\x27\x31\x1e\x82\xca\xb0\x29\x20\x40\x00\x00\x24\x03\x1f\x00\x00\x24\x14\x54\xdb\x78\x65\x65\x20\x70\x72\x6f\x67\x72\x61\x6d\x73\x3b",
        "\x38\x91\x8a\x27\x31\x1e\x82\xca\xb0\x29\x20\x40\x00\x00\x54\x03\x1f\x00\x00\x1f\x14\x54\xdb\xa3\x20\x50\x65\x72\x6c\x69\x73",
        "\x38\x91\x8a\x27\x31\x1e\x82\xca\xb0\x29\x20\x40\x00\x00\x48\x03\x1f\x00\x00\x24\x14\x54\xdb\x9c\x73\x2e\x20\x2d\x20\x41\x6c\x61\x6e\x20\x4a\x2e",
    };
    unsigned int packets_len = 8;
    unsigned int message_len = 120; // original message has 91 characters
    char *message_act = malloc(message_len*sizeof(char));

    // fill memory with "random" garbage
    srand(2513);
    for (unsigned int i = 0; i < message_len; i++)
        message_act[i] = (char)(rand() % 200 + 33);

    unsigned int num_packets_act = reconstruct_sf((unsigned char **)packets, packets_len, message_act, message_len);
    unsigned int num_packets_exp = 8;
    cr_expect_eq(num_packets_act, num_packets_exp, "The return value was %d, but it should have been %d.\n", num_packets_act, num_packets_exp);
    free(message_act);
}
Test(base_valgrind, reconstruct01_valgrind) {
    expect_no_valgrind_errors(run_using_system("reconstruct01"));
}

Test(base_output, reconstruct02, .description="Function not given enough memory to reconstruct the message.") {
    char *packets[] = {
        "\x38\x91\x8a\x27\x31\x1e\x82\xca\xb0\x29\x20\x40\x00\x00\x0c\x03\x1f\x00\x00\x24\x14\x54\xdb\x60\x6f\x20\x77\x61\x79\x73\x20\x74\x6f\x20\x77\x72",
        "\x38\x91\x8a\x27\x31\x1e\x82\xca\xb0\x29\x20\x40\x00\x00\x30\x03\x1f\x00\x00\x24\x14\x54\xdb\x84\x20\x6f\x6e\x6c\x79\x20\x74\x68\x65\x20\x74\x68",
        "\x38\x91\x8a\x27\x31\x1e\x82\xca\xb0\x29\x20\x40\x00\x00\x00\x03\x1f\x00\x00\x24\x14\x54\xdb\x54\x54\x68\x65\x72\x65\x20\x61\x72\x65\x20\x74\x77",
        "\x38\x91\x8a\x27\x31\x1e\x82\xca\xb0\x29\x20\x40\x00\x00\x54\x03\x1f\x00\x00\x1f\x14\x54\xdb\xa3\x20\x50\x65\x72\x6c\x69\x73",
        "\x38\x91\x8a\x27\x31\x1e\x82\xca\xb0\x29\x20\x40\x00\x00\x18\x03\x1f\x00\x00\x24\x14\x54\xdb\x6c\x69\x74\x65\x20\x65\x72\x72\x6f\x72\x2d\x66\x72",
        "\x38\x91\x8a\x27\x31\x1e\x82\xca\xb0\x29\x20\x40\x00\x00\x24\x03\x1f\x00\x00\x24\x14\x54\xdb\x78\x65\x65\x20\x70\x72\x6f\x67\x72\x61\x6d\x73\x3b",
        "\x38\x91\x8a\x27\x31\x1e\x82\xca\xb0\x29\x20\x40\x00\x00\x48\x03\x1f\x00\x00\x24\x14\x54\xdb\x9c\x73\x2e\x20\x2d\x20\x41\x6c\x61\x6e\x20\x4a\x2e",
        "\x38\x91\x8a\x27\x31\x1e\x82\xca\xb0\x29\x20\x40\x00\x00\x3c\x03\x1f\x00\x00\x24\x14\x54\xdb\x90\x69\x72\x64\x20\x6f\x6e\x65\x20\x77\x6f\x72\x6b",
    };
    unsigned int packets_len = 8;
    unsigned int message_len = 60;  // original message has 91 characters
    char *message_act = malloc(message_len*sizeof(char));

    // fill memory with "random" garbage
    srand(444);
    for (unsigned int i = 0; i < message_len; i++)
        message_act[i] = (char)(rand() % 200 + 33);

    reconstruct_sf((unsigned char **)packets, packets_len, message_act, message_len);
    char *message_exp = "There are two ways to write error-free programs; only the t";
    cr_expect_arr_eq(message_act, message_exp, strlen(message_exp)+1,
            "Contents of message incorrect.\nActual:   %s\nExpected: %s", message_act, message_exp);
    free(message_act);
}
Test(base_return, reconstruct02, .description="Function not given enough memory to reconstruct the message.") {
    char *packets[] = {
        "\x38\x91\x8a\x27\x31\x1e\x82\xca\xb0\x29\x20\x40\x00\x00\x0c\x03\x1f\x00\x00\x24\x14\x54\xdb\x60\x6f\x20\x77\x61\x79\x73\x20\x74\x6f\x20\x77\x72",
        "\x38\x91\x8a\x27\x31\x1e\x82\xca\xb0\x29\x20\x40\x00\x00\x30\x03\x1f\x00\x00\x24\x14\x54\xdb\x84\x20\x6f\x6e\x6c\x79\x20\x74\x68\x65\x20\x74\x68",
        "\x38\x91\x8a\x27\x31\x1e\x82\xca\xb0\x29\x20\x40\x00\x00\x00\x03\x1f\x00\x00\x24\x14\x54\xdb\x54\x54\x68\x65\x72\x65\x20\x61\x72\x65\x20\x74\x77",
        "\x38\x91\x8a\x27\x31\x1e\x82\xca\xb0\x29\x20\x40\x00\x00\x54\x03\x1f\x00\x00\x1f\x14\x54\xdb\xa3\x20\x50\x65\x72\x6c\x69\x73",
        "\x38\x91\x8a\x27\x31\x1e\x82\xca\xb0\x29\x20\x40\x00\x00\x18\x03\x1f\x00\x00\x24\x14\x54\xdb\x6c\x69\x74\x65\x20\x65\x72\x72\x6f\x72\x2d\x66\x72",
        "\x38\x91\x8a\x27\x31\x1e\x82\xca\xb0\x29\x20\x40\x00\x00\x24\x03\x1f\x00\x00\x24\x14\x54\xdb\x78\x65\x65\x20\x70\x72\x6f\x67\x72\x61\x6d\x73\x3b",
        "\x38\x91\x8a\x27\x31\x1e\x82\xca\xb0\x29\x20\x40\x00\x00\x48\x03\x1f\x00\x00\x24\x14\x54\xdb\x9c\x73\x2e\x20\x2d\x20\x41\x6c\x61\x6e\x20\x4a\x2e",
        "\x38\x91\x8a\x27\x31\x1e\x82\xca\xb0\x29\x20\x40\x00\x00\x3c\x03\x1f\x00\x00\x24\x14\x54\xdb\x90\x69\x72\x64\x20\x6f\x6e\x65\x20\x77\x6f\x72\x6b",
    };
    unsigned int packets_len = 8;
    unsigned int message_len = 60;  // original message has 91 characters
    char *message_act = malloc(message_len*sizeof(char));

    // fill memory with "random" garbage
    srand(444);
    for (unsigned int i = 0; i < message_len; i++)
        message_act[i] = (char)(rand() % 200 + 33);

    unsigned int num_packets_act = reconstruct_sf((unsigned char **)packets, packets_len, message_act, message_len);
    unsigned int num_packets_exp = 5;
    cr_expect_eq(num_packets_act, num_packets_exp, "The return value was %d, but it should have been %d.\n", num_packets_act, num_packets_exp);
    free(message_act);
}
Test(base_valgrind, reconstruct02_valgrind) {
    expect_no_valgrind_errors(run_using_system("reconstruct02"));
}

Test(base_output, reconstruct03, .description="Function given more memory than needed, but some packets are corrupted.") {
    char *packets[] = {
        "\x38\x91\x8a\x27\x31\x1e\x82\xca\xb0\x29\x20\x40\x00\x00\x3c\x03\x1f\x00\x00\x24\x14\x54\xdb\x90\x69\x72\x64\x20\x6f\x6e\x65\x20\x77\x6f\x72\x6b",
        "\x38\x91\x8a\x27\x31\x1e\x82\xca\xb0\x29\x20\x40\x00\x00\x48\x03\x1f\x01\x00\x24\x14\x54\xdb\x9c\x73\x2e\x20\x2d\x20\x41\x6c\x61\x6e\x20\x4a\x2e",
        "\x38\x91\x8a\x27\x31\x1e\x82\xca\xb0\x29\x20\x40\x00\x00\x00\x03\x1f\x00\x00\x24\x14\x5c\xdb\x54\x54\x68\x65\x72\x65\x20\x61\x72\x65\x20\x74\x77",
        "\x38\x91\x8a\x27\x31\x1e\x82\xca\xb0\x29\x30\x40\x00\x00\x18\x03\x1f\x00\x00\x24\x14\x54\xdb\x6c\x69\x74\x65\x20\x65\x72\x72\x6f\x72\x2d\x66\x72",
        "\x38\x91\x8a\x27\x31\x1e\x82\xca\xb0\x29\x20\x40\x00\x00\x24\x03\x1f\x00\x00\x24\x14\x54\xdb\x78\x65\x65\x20\x70\x72\x6f\x67\x72\x61\x6d\x73\x3b",
        "\x38\x91\x8a\x27\x31\x1e\x82\xca\xb0\x29\x20\x40\x00\x00\x0c\x03\x1f\x00\x00\x24\x14\x54\xdb\x60\x6f\x20\x77\x61\x79\x73\x20\x74\x6f\x20\x77\x72",
        "\x38\x91\x8a\x27\x31\x1e\x82\xca\xb0\x29\x20\x40\x00\x00\x30\x03\x1f\x00\x00\x24\x14\x54\xdb\x84\x20\x6f\x6e\x6c\x79\x20\x74\x68\x65\x20\x74\x68",
        "\x38\x91\x8a\x27\x31\x1e\x82\xca\xb0\x29\x20\x40\x00\x00\x54\x03\x1f\x00\x00\x1f\x14\x54\xdb\xa3\x20\x50\x65\x72\x6c\x69\x73",
    };
    unsigned int packets_len = 8;
    unsigned int message_len = 120;  // original message has 91 characters
    char *message_act = malloc(message_len*sizeof(char));

    // fill memory with "random" garbage
    for (unsigned int i = 0; i < message_len; i++)
        message_act[i] = '@';

    reconstruct_sf((unsigned char **)packets, packets_len, message_act, message_len);
    char *message_exp = "@@@@@@@@@@@@o ways to wr@@@@@@@@@@@@ee programs; only the third one work@@@@@@@@@@@@ Perlis";
    cr_expect_arr_eq(message_act, message_exp, strlen(message_exp)+1,
            "Contents of message incorrect.\nActual:   %s\nExpected: %s", message_act, message_exp);
    free(message_act);
}
Test(base_return, reconstruct03, .description="Function given more memory than needed, but some packets are corrupted.") {
    char *packets[] = {
        "\x38\x91\x8a\x27\x31\x1e\x82\xca\xb0\x29\x20\x40\x00\x00\x3c\x03\x1f\x00\x00\x24\x14\x54\xdb\x90\x69\x72\x64\x20\x6f\x6e\x65\x20\x77\x6f\x72\x6b",
        "\x38\x91\x8a\x27\x31\x1e\x82\xca\xb0\x29\x20\x40\x00\x00\x48\x03\x1f\x01\x00\x24\x14\x54\xdb\x9c\x73\x2e\x20\x2d\x20\x41\x6c\x61\x6e\x20\x4a\x2e",
        "\x38\x91\x8a\x27\x31\x1e\x82\xca\xb0\x29\x20\x40\x00\x00\x00\x03\x1f\x00\x00\x24\x14\x5c\xdb\x54\x54\x68\x65\x72\x65\x20\x61\x72\x65\x20\x74\x77",
        "\x38\x91\x8a\x27\x31\x1e\x82\xca\xb0\x29\x30\x40\x00\x00\x18\x03\x1f\x00\x00\x24\x14\x54\xdb\x6c\x69\x74\x65\x20\x65\x72\x72\x6f\x72\x2d\x66\x72",
        "\x38\x91\x8a\x27\x31\x1e\x82\xca\xb0\x29\x20\x40\x00\x00\x24\x03\x1f\x00\x00\x24\x14\x54\xdb\x78\x65\x65\x20\x70\x72\x6f\x67\x72\x61\x6d\x73\x3b",
        "\x38\x91\x8a\x27\x31\x1e\x82\xca\xb0\x29\x20\x40\x00\x00\x0c\x03\x1f\x00\x00\x24\x14\x54\xdb\x60\x6f\x20\x77\x61\x79\x73\x20\x74\x6f\x20\x77\x72",
        "\x38\x91\x8a\x27\x31\x1e\x82\xca\xb0\x29\x20\x40\x00\x00\x30\x03\x1f\x00\x00\x24\x14\x54\xdb\x84\x20\x6f\x6e\x6c\x79\x20\x74\x68\x65\x20\x74\x68",
        "\x38\x91\x8a\x27\x31\x1e\x82\xca\xb0\x29\x20\x40\x00\x00\x54\x03\x1f\x00\x00\x1f\x14\x54\xdb\xa3\x20\x50\x65\x72\x6c\x69\x73",
    };
    unsigned int packets_len = 8;
    unsigned int message_len = 120;  // original message has 91 characters
    char *message_act = malloc(message_len*sizeof(char));

    // fill memory with "random" garbage
    for (unsigned int i = 0; i < message_len; i++)
        message_act[i] = '@';

    unsigned int num_packets_act = reconstruct_sf((unsigned char **)packets, packets_len, message_act, message_len);
    unsigned int num_packets_exp = 5;
    cr_expect_eq(num_packets_act, num_packets_exp, "The return value was %d, but it should have been %d.\n", num_packets_act, num_packets_exp);
    free(message_act);
}
Test(base_valgrind, reconstruct03_valgrind) {
    expect_no_valgrind_errors(run_using_system("reconstruct03"));
}

Test(base_output, packetize01, .description="Packetize a short message. packets[] is the optimal length.") {
    char msg[] = "ABCDEFGHIJKL";
    unsigned char *packets_act[3];
    unsigned int packets_len = 3;
    unsigned int max_payload = 5;
    unsigned long src_addr = 12345;
    unsigned long dest_addr = 67899;
    unsigned short encryption = 4096;

    char *packets_exp[] = {
        "\x00\x00\x00\x30\x39\x00\x00\x01\x09\x3b\x20\x40\x00\x00\x00\x10\x00\x00\x00\x1d\x00\x01\x49\xf1\x41\x42\x43\x44\x45",
        "\x00\x00\x00\x30\x39\x00\x00\x01\x09\x3b\x20\x40\x00\x00\x05\x10\x00\x00\x00\x1d\x00\x01\x49\xf6\x46\x47\x48\x49\x4a",
        "\x00\x00\x00\x30\x39\x00\x00\x01\x09\x3b\x20\x40\x00\x00\x0a\x10\x00\x00\x00\x1a\x00\x01\x49\xf8\x4b\x4c",
    };
    unsigned int packet_lens_exp[] = {29, 29, 26};
    packetize_sf(msg, packets_act, packets_len, max_payload, src_addr, dest_addr, encryption);
    unsigned int num_packets_exp = 3;
    for (unsigned int i = 0; i < num_packets_exp; i++) {
        cr_expect_arr_eq(packets_act[i], packets_exp[i], packet_lens_exp[i], 
            "Contents of packet #%d incorrect. See unit_tests.c for expected packet contents.", i);
    }
}
Test(base_return, packetize01, .description="Packetize a short message. packets[] is the optimal length.") {
    char msg[] = "ABCDEFGHIJKL";
    unsigned char *packets_act[3];
    unsigned int packets_len = 3;
    unsigned int max_payload = 5;
    unsigned long src_addr = 12345;
    unsigned long dest_addr = 67899;
    unsigned short encryption = 4096;
    unsigned int num_packets_act = packetize_sf(msg, packets_act, packets_len, max_payload, src_addr, dest_addr, encryption);
    unsigned int num_packets_exp = 3;
    cr_expect_eq(num_packets_act, num_packets_exp, "The return value was %d, but it should have been %d.\n", num_packets_act, num_packets_exp);
}
Test(base_valgrind, packetize01_valgrind) {
    expect_no_valgrind_errors(run_using_system("packetize01"));
}

Test(base_output, packetize02, .description="Packetize a message that is too large to completely packetize.") {
    char msg[] = "The sight of a dozen patients sitting in the waiting room, each wearing a pained expression, did nothing to lift Harry's mood.";
    unsigned char *packets_act[4];
    unsigned int packets_len = 4;
    unsigned int max_payload = 17;
    unsigned long src_addr = 5000000000;
    unsigned long dest_addr = 16500000000;
    unsigned short encryption = 265;

    char *packets_exp[] = {
       "\x01\x2a\x05\xf2\x00\x03\xd7\x7a\x05\x00\x20\x40\x00\x00\x00\x01\x09\x00\x00\x29\x01\x7f\xf8\x97\x54\x68\x65\x20\x73\x69\x67\x68\x74\x20\x6f\x66\x20\x61\x20\x64\x6f",
        "\x01\x2a\x05\xf2\x00\x03\xd7\x7a\x05\x00\x20\x40\x00\x00\x11\x01\x09\x00\x00\x29\x01\x7f\xf8\xa8\x7a\x65\x6e\x20\x70\x61\x74\x69\x65\x6e\x74\x73\x20\x73\x69\x74\x74",
        "\x01\x2a\x05\xf2\x00\x03\xd7\x7a\x05\x00\x20\x40\x00\x00\x22\x01\x09\x00\x00\x29\x01\x7f\xf8\xb9\x69\x6e\x67\x20\x69\x6e\x20\x74\x68\x65\x20\x77\x61\x69\x74\x69\x6e",
        "\x01\x2a\x05\xf2\x00\x03\xd7\x7a\x05\x00\x20\x40\x00\x00\x33\x01\x09\x00\x00\x29\x01\x7f\xf8\xca\x67\x20\x72\x6f\x6f\x6d\x2c\x20\x65\x61\x63\x68\x20\x77\x65\x61\x72",
    };
    unsigned int packet_lens_exp[] = {41, 41, 41, 41};
    packetize_sf(msg, packets_act, packets_len, max_payload, src_addr, dest_addr, encryption);
    unsigned int num_packets_exp = 4;
    for (unsigned int i = 0; i < num_packets_exp; i++) {
        cr_expect_arr_eq(packets_act[i], packets_exp[i], packet_lens_exp[i], 
            "Contents of packet #%d incorrect. See unit_tests.c for expected packet contents.", i);
    }
}
Test(base_return, packetize02, .description="Packetize a message that is too large to completely packetize.") {
    char msg[] = "The sight of a dozen patients sitting in the waiting room, each wearing a pained expression, did nothing to lift Harry's mood.";
    unsigned char *packets_act[4];
    unsigned int packets_len = 4;
    unsigned int max_payload = 17;
    unsigned long src_addr = 5000000000;
    unsigned long dest_addr = 16500000000;
    unsigned short encryption = 265;
    unsigned int num_packets_act = packetize_sf(msg, packets_act, packets_len, max_payload, src_addr, dest_addr, encryption);
    unsigned int num_packets_exp = 4;
    cr_expect_eq(num_packets_act, num_packets_exp, "The return value was %d, but it should have been %d.\n", num_packets_act, num_packets_exp);
}
Test(base_valgrind, packetize02_valgrind) {
    expect_no_valgrind_errors(run_using_system("packetize02"));
}

