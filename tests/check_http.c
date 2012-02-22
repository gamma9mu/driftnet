#include <stdlib.h>
#include <check.h>

/* TODO move these out of media.c into their own header file */
unsigned char *find_http_req(const unsigned char *data, const size_t len, unsigned char **http, size_t *httplen);
void dispatch_http_req(const char *mname, const unsigned char *data, const size_t len);


START_TEST (tooSmall)
{
  const unsigned char *inputData = "data";
  fail_unless(find_http_req(inputData, 12, NULL, NULL) == inputData, NULL);
}
END_TEST

START_TEST (noGETorPOST)
{
  const unsigned char *inputData = "datadatadatadatadatadatadatadatadatadatadatadatadatadatadatadatadatadatadatadatadatadatadatadatadatadatadatadatadatadata";
  fail_unless(find_http_req(inputData, 120, NULL, NULL) == (inputData + 120 - 4), NULL);
}
END_TEST


Suite *http_suite (void) {
  Suite *s = suite_create("HTTP");

  /* Core test case */
  TCase *tc_core = tcase_create("Core");
  tcase_add_test(tc_core, tooSmall);
  tcase_add_test(tc_core, noGETorPOST);
  suite_add_tcase(s, tc_core);

  return s;
}

int main (void) {
  int number_failed;
  Suite *s = http_suite();
  SRunner *sr = srunner_create(s);
  srunner_run_all(sr, CK_VERBOSE);
  number_failed = srunner_ntests_failed(sr);
  srunner_free(sr);
  return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
