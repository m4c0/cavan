module cavan;
import :fail;
import silog;

void cavan::lint_tag(const cavan::token *& t) {
  if (match(*t, T_TAG)) return;

  if (!match(*t, T_OPEN_TAG)) return fail("missing open tag around [" + t->text + "]");

  jute::view id = t->text;

  for (t++; !match(*t, T_END); t++) {
    if (match(*t, T_CLOSE_TAG, id)) return;

    if (match(*t, T_CLOSE_TAG)) return fail("mismated close tag - expecting: " + t->text + ", got: " + id);

    if (match(*t, T_OPEN_TAG)) {
      try {
        lint_tag(t);
      } catch (...) {
        silog::log(silog::info, "inside %.*s", (int)t->text.size(), t->text.begin());
        throw;
      }
    }
  }
  return fail("missing close tag for "_s + id);
}

void cavan::lint_xml(const cavan::tokens & tokens) {
  const auto * t = tokens.begin();
  return lint_tag(t);
}
