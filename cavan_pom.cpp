module;
extern "C" char * getenv(const char *);

module cavan;
import jojo;
import silog;

static cavan::props list_props(const cavan::token *& t) {
  cavan::props res { 32 };
  take_if(t, "properties", [&] {
    jute::view key = t->text;

    if (match(*t, cavan::T_TAG)) {
      res.push_back(cavan::prop { key, {} });
      return;
    }
    if (!match(*t, cavan::T_OPEN_TAG)) return;

    t++;

    if (!match(*t, cavan::T_TEXT)) return;
    jute::view val = t->text;
    t++;

    if (!match(*t, cavan::T_CLOSE_TAG, key)) return;

    res.push_back(cavan::prop { key, val });
  });
  return res;
}

static void parse_project(const cavan::token *& t, cavan::pom & res) {
  using namespace cavan;

  if (match(*t, T_OPEN_TAG, "groupId")) take_tag("groupId", t, &res.grp);
  else if (match(*t, T_OPEN_TAG, "artifactId")) take_tag("artifactId", t, &res.art);
  else if (match(*t, T_OPEN_TAG, "version")) take_tag("version", t, &res.ver);
  else if (match(*t, T_OPEN_TAG, "parent"))
    take_if(t, "parent", [&] {
      if (match(*t, T_OPEN_TAG, "groupId")) take_tag("groupId", t, &res.parent.grp);
      else if (match(*t, T_OPEN_TAG, "artifactId")) take_tag("artifactId", t, &res.parent.art);
      else if (match(*t, T_OPEN_TAG, "version")) take_tag("version", t, &res.parent.ver);
      else lint_tag(t);
    });
  else if (match(*t, T_OPEN_TAG, "dependencyManagement"))
    take_if(t, "dependencyManagement", [&] { res.deps_mgmt = list_deps(t); });
  else if (match(*t, T_OPEN_TAG, "dependencies")) res.deps = list_deps(t);
  else if (match(*t, T_OPEN_TAG, "properties")) res.props = list_props(t);
  else lint_tag(t);
}

cavan::pom cavan::parse_pom(const cavan::tokens & ts) {
  auto * t = ts.begin();

  if (!match(*t++, T_DIRECTIVE, "xml")) fail("missing <?xml?> directive");

  cavan::pom res {};
  take(t, "project", [&] { parse_project(t, res); });

  if (res.grp.size() == 0) res.grp = res.parent.grp;
  if (res.ver.size() == 0) res.ver = res.parent.ver;
  return res;
}

cavan::pom cavan::read_pom(jute::view file) {
  jojo::on_error([](void *, jute::view msg) {
    silog::log(silog::error, "IO error: %s", msg.cstr().begin());
    struct io_error {};
    throw io_error {};
  });

  auto xml = jojo::read_cstr(file);
  auto tokens = split_tokens(xml);
  lint_xml(tokens);
  auto pom = parse_pom(tokens);
  pom.xml = traits::move(xml);
  pom.filename = file.cstr();
  return pom;
}

cavan::pom cavan::read_pom(jute::view grp, jute::view art, jute::view ver) try {
  if (grp == "" || art == "" || ver == "") fail("missing identifier");

  auto home_env = getenv("HOME");
  if (!home_env) fail("missing HOME environment variable");

  auto grp_path = grp.cstr();
  for (auto & c : grp_path)
    if (c == '.') c = '/';

  auto home = jute::view::unsafe(home_env);
  auto pom_file = home + "/.m2/repository/" + grp_path + "/" + art + "/" + ver + "/" + art + "-" + ver + ".pom";

  return read_pom(pom_file.cstr());
} catch (...) {
  cavan::whilst("parsing POM of " + grp + ":" + art + ":" + ver);
}

void cavan::read_parent_chain(pom * p) try {
  // TODO: read parent locally
  while (p->parent.grp.size() > 0) {
    if (!p->ppom) {
      auto * next = new pom { read_pom(p->parent.grp, p->parent.art, p->parent.ver) };
      p->ppom.reset(next);
    }
    p = &*p->ppom;
  }
} catch (...) {
  cavan::whilst("reading chain of " + p->grp + ":" + p->art + ":" + p->ver);
}
