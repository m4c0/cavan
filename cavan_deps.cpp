module cavan;
import :fail;
import silog;

using namespace cavan;

void cavan::take_tag(jute::view exp_id, const token *& t, jute::view * out) {
  t++;
  if (!match(*t, T_TEXT)) fail("expecting text inside tag");

  *out = t->text;

  t++;
  if (!match(*t, T_CLOSE_TAG, exp_id)) fail("missing close tag for " + exp_id);
}
void take_tag(jute::view exp_id, const token *& t, jute::heap * out) {
  jute::view tmp;
  cavan::take_tag(exp_id, t, &tmp);
  *out = jute::heap { jute::no_copy {}, tmp };
}

void take_opt_tag(jute::view exp_id, const token *& t, jute::view * out) {
  t++;
  if (match(*t, T_TEXT)) {
    *out = t->text;
    t++;
  }
  if (!match(*t, T_CLOSE_TAG, exp_id)) fail("missing close tag for " + exp_id);
}

static void take_exclusions(const token *& t, auto & exc) try {
  exc = hai::sptr<hai::chain<cavan::excl>>::make(16U);

  jute::view grp {};
  jute::view art {};
  for (; !match(*t, T_END); t++) {
    if (match(*t, T_OPEN_TAG, "groupId")) {
      take_tag("groupId", t, &grp);
    } else if (match(*t, T_OPEN_TAG, "artifactId")) {
      take_tag("artifactId", t, &art);
    } else if (match(*t, T_CLOSE_TAG, "exclusion")) {
      exc->push_back(cavan::excl { grp, art });
    } else if (match(*t, T_CLOSE_TAG, "exclusions")) {
      return;
    }
  }
} catch (...) {
  silog::log(silog::info, "while parsing exclusions");
  throw;
}

static dep take_dep(const token *& t) try {
  dep d {};

  for (; t->type != T_END; t++) {
    if (match(*t, T_OPEN_TAG, "groupId")) {
      take_tag("groupId", t, &d.grp);
    } else if (match(*t, T_OPEN_TAG, "artifactId")) {
      take_tag("artifactId", t, &d.art);
    } else if (match(*t, T_OPEN_TAG, "version")) {
      take_tag("version", t, &d.ver);
    } else if (match(*t, T_OPEN_TAG, "scope")) {
      take_tag("scope", t, &d.scp);
    } else if (match(*t, T_OPEN_TAG, "optional")) {
      jute::view tmp {};
      take_tag("optional", t, &tmp);
      d.opt = "true"_s == tmp;
    } else if (match(*t, T_OPEN_TAG, "classifier")) {
      take_opt_tag("classifier", t, &d.cls);
    } else if (match(*t, T_TAG, "classifier")) {
    } else if (match(*t, T_OPEN_TAG, "systemPath")) {
      jute::view tmp {};
      take_opt_tag("systemPath", t, &tmp);
    } else if (match(*t, T_OPEN_TAG, "type")) {
      take_tag("type", t, &d.typ);
    } else if (match(*t, T_OPEN_TAG, "exclusions")) {
      take_exclusions(t, d.exc);
    } else if (match(*t, T_CLOSE_TAG, "dependency")) {
      break;
    } else fail("unknown stuff found inside dependencies around " + t->text);
  }

  // silog::log(silog::debug, "dependency: %s:%s:%s:%s", grp, art, ver, scp);
  return d;
} catch (...) {
  whilst("parsing dependency");
}

deps cavan::list_deps(const token *& t) {
  deps res {};
  take_if(t, "dependencies", [&] {
    if (!match(*t, T_OPEN_TAG, "dependency")) return;

    res.push_back(take_dep(++t));
  });
  return res;
}
