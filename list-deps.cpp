#pragma leco tool

import gopt;
import hai;
import jute;
import traits;
import yoyo;
import silog;

static void usage() {
  // TODO: document usage
  silog::log(silog::error, "invalid usage");
  throw 1;
}

enum type {
  T_NULL,
  T_OPEN_TAG,
  T_CLOSE_TAG,
  T_TAG,
  T_TEXT,
};
struct token {
  hai::cstr id{};
  type type{};
};
using tokens = hai::varray<token>;

static auto take(const char *&buffer, char c) {
  if (*buffer != c)
    return mno::req<void>::failed("mismatched char");

  buffer++;
  return mno::req{};
}

static auto read_tag(const char *&b) {
  const char *start = b;

  while (*b && *b != '>')
    b++;

  if (!*b)
    return mno::req<token>::failed("expecting '>' got EOF");

  const char *end;
  for (end = start; end <= b; end++)
    if (*end == ' ')
      break;

  auto id = jute::view{start, static_cast<unsigned>(end - start)};
  return mno::req{token{id.cstr()}};
}

static auto split_tokens(const hai::cstr &cstr) {
  const char *buffer = cstr.begin();

  tokens ts{1024};
  return take(buffer, '<')
      .fmap([&] { return read_tag(buffer); })
      .map([&](token &t) { ts.push_back_doubling(traits::move(t)); })
      .map([&] { return traits::move(ts); });
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
        for (auto &t : tokens) {
          silog::log(silog::info, "-- [%s]", t.id.begin());
        }
      })
      .log_error([] { throw 1; });
} catch (...) {
  return 1;
}
