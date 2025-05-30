#pragma leco tool

import cavan;
import hai;
import jojo;
import jute;
import print;
import tora;

int main(int argc, char ** argv) try {
  tora::db db { "out/poc-tdeps-sqlite.db" };
  db.exec(R"(
    PRAGMA mmap_size=268435456;

    CREATE TABLE IF NOT EXISTS file (
      path TEXT NOT NULL PRIMARY KEY,
      data BLOB NOT NULL
    ) STRICT;
  )");
  auto sel = db.prepare("SELECT data FROM file WHERE path = ?");
  auto ins = db.prepare("INSERT INTO file (path, data) VALUES (?, ?)");

  const auto shift = [&] { return jute::view::unsafe(argc == 1 ? "" : (--argc, *++argv)); };

  auto file = shift();
  if (file == "") die("missing file");

  cavan::file_reader = [&](auto fname) -> hai::cstr {
    sel.reset();
    sel.bind(1, fname);
    if (sel.step()) return sel.column_blob(0).cstr();

    auto data = jojo::read_cstr(fname);
    ins.reset();
    ins.bind(1, fname);
    ins.bind_blob(2, data);
    ins.step();
    return data;
  };

  auto pom = cavan::read_pom(file);

  for (auto p : cavan::resolve_transitive_deps(pom)) {
    putln(cavan::path_of(p->grp, p->art, p->ver, "jar"));
  }
} catch (...) {
  return 3;
}
