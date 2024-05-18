-- Inserting DiversAddresses (with French addresses):
INSERT INTO DiversAddresses (address, postal_code, city)
VALUES
  -- Address for John Doe (Level 1)
  ('12 Rue de la Plage', '75001', 'Paris'),
  -- Address for Jane Smith (Level 2) and David Brown (Level 4)
  ('23 Avenue des Champs-Élysées', '69001', 'Lyon'),
  -- Address for Michael Johnson (Level 3)
  ('7 Place du Marché', '33000', 'Bordeaux'),
  -- Address for Sarah Williams (Level 1)
  ('9 Rue de la Liberté', '13001', 'Marseille'),
  -- Unused address for test
  ('5 Rue Victor Hugo', '31000', 'Toulouse');

-- Inserting Divers (with French phone numbers):
INSERT INTO Divers (first_name, last_name, birth_date, email, phone_number, address_id,
license_number, certif_date, diver_level_id, registration_date, is_member, member_date, paid_dives,
gear_regulator, gear_suit, gear_computer, gear_jacket)
VALUES
  ('John', 'DOE', '1990-05-15', 'john.doe@example.com', '0612345678', 1,
    'ABC123', '2022-01-01', 1, '2024-02-28', 0, '2021-02-28', 10, 1, 1, 0, 0),-- Level 1 (PE20)
  ('Jane', 'SMITH', '1988-10-20', 'jane.smith@example.com', '0698745632', 2,
    'XYZ456', '2022-02-15', 2, '2024-02-28', 1, '2024-02-28', 15, 0, 1, 1, 1),-- Level 2 (PE40)
  ('Michael', 'JOHNSON', '1995-03-12', 'michael.johnson@example.com', '0655565556', 3,
    'DEF789', '2023-05-20', 3, '2024-02-28', 1, '2024-02-28', 1, 1, 0, 1, 0),-- Level 3 (PA20)
  ('Sarah', 'WILLIAMS', '1992-07-08', 'sarah.williams@example.com', '0611122333', 4,
    'GHI101', '2023-08-10', 1, '2022-08-12', 0, '2022-08-12', 20, 1, 1, 1, 1),-- Level 1 (PE20)
  ('David', 'BROWN', '1998-12-25', 'david.brown@example.com', '0677889999', 2,
    'JKL202', '2023-10-30', 4, '2021-02-28', 0, '2021-02-28', 8, 0, 1, 0, 1);-- Level 4 (PE60)

-- Inserting Dives:
INSERT INTO Dives (datetime, diving_site_id)
VALUES
  ('2024-02-10 09:00:00', 1),  -- Dive 1: Les deux frères
  ('2024-02-11 10:30:00', 2),  -- Dive 2: L'Arroyo
  ('2024-02-12 14:15:00', 3),  -- Dive 3: La sèche du pêcheur
  ('2024-02-13 08:45:00', 4),  -- Dive 4: La sèche de Saint-Elme
  ('2024-02-13 12:30:00', 5);  -- Dive 5: La roche à l'ancre

-- Inserting DivesMembers:
INSERT INTO DivesMembers (dive_id, diver_id, diving_type_id)
VALUES
--  (1, 1, 1),  -- John Doe (Level 1) in Dive 1 - Explo
  (1, 2, 1),  -- Jane Smith (Level 2) in Dive 1 - Explo
  (1, 3, 1),  -- Michael Johnson (Level 3) in Dive 1 - Explo
  (2, 2, 1),  -- Jane Smith (Level 2) in Dive 2 - Explo
  (2, 4, 1),  -- Sarah Williams (Level 1) in Dive 2 - Explo
  (2, 5, 1),  -- David Brown (Level 4) in Dive 2 - Explo
  (3, 1, 2),  -- John Doe (Level 1) in Dive 3 - Tech
  (3, 3, 2),  -- Michael Johnson (Level 3) in Dive 3 - Tech
  (3, 5, 2),  -- David Brown (Level 4) in Dive 3 - Tech
  (4, 2, 2),  -- Jane Smith (Level 2) in Dive 4 - Tech
  (4, 3, 2),  -- Michael Johnson (Level 3) in Dive 4 - Tech
  (4, 4, 2),  -- Sarah Williams (Level 1) in Dive 4 - Tech
  (5, 1, 1),  -- John Doe (Level 1) in Dive 5 - Explo
  (5, 4, 1),  -- Sarah Williams (Level 1) in Dive 5 - Explo
  (5, 5, 1);  -- David Brown (Level 4) in Dive 5 - Explo
