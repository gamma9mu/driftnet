#include <stdlib.h>
#include <check.h>

/* TODO move these out of media.c into their own header file */
unsigned char *find_http_req(const unsigned char *data, const size_t len, unsigned char **http, size_t *httplen);
void dispatch_http_req(const char *mname, const unsigned char *data, const size_t len);

#define TEST_FAIL(name, input, expectedSkipLength) START_TEST (name) \
{\
  const unsigned char *inputData = input;\
  fail_unless(find_http_req(inputData, strlen(input), NULL, NULL) == (inputData + (expectedSkipLength)), NULL);\
}\
END_TEST\

#define TEST_PASS(name, input, expectedSkipLength, expectedDataLength) START_TEST (name) \
{\
  const unsigned char *inputData = input;\
  unsigned char *outputData = NULL;\
  size_t outlen;\
  fail_unless(find_http_req(inputData, strlen(input), &outputData, &outlen) == (inputData + (expectedSkipLength) + (expectedDataLength) + 4), NULL);\
  fail_unless(outputData == inputData + (expectedSkipLength), NULL);\
  fail_unless(outlen == (expectedDataLength), NULL);\
}\
END_TEST\

TEST_FAIL(tooSmall,          "data", 0)
TEST_FAIL(noGETorPOST,       "datadatadatadatadatadatadatadatadatadatadatadatadatadatadatadatadatadatadatadatadatadatadatadatadatadatadatadatadatadata", 120 - 4)
TEST_FAIL(GETnoNewline,      "datadataGET datadatadatadatadatadatadata", 8)
TEST_FAIL(POSTnoNewline,     "datadataPOST atadatadatadatadatadatadata", 8)
//TODO: what's MAX_REQ for?
TEST_FAIL(GETtooShort,       "datadataGET data\r\ntadatadatadatadatadata", 18)
TEST_FAIL(GETnoHttp,         "datadataGET datadatadata\r\ntadatadatadata", 26)
TEST_FAIL(GETinvalidHttp,    "datadataGET dat HTTP/1.2\r\ntadatadatadata", 26)
TEST_FAIL(GETvalidHttpNoNLNL,"datadataGET dat HTTP/1.1\r\ntadatadatadata", 8) //This might be valid, just incomplete so keep the partial.
TEST_FAIL(GETnoHost,         "datadataGET dat HTTP/1.1\r\ntadata\r\n\r\ndata", 36)
TEST_PASS(GETHost,           "datadataGET dat HTTP/1.1\r\nHost: data\r\n\r\n", 8, 28)
TEST_PASS(GETCache,          "datadataGET http://example.com HTTP/1.1\r\ndata\r\n\r\n", 8, 37)
TEST_PASS(POSTCache,         "datadataPOST http://example.com HTTP/1.1\r\ndata\r\n\r\n", 8, 38)

START_TEST (overflow) 
{
  const unsigned char *inputData = "datadatadatadatadatadatadatadatadatadataGET dat HTTP/1.1\r\nHost: data\r\n\r\n";
  unsigned char *outputData = NULL;
  size_t outlen = 0;
  fail_unless(find_http_req(inputData, 48, &outputData, &outlen) == (inputData + 40), NULL); //NB. we tell find_http_req the length of the input is shorter than it is!
  fail_unless(outputData == NULL, NULL);
  fail_unless(outlen == 0, NULL);
}
END_TEST

Suite *http_suite (void) {
  Suite *s = suite_create("HTTP");

  /* Core test case */
  TCase *tc_core = tcase_create("Core");
  tcase_add_test(tc_core, tooSmall);
  tcase_add_test(tc_core, noGETorPOST);
  tcase_add_test(tc_core, GETnoNewline);
  tcase_add_test(tc_core, POSTnoNewline);
  tcase_add_test(tc_core, GETtooShort);
  tcase_add_test(tc_core, GETnoHttp);
  tcase_add_test(tc_core, GETinvalidHttp);
  tcase_add_test(tc_core, GETvalidHttpNoNLNL);
  tcase_add_test(tc_core, GETnoHost);
  tcase_add_test(tc_core, GETHost);
  tcase_add_test(tc_core, GETCache);
  tcase_add_test(tc_core, POSTCache);
  tcase_add_test(tc_core, overflow);
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
