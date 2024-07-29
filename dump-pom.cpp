#pragma leco tool

import cavan;
import gopt;
import hai;
import jute;
import traits;
import yoyo;
import silog;

using namespace cavan;
using namespace jute::literals;

static void usage() {
  // TODO: document usage
  silog::log(silog::error, "invalid usage");
  throw 1;
}

int main(int argc, char **argv) try {
  auto opts = gopt_parse(argc, argv, "i:", [&](auto ch, auto val) {
    switch (ch) {
    case 'i':
      yoyo::file_reader::open(val)
          .fmap(cavan::read_tokens)
          .fpeek(cavan::lint_xml)
          .fmap(cavan::parse_pom)
          .map([](auto &pom) {
            silog::log(silog::info, "found %d dependencies", pom.deps.size());
            silog::log(silog::info, "found %d managed dependencies",
                       pom.deps_mgmt.size());
          })
          .log_error([] { throw 1; });
      break;
    default:
      usage();
    }
  });

  if (opts.argc != 0)
    usage();
} catch (...) {
  return 1;
}
