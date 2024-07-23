#pragma leco tool

import gopt;
import hai;
import traits;
import yoyo;
import silog;

static void usage() {
  // TODO: document usage
  silog::log(silog::error, "invalid usage");
  throw 1;
}

struct token {
  hai::cstr id{};
};
using tokens = hai::varray<token>;

static auto take(const char *&buffer, char c) {
  if (*buffer != c)
    return mno::req<void>::failed("mismatched char");

  buffer++;
  return mno::req{};
}

static auto read_tag(const char *&b) {
  while (*b && *b != '>')
    b++;

  return *b ? mno::req{} : mno::req<void>::failed("expecting '>' got EOF");
}

static auto split_tokens(const hai::cstr &cstr) {
  const char *buffer = cstr.begin();

  tokens ts{1024};
  return take(buffer, '<').fmap([&] { return read_tag(buffer); }).map([&] {
    return traits::move(ts);
  });
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
      .map([](auto sz) { return hai::cstr{static_cast<unsigned>(sz)}; })
      .fpeek([&](auto &buf) {
        return input.fmap(yoyo::read(buf.begin(), buf.size()));
      })
      .peek([](auto &buffer) {
        silog::log(silog::info, "read %d bytes", buffer.size());
      })
      .fmap(split_tokens)
      .map([](auto &tokens) {
        silog::log(silog::info, "got %d tokens", tokens.size());
      })
      .log_error([] { throw 1; });
} catch (...) {
  return 1;
}
