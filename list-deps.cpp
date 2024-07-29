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
            auto &deps = pom.deps;

            silog::log(silog::info, "found %d dependencies", deps.size());

            for (auto &d : deps) {
              silog::log(silog::debug, "%s:%s:%s:%s", d.grp.begin(),
                         d.art.begin(), d.ver.begin(), d.scp.begin());
            }
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
