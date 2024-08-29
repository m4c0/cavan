module cavan;
import :fail;
import silog;

void cavan::lint_tag(const token *& t) {
  if (match(*t, T_TAG)) return;

  if (!match(*t, T_OPEN_TAG)) return fail("missing open tag around ["_s + t->id + "]");

  jute::view id { t->id };

  for (t++; !match(*t, T_END); t++) {
    if (match(*t, T_CLOSE_TAG, id)) return;

    if (match(*t, T_CLOSE_TAG)) return fail("mismated close tag - expecting: "_s + t->id + ", got: " + id);

    if (match(*t, T_OPEN_TAG)) {
      try {
        lint_tag(t);
      } catch (...) {
        silog::log(silog::info, "inside %s", t->id.begin());
        throw;
      }
    }
  }
  return fail("missing close tag for "_s + id);
}

void cavan::lint_xml(const cavan::tokens & tokens) {
  auto * t = tokens.begin();

  if (!match(*t, T_DIRECTIVE, "xml")) return fail("missing <?xml?> directive");

  return lint_tag(++t);
}
