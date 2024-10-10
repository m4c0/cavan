#pragma leco tool
#include <stdlib.h>
#include <string.h>

import cavan;
import jute;
import silog;

using namespace jute::literals;

int main(int argc, char ** argv) try {
  if (argc != 2) cavan::fail("usage: compile.exe <java-file>");

  char fname[10240] {};
  realpath(argv[1], fname);

  bool test_scope = strstr(fname, "src/test/");

  auto pom = cavan::read_pom_of_source(fname);
  silog::trace("processing", pom->filename);

  auto tmpnam = generate_javac_argfile(pom, test_scope);

  silog::trace("compiling", tmpnam);
  auto cmd = ("javac @"_s + tmpnam + " " + fname).cstr();
  return system(cmd.begin());
} catch (...) {
  return 1;
}
