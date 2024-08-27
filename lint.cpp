#pragma leco tool
import cavan;
import gopt;
import hai;
import jojo;
import jute;
import silog;

using namespace cavan;

static void usage() {
  // TODO: document usage
  silog::log(silog::error, "invalid usage");
  throw 1;
}

static void lint(void *, hai::cstr & xml) {
  split_tokens(xml).fmap(cavan::lint_xml).log_error([] { throw 1; });
}

int main(int argc, char ** argv) try {
  auto opts = gopt_parse(argc, argv, "i:", [&](auto ch, auto val) {
    switch (ch) {
      case 'i': jojo::read(jute::view::unsafe(val), nullptr, lint); break;
      default: usage();
    }
  });

  if (opts.argc != 0) usage();

} catch (...) {
  return 1;
}
