
DROP TABLE DivesMembers;

CREATE TABLE DivesMembers (
	dive_id INTEGER,
	diver_id INTEGER,
	diving_type_id INTEGER,
	CONSTRAINT DivesMembers_PK PRIMARY KEY (dive_id,diver_id),
	CONSTRAINT DivesMembers_Divers_FK FOREIGN KEY (diver_id) REFERENCES Divers(diver_id) ON DELETE SET NULL,
	CONSTRAINT DivesMembers_Dives_FK FOREIGN KEY (dive_id) REFERENCES Dives(dive_id) ON DELETE CASCADE,
	CONSTRAINT DivesMembers_DivingType_FK FOREIGN KEY (diving_type_id) REFERENCES DivingType(diving_type_id) ON DELETE SET NULL
);