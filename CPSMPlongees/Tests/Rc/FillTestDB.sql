-- DIVERS
INSERT INTO "DiversTable"
    (firstname,lastname) VALUES
    ('Bidule','Truc'),
    ('Rose','Orange'),
    ('Johnson','Beer'),
    ('Leath','Broom');


-- DIVES
INSERT INTO "DivesTable"
    ("dive_datetime",diving_site_id) VALUES
    (datetime(),1),
    (datetime(),2),
    (datetime(),1);


-- DIVES MEMBERS
INSERT INTO "DivesMembersTable"
    (dive_id,diver_id) VALUES
    (3,1),
    (3,2);

INSERT INTO "DivesMembersTable"
    (dive_id,diver_id) VALUES
    (1,1),
    (1,4);

INSERT INTO "DivesMembersTable"
    (dive_id,diver_id) VALUES
    (2,1),
    (2,4),
    (2,3);
