#pragma leco tool

import cavan;
import hai;
import jojo;
import jute;
import missingno;
import silog;

using namespace cavan;
using namespace jute::literals;

static void list_deps(void *, hai::cstr & xml) {
  mno::req { split_tokens(xml) }
      .peek(cavan::lint_xml)
      .map(cavan::parse_pom)
      .map([](auto & pom) {
        auto & deps = pom->deps;

        silog::log(silog::info, "found %d dependencies", deps.size());

        for (auto & d : deps) {
          silog::log(silog::debug, "%s:%s:%s:%s", d.grp.cstr().begin(), d.art.cstr().begin(), (*d.ver).cstr().begin(),
                     d.scp.cstr().begin());
        }
      })
      .log_error([] { throw 1; });
}

int main(int argc, char ** argv) try {
  for (auto i = 1; i < argc; i++) {
    jojo::read(jute::view::unsafe(argv[i]), nullptr, list_deps);
  }
} catch (...) {
  return 1;
}
