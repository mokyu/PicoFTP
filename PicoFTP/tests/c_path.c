/*
 * File:   c_path.c
 * Author: parallels
 *
 * Created on Jan 6, 2019, 8:38:09 PM
 */

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <CUnit/Basic.h>
#include "../path.h"

/*
 * CUnit Test Suite
 */

int init_suite(void) {
    return 0;
}

int clean_suite(void) {
    return 0;
}

void test1_toString() {
    path_t* dira = malloc(sizeof (path_t));
    path_t* dirb = malloc(sizeof (path_t));
    path_t* dirc = malloc(sizeof (path_t));
    dira->up = NULL;
    dira->down = dirb;
    snprintf(dira->name, 32, "a");
    dirb->up = dira;
    dirb->down = dirc;
    dirb->rootFolder = 1;
    snprintf(dirb->name, 32, "b");
    dirc->up = dirb;
    dirc->down = NULL;
    snprintf(dirc->name, 32, "c");
    char buffer[PATH_MAX];
    // test with middle segment dirb
    path_toString(dirb, buffer, BACKWARD);
    CU_ASSERT(strcmp(buffer, "/a/b") == 0);
    path_toString(dirb, buffer, FORWARD);
    CU_ASSERT(strcmp(buffer, "/b/c") == 0);
    path_toString(dirb, buffer, COMPLETE);
    CU_ASSERT(strcmp(buffer, "/a/b/c") == 0);

    // test with begin segment dira
    path_toString(dira, buffer, BACKWARD);
    CU_ASSERT(strcmp(buffer, "/a") == 0);
    path_toString(dira, buffer, FORWARD);
    CU_ASSERT(strcmp(buffer, "/a/b/c") == 0);
    path_toString(dira, buffer, COMPLETE);
    CU_ASSERT(strcmp(buffer, "/a/b/c") == 0);

    // test with end segment dirc
    path_toString(dirc, buffer, BACKWARD);
    CU_ASSERT(strcmp(buffer, "/a/b/c") == 0);
    path_toString(dirc, buffer, FORWARD);
    CU_ASSERT(strcmp(buffer, "/c") == 0);
    path_toString(dirc, buffer, COMPLETE);
    CU_ASSERT(strcmp(buffer, "/a/b/c") == 0);

    // test directory rooting;
    path_toString(dirc, buffer, ROOTED);
    CU_ASSERT(strcmp(buffer, "/c") == 0);
    dirb->down = NULL;
    path_toString(dirc, buffer, ROOTED);
    CU_ASSERT(strcmp(buffer, "/") == 0);
    free(dirc);
    free(dirb);
    free(dira);
}

void test2() {
    path_t* path = NULL;
    char buffer[PATH_MAX];
    char* input = malloc(255);
    snprintf(input, 254, "/var/lib/apt");
    path = path_build(input);
    CU_ASSERT(path != NULL);
    path_toString(path, buffer, COMPLETE);
    CU_ASSERT(strcmp(buffer, "/var/lib/apt") == 0);
    path_free(path);

    snprintf(input, 254, "/fake path/that_does/.not_exist");
    path = path_build(input);
    CU_ASSERT(path == NULL);
}

void test3() {
    path_t* path = NULL;
    char* input = malloc(255);
    snprintf(input, 254, "/var/lib");
    path = path_build(input);
    char buffer[PATH_MAX];
    // absolute path test from /var/lib (root) to /var/lib/apt/keyrings
    CU_ASSERT(path_cwd(path, "/apt/keyrings"));
    path_toString(path, buffer, COMPLETE);
    CU_ASSERT(strcmp(buffer, "/var/lib/apt/keyrings") == 0);

    // absolute path test from /var/lib/apt/keyrings to /var/lib
    CU_ASSERT(path_cwd(path, "/"));
    path_toString(path, buffer, COMPLETE);
    CU_ASSERT(strcmp(buffer, "/var/lib") == 0);

    // test going up 1 directory relative to /var/lib/apt/keyrings
    path_cwd(path, "/apt/keyrings");
    CU_ASSERT(path_cwd(path, "../lists"));
    path_toString(path, buffer, COMPLETE);
    CU_ASSERT(strcmp(buffer, "/var/lib/apt/lists") == 0);

    // test going up 1 directories relative to /var/lib/apt/keyrings without folder
    path_cwd(path, "/apt/keyrings");
    CU_ASSERT(path_cwd(path, ".."));
    path_toString(path, buffer, COMPLETE);
    CU_ASSERT(strcmp(buffer, "/var/lib/apt") == 0);


    // test going up too far down the dir tree past our ftp root
    path_cwd(path, "/");
    CU_ASSERT(path_cwd(path, "../../home/parallels") == 0);
    path_toString(path, buffer, COMPLETE);
    CU_ASSERT(strcmp(buffer, "/var/lib") == 0);

    // test relative invalid folder from /var/lib/apt/keyrings
    path_cwd(path, "/apt/keyrings");
    CU_ASSERT(path_cwd(path, "../invalid folder that totally does not exist") == 0);
    path_toString(path, buffer, COMPLETE);
    CU_ASSERT(strcmp(buffer, "/var/lib/apt/keyrings") == 0);

    // test entering directory by just it's name
    path_cwd(path, "/apt");
    path_cwd(path, "keyrings");
    path_toString(path, buffer, COMPLETE);
    CU_ASSERT(strcmp(buffer, "/var/lib/apt/keyrings") == 0);
    printf("\npath: %s\n", buffer);
}

void test4() {
    path_t* source = path_build("/var/lib");
    path_t* t1 = path_build("/var/lib/apt");
    path_t* t2 = path_build("/var/lib");
    path_t* t3 = path_build("/var/opt");
    CU_ASSERT(path_verify(source, t1) == 1);
    CU_ASSERT(path_verify(source, t2) == 1);
    CU_ASSERT(path_verify(source, t3) == 0);

}

int main() {
    CU_pSuite pSuite = NULL;

    /* Initialize the CUnit test registry */
    if (CUE_SUCCESS != CU_initialize_registry())
        return CU_get_error();

    /* Add a suite to the registry */
    pSuite = CU_add_suite("c_path", init_suite, clean_suite);
    if (NULL == pSuite) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    /* Add the tests to the suite */
    if ((NULL == CU_add_test(pSuite, "path to string", test1_toString)) ||
            (NULL == CU_add_test(pSuite, "Path creation", test2)) ||
            (NULL == CU_add_test(pSuite, "Path manipulation", test3)) ||
            (NULL == CU_add_test(pSuite, "Path comparison", test4))) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    /* Run all tests using the CUnit Basic interface */
    CU_basic_set_mode(CU_BRM_VERBOSE);
    CU_basic_run_tests();
    CU_cleanup_registry();
    return CU_get_error();
}
