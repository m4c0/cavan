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
  T_END,
};
struct token {
  hai::cstr id{};
  type type{};
};
using tokens = hai::varray<token>;

static auto blank(const char *c) {
  return *c == ' ' || *c == '\t' || *c == '\n' || *c == '\r';
}

static auto read_tag(const char *&b) {
  const char *start = b;

  while (*b && *b != '>')
    b++;

  if (!*b)
    return mno::req<token>::failed("expecting '>' got EOF");

  const char *end;
  for (end = start; end < b; end++)
    if (blank(end))
      break;

  b++; // consume '>'

  auto id = jute::view{start + 1, static_cast<unsigned>(end - start - 1)};
  return mno::req{token{id.cstr(), T_OPEN_TAG}};
}

static auto read_end_tag(const char *&b) {
  return read_tag(b).peek([](auto &t) {
    t.id = jute::view{t.id}.subview(1).after.cstr();
    t.type = T_CLOSE_TAG;
  });
}

static auto read_text(const char *&b) {
  const char *start = b;
  while (*b && *b != '<') {
    b++;
  }

  const char *end = b;
  while (blank(end) && end > start)
    end--;

  if (end == start)
    return mno::req<token>{};

  auto id = jute::view{start, static_cast<unsigned>(end - start)};
  return mno::req{token{id.cstr(), T_TEXT}};
}

static auto split_tokens(const hai::cstr &cstr) {
  const char *buffer = cstr.begin();

  mno::req<tokens> ts{tokens{1024}};
  while (*buffer && ts.is_valid()) {
    mno::req<token> t{};

    switch (*buffer) {
    case '<':
      t = (buffer[1] == '/') ? read_end_tag(buffer) : read_tag(buffer);
      break;
    case ' ':
    case '\t':
    case '\r':
    case '\n':
      buffer++;
      continue;
    default:
      t = read_text(buffer);
      break;
    }

    ts = mno::combine(
        [](auto &ts, auto &t) {
          if (t.type != T_NULL)
            ts.push_back_doubling(traits::move(t));
          return traits::move(ts);
        },
        ts, t);
  }
  return ts.peek([](auto &ts) { ts.push_back_doubling(token{{}, T_END}); });
}

void list_deps(const token *t) {
  using namespace jute::literals;

  int count{};
  for (; t->type != T_END; t++) {
    if (t->type == T_OPEN_TAG && "dependency"_s == t->id)
      count++;
  }
  silog::log(silog::debug, "found %d dependencies", count);
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
      .peek([](auto &tokens) {
        silog::log(silog::info, "got %d tokens", tokens.size());
      })
      .map([](auto &tokens) { list_deps(tokens.begin()); })
      .log_error([] { throw 1; });
} catch (...) {
  return 1;
}
