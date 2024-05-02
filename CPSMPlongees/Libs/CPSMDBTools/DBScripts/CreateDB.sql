PRAGMA foreign_keys = ON;


-- DiversLevels definition

CREATE TABLE IF NOT EXISTS "DiversLevels" (
  diver_level_id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
  level_name TEXT UNIQUE
);

-- Divers definition

CREATE TABLE IF NOT EXISTS Divers (
  diver_id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
  first_name TEXT,
  last_name TEXT,
  birth_date TEXT,
  email TEXT,
  phone_number TEXT,
  address_id INTEGER,
  license_number TEXT,
  certif_date TEXT,
  diver_level_id INTEGER,
  registration_date TEXT,
  paid_dives INTEGER,
  gear_regulator INTEGER DEFAULT (0),
  gear_suit INTEGER DEFAULT (0),
  gear_computer INTEGER DEFAULT (0),
  gear_jacket INTEGER DEFAULT (0),
  CONSTRAINT Divers_DiverLevels_FK FOREIGN KEY (diver_level_id) REFERENCES "DiversLevels"(diver_level_id) ON DELETE SET DEFAULT ON UPDATE SET DEFAULT,
  CONSTRAINT Divers_DiversAddresses_FK FOREIGN KEY (address_id) REFERENCES DiversAddresses(address_id) ON DELETE SET NULL
);

-- DiversAddresses definition

CREATE TABLE IF NOT EXISTS DiversAddresses (
  address_id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
  address TEXT,
  postal_code TEXT,
  city TEXT
);

-- Dives definition

CREATE TABLE IF NOT EXISTS Dives (
  dive_id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
  datetime TEXT,
  diving_site_id INTEGER,
  CONSTRAINT Dives_DivingSites_FK FOREIGN KEY (diving_site_id) REFERENCES DivingSites(diving_site_id)
);

-- DivesMembers definition

CREATE TABLE IF NOT EXISTS DivesMembers (
  dive_id INTEGER,
  diver_id INTEGER,
  diving_type_id INTEGER,
  CONSTRAINT DivesMembers_PK PRIMARY KEY (dive_id,diver_id),
  CONSTRAINT DivesMembers_Divers_FK FOREIGN KEY (diver_id) REFERENCES Divers(diver_id) ON DELETE CASCADE,
  CONSTRAINT DivesMembers_Dives_FK FOREIGN KEY (dive_id) REFERENCES Dives(dive_id) ON DELETE CASCADE,
  CONSTRAINT DivesMembers_DivingType_FK FOREIGN KEY (diving_type_id) REFERENCES DivingTypes(diving_type_id) ON DELETE SET NULL
);

-- DivingSites definition

CREATE TABLE IF NOT EXISTS DivingSites (
  diving_site_id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
  site_name TEXT
);

-- DivingType definition

CREATE TABLE IF NOT EXISTS DivingTypes (
  diving_type_id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
  type_name TEXT
);


-- INIT

--  Diver levels
INSERT OR IGNORE INTO "DiversLevels"
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
INSERT OR IGNORE INTO "DivingSites"
    ("site_name") VALUES
    ("Les deux frères"),
    ("L'Arroyo"),
    ("La sèche du pêcheur"),
    ("La sèche de Saint-Elme"),
    ("La roche à l'ancre"),
    ("La vallée aux gorgones"),
    ("Le Tromblon");


--  Diving types
INSERT OR IGNORE INTO "DivingTypes"
    ("type_name") VALUES
    ("Explo"),
    ("Tech");
