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

static mno::req<void> lint_tag(const token *&t) {
  if (!match(*t, T_OPEN_TAG))
    return mno::req<void>::failed("missing open tag");

  jute::view id{t->id};

  for (t++; !match(*t, T_END); t++) {
    if (match(*t, T_CLOSE_TAG, id)) {
      return mno::req{};
    }

    if (match(*t, T_CLOSE_TAG))
      return mno::req<void>::failed("mismatched close tag");

    if (match(*t, T_OPEN_TAG)) {
      auto res = lint_tag(t);
      if (!res.is_valid())
        return res;
      continue;
    }
  }
  return mno::req<void>::failed("missing open tag");
}

static mno::req<void> lint_xml(const cavan::tokens &tokens) {
  auto *t = tokens.begin();

  if (!match(*t, T_OPEN_TAG, "?xml"))
    return mno::req<void>::failed("missing <?xml?> directive");

  return lint_tag(++t);
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

  input.fmap(read_tokens).fmap(lint_xml).log_error([] { throw 1; });
} catch (...) {
  return 1;
}
