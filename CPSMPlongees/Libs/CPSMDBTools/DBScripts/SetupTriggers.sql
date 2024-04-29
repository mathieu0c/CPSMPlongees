CREATE TRIGGER remove_unused_address AFTER DELETE
ON Divers
BEGIN
 DELETE FROM DiversAddresses
 WHERE address_id = OLD.address_id
   AND NOT EXISTS (SELECT 1 FROM Divers WHERE address_id = OLD.address_id);
END;
