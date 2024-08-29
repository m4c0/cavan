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
      .fpeek(cavan::lint_xml)
      .fmap(cavan::parse_pom)
      .map([](auto & pom) {
        auto & deps = pom.deps;

        silog::log(silog::info, "found %d dependencies", deps.size());

        for (auto & d : deps) {
          silog::log(silog::debug, "%s:%s:%s:%s", d.grp.begin(), d.art.begin(), d.ver.begin(), d.scp.begin());
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
