module;
extern "C" char * getenv(const char *);

module cavan;
import jojo;
import silog;

static cavan::props list_props(const cavan::token *& t) {
  cavan::props res { 100 };
  take_if(t, "properties", [&] {
    if (!match(*t, cavan::T_OPEN_TAG)) return;

    jute::view key = t->id;
    t++;

    if (!match(*t, cavan::T_TEXT)) return;
    jute::view val = t->id;

    if (!match(*t, cavan::T_CLOSE_TAG, key)) return;

    res.push_back_doubling(cavan::prop { key.cstr(), val.cstr() });
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

  if (res.grp.size() == 0) res.grp = jute::view { res.parent.grp }.cstr();
  if (res.ver.size() == 0) res.ver = jute::view { res.parent.ver }.cstr();
  return res;
}

cavan::pom cavan::read_pom(const hai::cstr & xml) {
  auto tokens = split_tokens(xml);
  lint_xml(tokens);
  return parse_pom(tokens);
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

  auto pom = read_pom(jojo::read_cstr(pom_file.cstr()));
  pom.filename = pom_file.cstr();
  return pom;
} catch (...) {
  silog::log(silog::info, "while parsing POM of %s:%s:%s", grp.cstr().begin(), art.cstr().begin(), ver.cstr().begin());
  throw;
}
