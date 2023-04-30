SELECT * FROM DivesMembersTable GROUP BY dive_id;

SELECT * FROM DivesMembersTable
        INNER JOIN DivesTable USING (dive_id)
        INNER JOIN DiversTable USING (diver_id)
        INNER JOIN DivingSitesTable USING (diving_site_id)
;

SELECT *,COUNT(diver_id) as kDiverCount FROM DivesMembersTable
        INNER JOIN DivesTable USING (dive_id)
        INNER JOIN DiversTable USING (diver_id)
        INNER JOIN DivingSitesTable USING (diving_site_id)
        GROUP BY dive_id
        ORDER BY kDiverCount DESC
;

DELETE FROM "DivesTable" WHERE dive_id=3;

SELECT dive_datetime,site_name,COUNT(diver_id) as kDiverCount FROM DivesMembersTable
        INNER JOIN DivesTable USING (dive_id)
        INNER JOIN DiversTable USING (diver_id)
        INNER JOIN DivingSitesTable USING (diving_site_id)
        GROUP BY dive_id
        ORDER BY kDiverCount DESC
;

SELECT *,(SELECT COUNT(1) FROM DivesMembersTable WHERE dive_id = 1) FROM DivesMembersTable
        INNER JOIN DivesTable USING (dive_id)
        INNER JOIN DiversTable USING (diver_id)
        LEFT JOIN DivingSitesTable USING (diving_site_id)
        WHERE dive_id = 1
        ORDER BY diving_site_id ASC, lastname
;

DELETE FROM "DiversTable" WHERE diver_id = 4;

SELECT *,(SELECT COUNT(1) FROM DivesMembersTable WHERE dive_id = 1) FROM DivesMembersTable
        INNER JOIN DivesTable USING (dive_id)
        INNER JOIN DiversTable USING (diver_id)
        LEFT JOIN DivingSitesTable USING (diving_site_id)
        WHERE dive_id = 1
        ORDER BY diving_site_id ASC, lastname
;
