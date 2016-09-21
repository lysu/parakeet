#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include <CUnit/Basic.h>
#include <CUnit/CUnit.h>
#include <CUnit/TestDB.h>

#include "limiter.h"

void test_init_and_destroy_limiter(void) {
    limiter_mtx_init();

    limiter_t *l = calloc(1, sizeof(limiter_t));
    if (limiter_init(l, 10, 1, 10) == -1) {
        fprintf(stderr, "init limiter failure\n");
    }
    fprintf(stderr, "init ok\n");

    int ret = limiter_allow(l, getpid());

    CU_ASSERT_TRUE(ret);

    limiter_destroy(l);
    free(l);

    CU_ASSERT_TRUE(1);
}

CU_TestInfo limiterTestCase[] = {
        {"test_for_lenth:", test_init_and_destroy_limiter},
        CU_TEST_INFO_NULL
};

int suite_success_init(void) {
    return 0;
}

int suite_success_clean(void) {
    return 0;
}

CU_SuiteInfo suites[] = {
        {"limiterTestSuit", NULL, NULL, NULL, NULL, limiterTestCase},
        CU_SUITE_INFO_NULL
};

void add_tests() {
    assert(NULL != CU_get_registry());
    assert(!CU_is_test_running());

    if (CUE_SUCCESS != CU_register_suites(suites)) {
        exit(EXIT_FAILURE);
    }
}

int run_test(void) {
    if (CU_initialize_registry()) {
        fprintf(stderr, " Initialization of Test Registry failed. ");
        exit(EXIT_FAILURE);
    }
    add_tests();
    CU_basic_set_mode(CU_BRM_VERBOSE);
    CU_basic_run_tests();
    CU_cleanup_registry();
    return CU_get_error();
}

int main(void) {
    return run_test();
}
