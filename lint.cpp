#pragma leco tool
import cavan;
import gopt;
import hai;
import jute;
import silog;
import yoyo;

using namespace cavan;

static void usage() {
  // TODO: document usage
  silog::log(silog::error, "invalid usage");
  throw 1;
}

int main(int argc, char **argv) try {
  auto input = yoyo::file_reader::std_in();
  auto opts = gopt_parse(argc, argv, "i:", [&](auto ch, auto val) {
    switch (ch) {
    case 'i':
      input = yoyo::file_reader::open(val);
      break;
    default:
      usage();
    }
  });

  if (opts.argc != 0)
    usage();

  input.fmap(read_tokens).fmap(cavan::lint_xml).log_error([] { throw 1; });
} catch (...) {
  return 1;
}
