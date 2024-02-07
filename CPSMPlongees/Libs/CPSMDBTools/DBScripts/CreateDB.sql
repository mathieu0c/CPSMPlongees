PRAGMA foreign_keys = ON;


CREATE TABLE IF NOT EXISTS "DivingSitesTable" (
        "diving_site_id" INTEGER,
        "site_name"	TEXT UNIQUE,
        PRIMARY KEY("diving_site_id" AUTOINCREMENT)
);


CREATE TABLE IF NOT EXISTS "DiverLevels" (
        "diver_level_id" INTEGER,
        "level_name"	TEXT UNIQUE,
        PRIMARY KEY("diver_level_id" AUTOINCREMENT)
);

CREATE TABLE IF NOT EXISTS "DiversTable" (
        "diver_id"              INTEGER NOT NULL,
        "lastname"              TEXT,
        "firstname"             TEXT,
        "diver_level_id"	TEXT,
        PRIMARY KEY("diver_id" AUTOINCREMENT),
        FOREIGN KEY("diver_level_id") REFERENCES "DiverLevels"("diver_level_id")
);


CREATE TABLE IF NOT EXISTS "DivesTable" (
        "dive_id"               INTEGER,
        "dive_datetime"         TEXT,
        "diving_site_id"	INTEGER,
        PRIMARY KEY("dive_id" AUTOINCREMENT),
        FOREIGN KEY("diving_site_id") REFERENCES "DivingSitesTable"("diving_site_id")
);


CREATE TABLE IF NOT EXISTS "DivesMembersTable" (
        "dive_id"	INTEGER,
        "diver_id"	INTEGER,
        FOREIGN KEY("dive_id") REFERENCES "DivesTable"("dive_id") ON DELETE CASCADE,
        FOREIGN KEY("diver_id") REFERENCES "DiversTable"("diver_id") ON DELETE CASCADE,
        PRIMARY KEY("dive_id","diver_id")
) WITHOUT ROWID;


-- INIT


--  Diver levels
INSERT OR IGNORE INTO "DiverLevels"
    ("level_name") VALUES
    ("PE20"),
    ("PE40"),
    ("PA20"),
    ("PE60"),
    ("PA60"),
    ("E1"),
    ("E2"),
    ("E3");


--  Diving sites
INSERT OR IGNORE INTO "DivingSitesTable"
    ("site_name") VALUES
    ("Les deux frères"),
    ("L'Arroyo");
