PRAGMA foreign_keys = ON;

-- DBInfo definition
CREATE TABLE IF NOT EXISTS DBInfo (
  dbinfo_id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
  latest_cpsm_soft_version_used TEXT NOT NULL,
  latest_cpsm_db_version_used TEXT NOT NULL,
  latest_open_date TEXT NOT NULL
);

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
  member_date TEXT,
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

-- DBInfo
INSERT OR IGNORE INTO DBInfo
    (dbinfo_id, latest_cpsm_soft_version_used, latest_cpsm_db_version_used, latest_open_date)
    VALUES
    (1, "0.0.0", "0.0.0", "1970-01-01 00:00:00");

--  Diver levels
INSERT OR IGNORE INTO DiversLevels
    (diver_level_id,level_name) VALUES
    (1,"Débutant"),
    (2,"PE12"),
    (3,"PE20"),
    (4,"PE40"),
    (5,"PA20"),
    (6,"PA20 + PE40"),
    (7,"PA40"),
    (8,"PE60"),
    (9,"PA60"),
    (10,"Stagiaire péda"),
    (11,"E1"),
    (12,"E2"),
    (13,"E3"),
    (14,"E4");


--  Diving sites
INSERT OR IGNORE INTO DivingSites
    (diving_site_id, site_name) VALUES
    (1,"Les deux frères"),
    (2,"L'Arroyo"),
    (3,"La sèche du pêcheur"),
    (4,"La sèche de Saint-Elme"),
    (5,"La roche à l'ancre"),
    (6,"La vallée aux gorgones"),
    (7,"Le Tromblon"),
    (8,"Le SMBD2"),
    (9,"Le Dornier"),
    (10,"La Corée");


--  Diving types
INSERT OR IGNORE INTO DivingTypes
    (diving_type_id,type_name) VALUES
    (1,"Explo"),
    (2,"Tech");
