#pragma leco tool
#include <stdlib.h>
#include <string.h>

import cavan;
import jute;
import silog;

using namespace jute::literals;

int main(int argc, char ** argv) try {
  const char * pwd = argc == 1 ? "pom.xml" : argv[1];

  char fname[10240] {};
  realpath(pwd, fname);

  silog::trace("processing", fname);
  auto pom = cavan::read_pom(fname);
  if (pom->modules.size() > 0) {
    for (auto & m : pom->modules) {
      silog::trace("module", m);
      auto [dir, fn] = jute::view { pom->filename }.rsplit('/');
      auto cpom = dir + "/" + m + "/pom.xml";
      auto pom = cavan::read_pom(cpom.cstr());
      cavan::generate_javac_argfile(pom, true);
    }
  } else {
    cavan::generate_javac_argfile(pom, true);
  }
} catch (...) {
  return 1;
}
