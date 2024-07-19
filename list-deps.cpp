#pragma leco tool

import gopt;
import hai;
import yoyo;
import silog;

struct token {
  hai::cstr id{};
};
using tokens = hai::varray<token>;

static void usage() {
  silog::log(silog::error, "invalid usage");
  throw 1;
}

static auto parse_file(const hai::array<char> &buffer) {
  silog::log(silog::info, "read %d bytes", buffer.size());
  return mno::req{};
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

  input.fmap(yoyo::size())
      .map([](auto sz) { return hai::array<char>{static_cast<unsigned>(sz)}; })
      .fpeek([&](auto &buf) {
        return input.fmap(yoyo::read(buf.begin(), buf.size()));
      })
      .fmap(parse_file)
      .log_error([] { throw 1; });
} catch (...) {
  return 1;
}
