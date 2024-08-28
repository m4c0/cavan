#pragma leco tool

import cavan;
import hai;
import jojo;
import jute;
import traits;
import silog;

using namespace cavan;
using namespace jute::literals;

static void dump_pom(void *, hai::cstr & xml) {
  split_tokens(xml)
      .fpeek(cavan::lint_xml)
      .fmap(cavan::parse_pom)
      .until_failure(
          [](auto & pom) {
            silog::log(silog::info, "filename: %s", pom.filename.begin());
            silog::log(silog::info, "name: %s:%s:%s", pom.grp.begin(), pom.art.begin(), pom.ver.begin());
            silog::log(silog::info, "parent: %s:%s:%s", pom.parent.grp.begin(), pom.parent.art.begin(),
                       pom.parent.ver.begin());
            silog::log(silog::info, "found %d dependencies", pom.deps.size());
            silog::log(silog::info, "found %d managed dependencies", pom.deps_mgmt.size());

            return cavan::read_pom(pom.parent.grp, pom.parent.art, pom.parent.ver);
          },
          [](auto) { return true; })
      .map([](auto &) {})
      .log_error([] { throw 1; });
}

int main(int argc, char ** argv) try {
  for (auto i = 1; i < argc; i++) {
    jojo::read(jute::view::unsafe(argv[i]), nullptr, dump_pom);
  }
} catch (...) {
  return 1;
}
